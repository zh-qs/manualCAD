#include "object.h"
#include "object_settings.h"
#include "tridiagonal_linear_equation_system.h"
#include "object_controller.h"
#include "settings_window.h"
#include "serializer.h"
#include "linear_equation_system_4x4.h"
#include <ctime>
#include <random>
#include <map>

namespace ManualCAD
{
	int Torus::counter = 0;
	int Point::counter = 0;
	int BezierC0Curve::counter = 0;
	int BezierC2Curve::counter = 0;
	int InterpolationSpline::counter = 0;
	int BicubicC0BezierSurface::counter = 0;
	int BicubicC2BezierSurface::counter = 0;
	int Gregory20ParamSurface::counter = 0;
	int IntersectionCurve::counter = 0;

	Transformation ObjectCollection::EMPTY = {};

	float general_intersection(const Vector3& position, const Vector3& origin, const Vector3& ray)
	{
		float min_sin = ApplicationSettings::SELECTION_THRESHOLD;
		Vector3 to_object = position - origin;
		float sin = cross(normalize(to_object), ray).length();
		if (sin < min_sin)
			return to_object.length();

		return NAN;
	}

	const Renderable& Object::get_renderable()
	{
		if (!valid) {
			generate_renderable();
			valid = true;
		}
		update_renderable_matrix();
		return renderable;
	}

	const Renderable& Object::get_renderable(const Transformation& combine_transformation, const Vector3& center)
	{
		if (!valid) {
			generate_renderable();
			valid = true;
		}
		update_renderable_matrix(combine_transformation, center);
		return renderable;
	}

	void Object::update_renderable_matrix()
	{
		if (!illusory) renderable.set_model_matrix(transformation.get_matrix());
	}

	void Object::update_renderable_matrix(const Transformation& combine_transformation, const Vector3& center)
	{
		if (!illusory) renderable.set_model_matrix(transformation.get_matrix_combined_with(combine_transformation, center));
	}

	void Object::build_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_general_settings(*this);
		build_specific_settings(parent);
	}

	void Torus::generate_renderable()
	{
		//mesh = WireframeMesh::torus(large_radius, small_radius, divisions_x, divisions_y, Matrix4x4::identity());
		mesh.generate_torus(large_radius, small_radius, divisions_x, divisions_y);
		mesh.color = color;
	}

	void Torus::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_torus_settings(*this, parent);
		ObjectSettings::build_parametric_surface_settings(*this, parent);
	}

	float Torus::intersect_with_ray(const Vector3& origin, const Vector3& ray)
	{
		return general_intersection(transformation.position, origin, ray); // TODO proper torus raycasting
	}

	void Torus::add_to_serializer(Serializer& serializer, int idx)
	{
		serializer.add_torus(*this, idx);
	}

	std::vector<ObjectHandle> Torus::clone() const
	{
		std::vector<ObjectHandle> result(1);
		auto handle = Object::create<Torus>();
		copy_basic_attributes_to(*handle);
		handle->divisions_x = divisions_x;
		handle->divisions_y = divisions_y;
		handle->large_radius = large_radius;
		handle->small_radius = small_radius;
		result[0] = std::move(handle);
		return result;
	}

	void Point::generate_renderable()
	{
		//set = PointSet::point(Matrix4x4::identity());
		set.generate_point();
		set.color = color;
	}

	void Point::build_specific_settings(ObjectSettingsWindow& parent)
	{
	}

	bool Point::is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const
	{
		Vector4 pos = transformation * Vector4::extend(get_const_position(), 1.0f);
		pos /= pos.w;
		return rect.contains(pos.x, pos.y);
	}

	void Point::add_to_serializer(Serializer& serializer, int idx)
	{
		serializer.add_point(*this, idx);
	}

	std::vector<ObjectHandle> Point::clone() const
	{
		std::vector<ObjectHandle> result(1);
		auto handle = Object::create<Point>();
		copy_basic_attributes_to(*handle);
		result[0] = std::move(handle);
		return result;
	}

	float Point::intersect_with_ray(const Vector3& origin, const Vector3& ray)
	{
		return general_intersection(transformation.position, origin, ray);
	}

	void ObjectCollection::build_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_collection_settings(*this, parent);
	}

	void BezierC0Curve::generate_renderable()
	{
		std::vector<const Point*> point_ptr_vec(points.begin(), points.end());
		curve.generate_curve(point_ptr_vec, polyline_visible, curve_visible);
		curve.color = color;
		curve.polyline_color = polyline_color;
	}

	void BezierC0Curve::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_bezier_c0_curve_settings(*this, parent);
	}

	void BezierC0Curve::bind_with(Object& object)
	{
		Point* p = dynamic_cast<Point*>(&object);
		if (p != nullptr)
		{
			points.push_back(p);
			p->add_observer(*this);
			invalidate();
		}
	}

	void BezierC0Curve::remove_binding_with(Object& object)
	{
		for (auto it = points.begin(); it != points.end();) {
			if (*it == &object)
			{
				(*it)->remove_observer(*this);
				it = points.erase(it);
			}
			else it++;
		}
		invalidate();
	}

	void BezierC0Curve::add_to_serializer(Serializer& serializer, int idx)
	{
		serializer.add_bezier_c0_curve(*this, idx);
	}

	std::vector<ObjectHandle> BezierC0Curve::clone() const
	{
		std::vector<ObjectHandle> result;
		std::map<Object*, Object*> old_to_new;
		std::list<Point*> new_points;
		result.reserve(points.size() + 1);
		for (Point* point : points)
		{
			auto it = old_to_new.find(point);
			if (it == old_to_new.end()) // not found
			{
				result.push_back(std::move(point->clone()[0]));
				old_to_new[point] = result.back().get();
			}
			new_points.push_back(dynamic_cast<Point*>(old_to_new[point]));
		}
		auto handle = Object::create<BezierC0Curve>(new_points);
		copy_basic_attributes_to(*handle);
		handle->polyline_color = polyline_color;
		handle->polyline_visible = polyline_visible;
		handle->curve_visible = curve_visible;
		result.push_back(std::move(handle));
		return result;
	}


	void BezierC2Curve::generate_renderable()
	{
		if (points.size() <= 3)
		{
			bernstein_points->set_points({});
			curve.set_data({});
			return;
		}

		std::vector<Vector3> positions;
		positions.reserve(points.size());

		for (auto it = points.begin(); it != points.end(); ++it)
		{
			positions.push_back((*it)->get_const_position());
		}

		std::vector<Vector3> bezier(3 * (points.size() - 3) + 1);

		// to z �wicze�
		for (int i = 0; i < positions.size() - 3; ++i)
		{
			bezier[3 * i] = (positions[i] + positions[i + 2] + 4.0f * positions[i + 1]) / 6.0f;
			bezier[3 * i + 1] = (positions[i + 2] + 2.0f * positions[i + 1]) / 3.0f;
			bezier[3 * i + 2] = (2.0f * positions[i + 2] + positions[i + 1]) / 3.0f;
		}
		int i = positions.size() - 3;
		bezier[bezier.size() - 1] = (positions[i] + positions[i + 2] + 4.0f * positions[i + 1]) / 6.0f;

		curve.generate_curve(bezier, polyline_visible, curve_visible);
		curve.color = color;
		curve.polyline_color = polyline_color;

		bernstein_points->set_points(std::move(bezier));
	}

	void BezierC2Curve::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_bezier_c2_curve_settings(*this, parent);
	}

	void BezierC2Curve::bind_with(Object& object)
	{
		Point* p = dynamic_cast<Point*>(&object);
		if (p != nullptr)
		{
			points.push_back(p);
			p->add_observer(*this);
			invalidate();
		}
	}

	void BezierC2Curve::remove_binding_with(Object& object)
	{
		for (auto it = points.begin(); it != points.end();) {
			if (*it == &object)
			{
				(*it)->remove_observer(*this);
				it = points.erase(it);
			}
			else it++;
		}
		invalidate();
	}

	void BezierC2Curve::on_child_move(int idx, const Vector3& prev_position, const Vector3& new_position)
	{
		const int this_idx = idx / 3 + 1; // BUG: przy przesuwaniu punktu beziera, jak krzywa zosta�a stworzona przez kolejne tworzenie punkt�w (zaznaczony obiekt krzywej), krzywa chaotycznie lata
		Vector3 middle = 0.5f * (points[this_idx]->get_const_position() + points[this_idx + 1]->get_const_position()); // maybe ->transformation.position will be better?
		Vector3 p1 = middle + 3.0f * (new_position - middle);
		Vector3 p2 = middle - 3.0f * (new_position - middle);
		auto vec = new_position - prev_position;
		switch (idx % 3)
		{
		case 0: {
			points[this_idx]->transformation.position += 1.5f * vec;
			points[this_idx]->invalidate_observers();
			break;
		}
		case 1: {
			switch (behaviour)
			{
			case DeBoorPointsBehaviour::RotateAroundCenter:
				points[this_idx]->transformation.position = p1;
				points[this_idx + 1]->transformation.position = p2;
				break;
			case DeBoorPointsBehaviour::MoveAdjacent:
				points[this_idx]->transformation.position += vec;
				points[this_idx + 1]->transformation.position += vec;
				break;
			}
			points[this_idx]->invalidate_observers();
			points[this_idx + 1]->invalidate_observers();
			break;
		}
		case 2: {
			switch (behaviour)
			{
			case DeBoorPointsBehaviour::RotateAroundCenter:
				points[this_idx]->transformation.position = p2;
				points[this_idx + 1]->transformation.position = p1;

				break;
			case DeBoorPointsBehaviour::MoveAdjacent:
				points[this_idx]->transformation.position += vec;
				points[this_idx + 1]->transformation.position += vec;
				break;
			}
			points[this_idx]->invalidate_observers();
			points[this_idx + 1]->invalidate_observers();
			break;
		}
		}
		invalidate();
	}

	void BezierC2Curve::add_to_serializer(Serializer& serializer, int idx)
	{
		serializer.add_bezier_c2_curve(*this, idx);
	}

	std::vector<ObjectHandle> BezierC2Curve::clone() const
	{
		std::vector<ObjectHandle> result;
		std::map<Object*, Object*> old_to_new;
		std::vector<Point*> new_points;
		result.reserve(points.size() + 2);
		for (Point* point : points)
		{
			auto it = old_to_new.find(point);
			if (it == old_to_new.end()) // not found
			{
				result.push_back(std::move(point->clone()[0]));
				old_to_new[point] = result.back().get();
			}
			new_points.push_back(dynamic_cast<Point*>(old_to_new[point]));
		}
		auto handle = Object::create<BezierC2Curve>(new_points);
		copy_basic_attributes_to(*handle);
		handle->polyline_color = polyline_color;
		handle->polyline_visible = polyline_visible;
		handle->curve_visible = curve_visible;
		handle->behaviour = behaviour;
		result.push_back(std::move(handle));
		result.push_back(handle->make_linked_bernstein_points());
		return result;
	}

	void PointCollection::generate_renderable()
	{
		set.set_data(points);
		set.color = color;
	}

	float PointCollection::intersect_with_ray(const Vector3& origin, const Vector3& ray)
	{
		int min_i = -1;
		float min_dist = INFINITY;
		for (int i = 0; i < points.size(); ++i)
		{
			float dist = general_intersection(points[i], origin, ray);
			if (isnan(dist)) continue;
			if (dist < min_dist)
			{
				min_i = i;
				min_dist = dist;
			}
		}
		if (min_i == -1) return NAN;

		current_idx = min_i;
		transformation.position = points[min_i];
		return min_dist;
	}

	std::vector<ObjectHandle> PointCollection::clone() const
	{
		return std::vector<ObjectHandle>(); // PointCollection can't be cloned independently
	}

	void InterpolationSpline::generate_renderable()
	{
		curve.color = color;
		curve.polyline_color = polyline_color;

		if (points.size() <= 1)
		{
			curve.set_data({});
			return;
		}

		std::vector<Vector3> positions;
		positions.reserve(points.size());
		Vector3 prev_pos = Vector3::nan();

		for (auto it = points.begin(); it != points.end(); ++it)
		{
			const Vector3 current_pos = (*it)->get_const_position();
			if (current_pos != prev_pos)
				positions.push_back(current_pos);
			prev_pos = current_pos;
		}

		if (positions.size() <= 2)
		{
			std::vector<Vector3> positions_to_curve(4);
			positions_to_curve[0] = positions[0];
			if (positions.size() == 2)
				positions_to_curve[1] = positions[1];
			else
				positions_to_curve[1] = Vector3::nan();
			positions_to_curve[3] = positions_to_curve[2] = Vector3::nan();
			curve.generate_curve(positions_to_curve, polyline_visible, curve_visible);
			return;
		}

		int n = positions.size() - 2;
		TridiagonalLinearEquationSystem<Vector3> eqs(n);
		for (int i = 0; i < n; ++i)
		{
			eqs.main_diagonal_element(i) = 2.0f;
		}
		for (int i = 1; i < n; ++i)
		{
			Vector3 di = positions[i + 2] - positions[i + 1];
			Vector3 dim1 = positions[i + 1] - positions[i];
			eqs.lower_diagonal_element(i) = dim1.length() / (dim1.length() + di.length());
		}
		for (int i = 0; i < n - 1; ++i)
		{
			Vector3 di = positions[i + 2] - positions[i + 1];
			Vector3 dim1 = positions[i + 1] - positions[i];
			eqs.upper_diagonal_element(i) = di.length() / (dim1.length() + di.length());
		}
		for (int i = 0; i < n; ++i)
		{
			Vector3 di = positions[i + 2] - positions[i + 1];
			Vector3 dim1 = positions[i + 1] - positions[i];
			eqs.free_term(i) = 3 * (di / di.length() - dim1 / dim1.length()) / (dim1.length() + di.length());
		}

		auto solution = eqs.solve();

		std::vector<Vector3> bezier(3 * (positions.size() - 1) + 1);
		std::vector<Vector3> b(positions.size() - 1);
		std::vector<Vector3> c(positions.size());
		for (int i = 0; i < positions.size() - 1; ++i)
		{
			c[i] = i == 0 ? Vector3{ 0.0f, 0.0f, 0.0f } : solution[i - 1];
		}
		c[positions.size() - 1] = Vector3{ 0.0f,0.0f,0.0f };
		for (int i = 0; i < positions.size() - 1; ++i)
		{
			Vector3 di = positions[i + 1] - positions[i];
			float dil = di.length();
			b[i] = di / dil - dil * (2.0f * c[i] + c[i + 1]) / 3.0f;
		}
		for (int i = 0; i < positions.size() - 1; ++i)
		{
			Vector3 di = positions[i + 1] - positions[i];
			float dil = di.length();
			bezier[3 * i] = positions[i];
			bezier[3 * i + 1] = positions[i] + dil * b[i] / 3.0f;
			bezier[3 * i + 2] = positions[i] + 2.0f * dil * b[i] / 3.0f + dil * dil * c[i] / 3.0f;
		}
		bezier[bezier.size() - 1] = positions[positions.size() - 1];

		curve.generate_curve(bezier, polyline_visible, curve_visible);
	}

	void InterpolationSpline::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_interpolation_spline_curve_settings(*this, parent);
	}

	void InterpolationSpline::bind_with(Object& object)
	{
		Point* p = dynamic_cast<Point*>(&object);
		if (p != nullptr)
		{
			points.push_back(p);
			p->add_observer(*this);
			invalidate();
		}
	}

	void InterpolationSpline::remove_binding_with(Object& object)
	{
		for (auto it = points.begin(); it != points.end();) {
			if (*it == &object)
			{
				(*it)->remove_observer(*this);
				it = points.erase(it);
			}
			else it++;
		}
		invalidate();
	}

	void InterpolationSpline::add_to_serializer(Serializer& serializer, int idx)
	{
		serializer.add_interpolation_spline(*this, idx);
	}

	std::vector<ObjectHandle> InterpolationSpline::clone() const
	{
		std::vector<ObjectHandle> result;
		std::map<Object*, Object*> old_to_new;
		std::vector<Point*> new_points;
		result.reserve(points.size() + 1);
		for (Point* point : points)
		{
			auto it = old_to_new.find(point);
			if (it == old_to_new.end()) // not found
			{
				result.push_back(std::move(point->clone()[0]));
				old_to_new[point] = result.back().get();
			}
			new_points.push_back(dynamic_cast<Point*>(old_to_new[point]));
		}
		auto handle = Object::create<InterpolationSpline>(new_points);
		copy_basic_attributes_to(*handle);
		handle->polyline_color = polyline_color;
		handle->polyline_visible = polyline_visible;
		handle->curve_visible = curve_visible;
		result.push_back(std::move(handle));
		return result;
	}

	void BicubicC0BezierSurface::generate_renderable()
	{
		std::vector<Vector3> positions;
		positions.reserve(points.size());

		for (auto it = points.begin(); it != points.end(); ++it)
		{
			positions.push_back((*it)->get_const_position());
		}

		surf.set_data(positions, patches_x, patches_y);
		surf.color = color;
		surf.contour_color = contour_color;
		surf.draw_contour = contour_visible;
		surf.draw_patch = patch_visible;
	}

	void BicubicC0BezierSurface::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_bicubic_c0_bezier_surface_settings(*this, parent);
		ObjectSettings::build_parametric_surface_settings(*this, parent);
	}

	void BicubicC0BezierSurface::on_delete()
	{
		for (auto* p : points)
			p->decrement_persistence();
	}

	BicubicC0BezierSurface& BicubicC0BezierSurface::create_and_add(ObjectController& controller, const Vector3& cursor_pos, int patches_x, int patches_y, float dist_between_points_x, float dist_between_points_y)
	{
		int points_x = 3 * patches_x + 1;
		int points_y = 3 * patches_y + 1;

		std::vector<ObjectHandle> points(points_x * points_y);
		std::vector<Point*> point_ptrs(points.size());

		for (int i = 0; i < points.size(); ++i)
		{
			int x = i % points_x, y = i / points_x;
			points[i] = Object::create<Point>();
			points[i]->transformation.position = { cursor_pos.x + dist_between_points_x * x, cursor_pos.y, cursor_pos.z + dist_between_points_y * y };
			points[i]->update_renderable_matrix();

			point_ptrs[i] = dynamic_cast<Point*>(points[i].get());
		}

		auto patch = Object::create<BicubicC0BezierSurface>(point_ptrs, patches_x, patches_y, false);
		patch->real_children = patch->children = points.size();

		auto& patch_ref = *patch;

		controller.add_object(std::move(patch));
		for (auto& obj : points)
			controller.add_object(std::move(obj));

		return patch_ref;
	}

	BicubicC0BezierSurface& BicubicC0BezierSurface::create_cylinder_and_add(ObjectController& controller, const Vector3& cursor_pos, int patches_x, int patches_y, float radius, float dist_between_points_y)
	{
		int points_x = 3 * patches_x + 1;
		int points_y = 3 * patches_y + 1;

		std::vector<ObjectHandle> points((points_x - 1) * points_y);
		std::vector<Point*> point_ptrs(points_x * points_y);

		const float angle_step = 2 * PI / patches_x;
		const float k = patches_x == 2 ? (4.0f / 3.0f) : (8.0f / 3.0f * (cosf(angle_step * 0.5f) - 0.5f - 0.5f * cosf(angle_step)) / sinf(angle_step));

		for (int i = 0; i < points.size(); ++i)
		{
			points[i] = Object::create<Point>();
		}
		for (int i = 0; i < point_ptrs.size(); ++i)
		{
			int x = i % points_x, y = i / points_x;
			if (x == points_x - 1)
				x = 0;
			point_ptrs[i] = dynamic_cast<Point*>(points[x + y * (points_x - 1)].get());
		}

		for (int x = 0; x < patches_x; ++x)
		{
			const float angle = angle_step * x;
			const float sin = sinf(angle), cos = cosf(angle);

			Vector3 on_circle = { radius * cos, radius * sin, 0.0f };
			Vector3 tangent = { -radius * sin, radius * cos,0.0f };

			for (int y = 0; y < points_y; ++y)
			{
				const Vector3 base_point = Vector3{ cursor_pos.x,cursor_pos.y,cursor_pos.z + dist_between_points_y * y } + on_circle;
				points[3 * x + y * (points_x - 1)]->transformation.position = base_point;
				points[(3 * x + 1) + y * (points_x - 1)]->transformation.position = base_point + k * tangent;
				if (x == 0)
					points[(3 * patches_x - 1) + y * (points_x - 1)]->transformation.position = base_point - k * tangent;
				else
					points[(3 * x - 1) + y * (points_x - 1)]->transformation.position = base_point - k * tangent;
			}
		}

		for (int i = 0; i < points.size(); ++i)
		{
			points[i]->update_renderable_matrix();
		}

		auto patch = Object::create<BicubicC0BezierSurface>(point_ptrs, patches_x, patches_y, true);
		patch->real_children = patch->children = points.size();

		auto& patch_ref = *patch;

		controller.add_object(std::move(patch));
		for (auto& obj : points)
			controller.add_object(std::move(obj));

		return patch_ref;
	}

	void BicubicC0BezierSurface::add_to_serializer(Serializer& serializer, int idx)
	{
		serializer.add_bicubic_bezier_c0_surface(*this, idx);
	}

	void BicubicC0BezierSurface::add_boundary_points(Graph<Point*, PatchEdge>& graph)
	{
		int points_x = 3 * patches_x + 1;
		int points_y = 3 * patches_y + 1;
		for (int i = 0; i < points_x - 1; i += 3)
		{
			graph.add_edge(points[i], points[i + 3], PatchEdge::from_surface_array(points, *this, i, 1, points_x));
		}
		for (int j = 0; j < points_y - 1; j += 3)
		{
			graph.add_edge(points[points_x - 1 + points_x * j], points[points_x - 1 + points_x * (j + 3)], PatchEdge::from_surface_array(points, *this, points_x - 1 + points_x * j, points_x, -1));
		}
		for (int i = points_x - 1; i > 0; i -= 3)
		{
			graph.add_edge(points[i + points_x * (points_y - 1)], points[i - 3 + points_x * (points_y - 1)], PatchEdge::from_surface_array(points, *this, i + points_x * (points_y - 1), -1, -points_x));
		}
		for (int j = points_y - 1; j > 0; j -= 3)
		{
			graph.add_edge(points[j * points_x], points[(j - 3) * points_x], PatchEdge::from_surface_array(points, *this, j * points_x, -points_x, 1));
		}
	}

	Vector3 BicubicC0BezierSurface::evaluate(float u, float v) const
	{
		float uif, vif;
		float uu = modf(u, &uif), vv = modf(v, &vif);

		int ui = 3 * static_cast<int>(uif), vi = 3 * static_cast<int>(vif);

		int points_x = 3 * patches_x + 1,
			points_y = 3 * patches_y + 1;

		if (uif < 0)
		{
			uu -= uif;
			ui = 0;
		}
		else if (uif >= patches_x)
		{
			uu += 1.0f + uif - patches_x;
			ui = points_x - 4;
		}

		if (vif < 0)
		{
			vv -= vif;
			vi = 0;
		}
		if (vif >= patches_y)
		{
			vv = 1.0f + vif - patches_y;
			vi = points_y - 4;
		}

		Vector3 p[4];
		for (int i = 0; i < 4; ++i)
		{
			auto b0 = points[(vi + i) * points_x + ui]->transformation.position,
				b1 = points[(vi + i) * points_x + ui + 1]->transformation.position,
				b2 = points[(vi + i) * points_x + ui + 2]->transformation.position,
				b3 = points[(vi + i) * points_x + ui + 3]->transformation.position;

			b0 = (1 - uu) * b0 + uu * b1;
			b1 = (1 - uu) * b1 + uu * b2;
			b2 = (1 - uu) * b2 + uu * b3;

			b0 = (1 - uu) * b0 + uu * b1;
			b1 = (1 - uu) * b1 + uu * b2;

			p[i] = (1 - uu) * b0 + uu * b1;
		}

		p[0] = (1 - vv) * p[0] + vv * p[1];
		p[1] = (1 - vv) * p[1] + vv * p[2];
		p[2] = (1 - vv) * p[2] + vv * p[3];

		p[0] = (1 - vv) * p[0] + vv * p[1];
		p[1] = (1 - vv) * p[1] + vv * p[2];

		p[0] = (1 - vv) * p[0] + vv * p[1];

		return p[0];
	}

	Vector3 BicubicC0BezierSurface::normal(float u, float v) const
	{
		return normalize(cross(du(u, v), dv(u, v)));
	}

	Vector3 BicubicC0BezierSurface::du(float u, float v) const
	{
		float uif, vif;
		float uu = modf(u, &uif), vv = modf(v, &vif);

		int ui = 3 * static_cast<int>(uif), vi = 3 * static_cast<int>(vif);

		int points_x = 3 * patches_x + 1,
			points_y = 3 * patches_y + 1;

		if (uif < 0)
		{
			uu -= uif;
			ui = 0;
		}
		else if (uif >= patches_x)
		{
			uu += 1.0f + uif - patches_x;
			ui = points_x - 4;
		}

		if (vif < 0)
		{
			vv -= vif;
			vi = 0;
		}
		if (vif >= patches_y)
		{
			vv = 1.0f + vif - patches_y;
			vi = points_y - 4;
		}

		Vector3 p[4];
		for (int i = 0; i < 4; ++i)
		{
			auto b0 = points[(vi + i) * points_x + ui]->transformation.position,
				b1 = points[(vi + i) * points_x + ui + 1]->transformation.position,
				b2 = points[(vi + i) * points_x + ui + 2]->transformation.position,
				b3 = points[(vi + i) * points_x + ui + 3]->transformation.position;

			// derivatives instead of first de Casteljau step
			b0 = 3.0f * (b1 - b0);
			b1 = 3.0f * (b2 - b1);
			b2 = 3.0f * (b3 - b2);

			b0 = (1 - uu) * b0 + uu * b1;
			b1 = (1 - uu) * b1 + uu * b2;

			p[i] = (1 - uu) * b0 + uu * b1;
		}

		p[0] = (1 - vv) * p[0] + vv * p[1];
		p[1] = (1 - vv) * p[1] + vv * p[2];
		p[2] = (1 - vv) * p[2] + vv * p[3];

		p[0] = (1 - vv) * p[0] + vv * p[1];
		p[1] = (1 - vv) * p[1] + vv * p[2];

		p[0] = (1 - vv) * p[0] + vv * p[1];

		return p[0];
	}

	Vector3 BicubicC0BezierSurface::dv(float u, float v) const
	{
		float uif, vif;
		float uu = modf(u, &uif), vv = modf(v, &vif);

		int ui = 3 * static_cast<int>(uif), vi = 3 * static_cast<int>(vif);

		int points_x = 3 * patches_x + 1,
			points_y = 3 * patches_y + 1;

		if (uif < 0)
		{
			uu -= uif;
			ui = 0;
		}
		else if (uif >= patches_x)
		{
			uu += 1.0f + uif - patches_x;
			ui = points_x - 4;
		}

		if (vif < 0)
		{
			vv -= vif;
			vi = 0;
		}
		if (vif >= patches_y)
		{
			vv = 1.0f + vif - patches_y;
			vi = points_y - 4;
		}

		Vector3 p[4];
		for (int i = 0; i < 4; ++i)
		{
			auto b0 = points[(vi + i) * points_x + ui]->transformation.position,
				b1 = points[(vi + i) * points_x + ui + 1]->transformation.position,
				b2 = points[(vi + i) * points_x + ui + 2]->transformation.position,
				b3 = points[(vi + i) * points_x + ui + 3]->transformation.position;

			b0 = (1 - uu) * b0 + uu * b1;
			b1 = (1 - uu) * b1 + uu * b2;
			b2 = (1 - uu) * b2 + uu * b3;

			b0 = (1 - uu) * b0 + uu * b1;
			b1 = (1 - uu) * b1 + uu * b2;

			p[i] = (1 - uu) * b0 + uu * b1;
		}

		// derivatives instead of first de Casteljau step
		p[0] = 3.0f * (p[1] - p[0]);
		p[1] = 3.0f * (p[2] - p[1]);
		p[2] = 3.0f * (p[3] - p[2]);

		p[0] = (1 - vv) * p[0] + vv * p[1];
		p[1] = (1 - vv) * p[1] + vv * p[2];

		p[0] = (1 - vv) * p[0] + vv * p[1];

		return p[0];
	}

	std::vector<ObjectHandle> BicubicC0BezierSurface::clone() const
	{
		std::vector<ObjectHandle> result;
		std::map<Object*, Object*> old_to_new;
		std::vector<Point*> new_points;
		result.reserve(points.size() + 1);
		new_points.reserve(points.size());
		for (Point* point : points)
		{
			auto it = old_to_new.find(point);
			if (it == old_to_new.end()) // not found
			{
				result.push_back(std::move(point->clone()[0]));
				old_to_new[point] = result.back().get();
			}
			new_points.push_back(dynamic_cast<Point*>(old_to_new[point]));
		}
		auto handle = Object::create<BicubicC0BezierSurface>(new_points, patches_x, patches_y, cylinder);
		copy_basic_attributes_to(*handle);
		handle->contour_color = contour_color;
		handle->contour_visible = contour_visible;
		handle->patch_visible = patch_visible;
		result.push_back(std::move(handle));
		return result;
	}

	/*std::vector<ObjectHandle> BicubicC0BezierSurface::clone_subdivide_patches(int subdivisions_x, int subdivisions_y) const
	{
		auto new_patches_x = patches_x * subdivisions_x,
			new_patches_y = patches_y * subdivisions_y;
		auto new_points_x = 3 * new_patches_x + 1,
			new_points_y = 3 * new_patches_y + 1;

		std::vector<ObjectHandle> result;
		std::map<Object*, Object*> old_to_new;
		std::vector<Point*> new_points;

		result.reserve(new_points_x * new_points_y + 1);
		new_points.reserve(new_points_x * new_points_y);

		for (int i = 0; i < new_patches_x; ++i)
		{
			for (int j = 0; j < new_patches_y; ++j)
			{
				
			}
		}
	}*/

	void BicubicC0BezierSurfacePreview::generate_renderable()
	{
		int points_x = 3 * patches_x + 1;
		int points_y = 3 * patches_y + 1;

		std::vector<Vector3> points(points_x * points_y);

		if (cylinder)
		{
			const float dist_between_points_y = width_y / (points_y - 1);
			const float angle_step = 2 * PI / patches_x;
			const float k = patches_x == 2 ? (4.0f / 3.0f) : (8.0f / 3.0f * (cosf(angle_step * 0.5f) - 0.5f - 0.5f * cosf(angle_step)) / sinf(angle_step));
			for (int x = 0; x < patches_x; ++x)
			{
				const float angle = angle_step * x;
				const float sin = sinf(angle), cos = cosf(angle);

				Vector3 on_circle = { radius * cos, radius * sin, 0.0f };
				Vector3 tangent = { -radius * sin, radius * cos,0.0f };

				for (int y = 0; y < points_y; ++y)
				{
					points[3 * x + y * points_x] = Vector3{ position.x,position.y,position.z + dist_between_points_y * y } + on_circle;
					points[(3 * x + 1) + y * points_x] = points[3 * x + y * points_x] + k * tangent;
					if (x == 0)
					{
						points[(3 * patches_x - 1) + y * points_x] = points[3 * x + y * points_x] - k * tangent;
						points[3 * patches_x + y * points_x] = points[3 * x + y * points_x];
					}
					else
						points[(3 * x - 1) + y * points_x] = points[3 * x + y * points_x] - k * tangent;
				}
			}
		}
		else
		{
			const float dist_between_points_x = width_x / (points_x - 1);
			const float dist_between_points_y = width_y / (points_y - 1);

			for (int i = 0; i < points.size(); ++i)
			{
				const int x = i % points_x, y = i / points_x;
				points[i] = { position.x + dist_between_points_x * x, position.y, position.z + dist_between_points_y * y };
			}
		}

		surf.reset_ebos();
		surf.set_data(points, patches_x, patches_y);
	}

	void BicubicC0BezierSurfacePreview::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_bicubic_c0_bezier_surface_preview_settings(*this, parent);
	}

	std::vector<ObjectHandle> BicubicC0BezierSurfacePreview::clone() const
	{
		return std::vector<ObjectHandle>(); // BicubicC0BezierSurfacePreview can't be cloned
	}

	void BicubicC2BezierSurface::generate_renderable()
	{
		std::vector<Vector3> positions;
		positions.reserve(points.size());

		for (auto it = points.begin(); it != points.end(); ++it)
		{
			positions.push_back((*it)->get_const_position());
		}

		surf.set_data(positions, patches_x, patches_y);
		surf.color = color;
		surf.contour_color = contour_color;
		surf.draw_contour = contour_visible;
		surf.draw_patch = patch_visible;
	}

	void BicubicC2BezierSurface::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_bicubic_c2_bezier_surface_settings(*this, parent);
		ObjectSettings::build_parametric_surface_settings(*this, parent);
	}

	void BicubicC2BezierSurface::on_delete()
	{
		for (auto* p : points)
			p->decrement_persistence();
	}

	BicubicC2BezierSurface& BicubicC2BezierSurface::create_and_add(ObjectController& controller, const Vector3& cursor_pos, int patches_x, int patches_y, float dist_between_points_x, float dist_between_points_y)
	{
		int points_x = patches_x + 3;
		int points_y = patches_y + 3;

		std::vector<ObjectHandle> points(points_x * points_y);
		std::vector<Point*> point_ptrs(points.size());

		for (int i = 0; i < points.size(); ++i)
		{
			int x = i % points_x, y = i / points_x;
			points[i] = Object::create<Point>();
			points[i]->transformation.position = { cursor_pos.x + dist_between_points_x * x, cursor_pos.y, cursor_pos.z + dist_between_points_y * y };
			points[i]->update_renderable_matrix();

			point_ptrs[i] = dynamic_cast<Point*>(points[i].get());
		}

		auto patch = Object::create<BicubicC2BezierSurface>(point_ptrs, patches_x, patches_y, false);
		patch->real_children = patch->children = points.size();

		auto& patch_ref = *patch;

		controller.add_object(std::move(patch));
		for (auto& obj : points)
			controller.add_object(std::move(obj));

		return patch_ref;
	}

	BicubicC2BezierSurface& BicubicC2BezierSurface::create_cylinder_and_add(ObjectController& controller, const Vector3& cursor_pos, int patches_x, int patches_y, float radius, float dist_between_points_y)
	{
		int points_y = patches_y + 3;
		int points_x = patches_x;

		std::vector<ObjectHandle> points(points_x * points_y);
		std::vector<Point*> point_ptrs((points_x + 3) * points_y);

		for (int i = 0; i < points.size(); ++i)
		{
			points[i] = Object::create<Point>();
		}
		for (int i = 0; i < point_ptrs.size(); ++i)
		{
			int x = i % (points_x + 3), y = i / (points_x + 3);
			if (x >= points_x)
				x -= points_x;
			point_ptrs[i] = dynamic_cast<Point*>(points[x + y * points_x].get());
		}

		const float angle_step = 2 * PI / points_x;
		const float actual_radius = 3.0f * radius / (cosf(angle_step) + 2.0f);

		for (int x = 0; x < points_x; ++x)
		{
			const float angle = angle_step * x;
			const float sin = sinf(angle), cos = cosf(angle);

			Vector3 on_circle = { actual_radius * cos, actual_radius * sin, 0.0f };

			for (int y = 0; y < points_y; ++y)
			{
				const Vector3 base_point = Vector3{ cursor_pos.x,cursor_pos.y,cursor_pos.z + dist_between_points_y * y } + on_circle;
				points[x + y * points_x]->transformation.position = base_point;
			}
		}

		for (int i = 0; i < points.size(); ++i)
		{
			points[i]->update_renderable_matrix();
		}

		auto patch = Object::create<BicubicC2BezierSurface>(point_ptrs, patches_x, patches_y, true);
		patch->real_children = patch->children = points.size();

		auto& patch_ref = *patch;

		controller.add_object(std::move(patch));
		for (auto& obj : points)
			controller.add_object(std::move(obj));

		return patch_ref;
	}

	void BicubicC2BezierSurface::add_to_serializer(Serializer& serializer, int idx)
	{
		serializer.add_bicubic_bezier_c2_surface(*this, idx);
	}

	Vector3 BicubicC2BezierSurface::evaluate(float u, float v) const
	{
		float uif, vif;
		float uu = modf(u, &uif), vv = modf(v, &vif);

		int ui = static_cast<int>(uif), vi = static_cast<int>(vif);

		int points_x = patches_x + 3,
			points_y = patches_y + 3;

		if (uif < 0)
		{
			uu -= uif;
			ui = 0;
		}
		else if (uif >= patches_x)
		{
			uu += 1.0f + uif - patches_x;
			ui = points_x - 4;
		}

		if (vif < 0)
		{
			vv -= vif;
			vi = 0;
		}
		if (vif >= patches_y)
		{
			vv = 1.0f + vif - patches_y;
			vi = points_y - 4;
		}

		Vector3 p[4];

		{
			// de Boor basis for uu
			const float n01 = 1;
			const float n10 = n01 * (1 - uu), n11 = n01 * uu;
			const float n2m1 = n10 * (1 - uu) / 2.0f, n20 = n10 * (uu + 1) / 2.0f + n11 * (2 - uu) / 2.0f, n21 = n11 * uu / 2.0f;
			const float n3m2 = n2m1 * (1 - uu) / 3.0f, n3m1 = n2m1 * (uu + 2) / 3.0f + n20 * (2 - uu) / 3.0f, n30 = n20 * (uu + 1) / 3.0f + n21 * (3 - uu) / 3.0f, n31 = n21 * uu / 3.0f;

			for (int i = 0; i < 4; ++i)
			{
				auto b0 = points[(vi + i) * points_x + ui]->transformation.position,
					b1 = points[(vi + i) * points_x + ui + 1]->transformation.position,
					b2 = points[(vi + i) * points_x + ui + 2]->transformation.position,
					b3 = points[(vi + i) * points_x + ui + 3]->transformation.position;

				p[i] = n3m2 * b0 + n3m1 * b1 + n30 * b2 + n31 * b3;
			}
		}

		{
			// de Boor basis for vv
			const float n01 = 1;
			const float n10 = n01 * (1 - vv), n11 = n01 * vv;
			const float n2m1 = n10 * (1 - vv) / 2.0f, n20 = n10 * (vv + 1) / 2.0f + n11 * (2 - vv) / 2.0f, n21 = n11 * vv / 2.0f;
			const float n3m2 = n2m1 * (1 - vv) / 3.0f, n3m1 = n2m1 * (vv + 2) / 3.0f + n20 * (2 - vv) / 3.0f, n30 = n20 * (vv + 1) / 3.0f + n21 * (3 - vv) / 3.0f, n31 = n21 * vv / 3.0f;

			return n3m2 * p[0] + n3m1 * p[1] + n30 * p[2] + n31 * p[3];
		}
	}

	Vector3 BicubicC2BezierSurface::normal(float u, float v) const
	{
		return normalize(cross(du(u, v), dv(u, v)));
	}

	Vector3 BicubicC2BezierSurface::du(float u, float v) const
	{
		float uif, vif;
		float uu = modf(u, &uif), vv = modf(v, &vif);

		int ui = static_cast<int>(uif), vi = static_cast<int>(vif);

		int points_x = patches_x + 3,
			points_y = patches_y + 3;

		if (uif < 0)
		{
			uu -= uif;
			ui = 0;
		}
		else if (uif >= patches_x)
		{
			uu += 1.0f + uif - patches_x;
			ui = points_x - 4;
		}

		if (vif < 0)
		{
			vv -= vif;
			vi = 0;
		}
		if (vif >= patches_y)
		{
			vv = 1.0f + vif - patches_y;
			vi = points_y - 4;
		}

		Vector3 p[4];

		{
			// de Boor basis for uu
			const float n01 = 1;
			const float n10 = n01 * (1 - uu), n11 = n01 * uu;
			const float n2m1 = n10 * (1 - uu) / 2.0f, n20 = n10 * (uu + 1) / 2.0f + n11 * (2 - uu) / 2.0f, n21 = n11 * uu / 2.0f;
			//const float n3m2 = n2m1 * (1 - uu) / 3.0f, n3m1 = n2m1 * (uu + 2) / 3.0f + n20 * (2 - uu) / 3.0f, n30 = n20 * (uu + 1) / 3.0f + n21 * (3 - uu) / 3.0f, n31 = n21 * uu / 3.0f;

			for (int i = 0; i < 4; ++i)
			{
				auto b0 = points[(vi + i) * points_x + ui]->transformation.position,
					b1 = points[(vi + i) * points_x + ui + 1]->transformation.position,
					b2 = points[(vi + i) * points_x + ui + 2]->transformation.position,
					b3 = points[(vi + i) * points_x + ui + 3]->transformation.position;

				// derivative
				p[i] = n2m1 * (b1 - b0) + n20 * (b2 - b1) + n21 * (b3 - b2);
			}
		}

		{
			// de Boor basis for vv
			const float n01 = 1;
			const float n10 = n01 * (1 - vv), n11 = n01 * vv;
			const float n2m1 = n10 * (1 - vv) / 2.0f, n20 = n10 * (vv + 1) / 2.0f + n11 * (2 - vv) / 2.0f, n21 = n11 * vv / 2.0f;
			const float n3m2 = n2m1 * (1 - vv) / 3.0f, n3m1 = n2m1 * (vv + 2) / 3.0f + n20 * (2 - vv) / 3.0f, n30 = n20 * (vv + 1) / 3.0f + n21 * (3 - vv) / 3.0f, n31 = n21 * vv / 3.0f;

			return n3m2 * p[0] + n3m1 * p[1] + n30 * p[2] + n31 * p[3];
		}
	}

	Vector3 BicubicC2BezierSurface::dv(float u, float v) const
	{
		float uif, vif;
		float uu = modf(u, &uif), vv = modf(v, &vif);

		int ui = static_cast<int>(uif), vi = static_cast<int>(vif);

		int points_x = patches_x + 3,
			points_y = patches_y + 3;

		if (uif < 0)
		{
			uu -= uif;
			ui = 0;
		}
		else if (uif >= patches_x)
		{
			uu += 1.0f + uif - patches_x;
			ui = points_x - 4;
		}

		if (vif < 0)
		{
			vv -= vif;
			vi = 0;
		}
		if (vif >= patches_y)
		{
			vv = 1.0f + vif - patches_y;
			vi = points_y - 4;
		}

		Vector3 p[4];

		{
			// de Boor basis for uu
			const float n01 = 1;
			const float n10 = n01 * (1 - uu), n11 = n01 * uu;
			const float n2m1 = n10 * (1 - uu) / 2.0f, n20 = n10 * (uu + 1) / 2.0f + n11 * (2 - uu) / 2.0f, n21 = n11 * uu / 2.0f;
			const float n3m2 = n2m1 * (1 - uu) / 3.0f, n3m1 = n2m1 * (uu + 2) / 3.0f + n20 * (2 - uu) / 3.0f, n30 = n20 * (uu + 1) / 3.0f + n21 * (3 - uu) / 3.0f, n31 = n21 * uu / 3.0f;

			for (int i = 0; i < 4; ++i)
			{
				auto b0 = points[(vi + i) * points_x + ui]->transformation.position,
					b1 = points[(vi + i) * points_x + ui + 1]->transformation.position,
					b2 = points[(vi + i) * points_x + ui + 2]->transformation.position,
					b3 = points[(vi + i) * points_x + ui + 3]->transformation.position;

				p[i] = n3m2 * b0 + n3m1 * b1 + n30 * b2 + n31 * b3;
			}
		}

		{
			// de Boor basis for vv
			const float n01 = 1;
			const float n10 = n01 * (1 - vv), n11 = n01 * vv;
			const float n2m1 = n10 * (1 - vv) / 2.0f, n20 = n10 * (vv + 1) / 2.0f + n11 * (2 - vv) / 2.0f, n21 = n11 * vv / 2.0f;
			//const float n3m2 = n2m1 * (1 - vv) / 3.0f, n3m1 = n2m1 * (vv + 2) / 3.0f + n20 * (2 - vv) / 3.0f, n30 = n20 * (vv + 1) / 3.0f + n21 * (3 - vv) / 3.0f, n31 = n21 * vv / 3.0f;

			// derivative
			return n2m1 * (p[1] - p[0]) + n20 * (p[2] - p[1]) + n21 * (p[3] - p[2]);
		}
	}

	std::vector<ObjectHandle> BicubicC2BezierSurface::clone() const
	{
		std::vector<ObjectHandle> result;
		std::map<Object*, Object*> old_to_new;
		std::vector<Point*> new_points;
		result.reserve(points.size() + 1);
		for (Point* point : points)
		{
			auto it = old_to_new.find(point);
			if (it == old_to_new.end()) // not found
			{
				result.push_back(std::move(point->clone()[0]));
				old_to_new[point] = result.back().get();
			}
			new_points.push_back(dynamic_cast<Point*>(old_to_new[point]));
		}
		auto handle = Object::create<BicubicC2BezierSurface>(new_points, patches_x, patches_y, cylinder);
		copy_basic_attributes_to(*handle);
		handle->contour_color = contour_color;
		handle->contour_visible = contour_visible;
		handle->patch_visible = patch_visible;
		result.push_back(std::move(handle));
		return result;
	}

	void BicubicC2BezierSurfacePreview::generate_renderable()
	{
		int points_x = patches_x + 3;
		int points_y = patches_y + 3;

		std::vector<Vector3> points(points_x * points_y);

		if (cylinder)
		{
			const float dist_between_points_y = width_y / patches_y;
			const float angle_step = 2 * PI / patches_x;
			const float actual_radius = 3.0f * radius / (cosf(angle_step) + 2.0f);
			for (int x = 0; x < points_x; ++x)
			{
				const float angle = angle_step * x;
				const float sin = sinf(angle), cos = cosf(angle);

				const Vector3 on_circle = { actual_radius * cos, actual_radius * sin, 0.0f };

				for (int y = 0; y < points_y; ++y)
				{
					points[x + y * points_x] = Vector3{ position.x,position.y,position.z + dist_between_points_y * y } + on_circle;
				}
			}
		}
		else
		{
			const float dist_between_points_x = width_x / patches_x;
			const float dist_between_points_y = width_y / patches_y;

			for (int i = 0; i < points.size(); ++i)
			{
				const int x = i % points_x, y = i / points_x;
				points[i] = { position.x + dist_between_points_x * x, position.y, position.z + dist_between_points_y * y };
			}
		}

		surf.reset_ebos();
		surf.set_data(points, patches_x, patches_y);
	}

	void BicubicC2BezierSurfacePreview::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_bicubic_c2_bezier_surface_preview_settings(*this, parent);
	}

	std::vector<ObjectHandle> BicubicC2BezierSurfacePreview::clone() const
	{
		return std::vector<ObjectHandle>(); // BicubicC2BezierSurfacePreview can't be cloned
	}

	void Gregory20ParamSurface::generate_renderable()
	{
		std::vector<PatchEdgePosition> patches(adjacent_patches.size());
		std::vector<PatchEdgePosition> first_patches(adjacent_patches.size()),
			second_patches(adjacent_patches.size());
		for (int i = 0; i < adjacent_patches.size(); ++i)
		{
			patches[i] = adjacent_patches[i].to_position();
			first_patches[i] = patches[i].bisect_get_first();
			second_patches[i] = patches[i].bisect_get_second();
		}

		std::vector<Vector3> points;
		points.reserve(20 * patches.size());

		std::vector<Vector3> auxiliary_points(patches.size());
		std::vector<Vector3> internal_to_first(patches.size()),
			internal_to_second(patches.size());
		for (int i = 0; i < patches.size(); ++i)
		{
			const auto& edge = second_patches[i].edge[0];
			const auto& second = second_patches[i].second[0];

			auxiliary_points[i] = 2.5f * edge - 1.5f * second;
		}
		Vector3 central = (auxiliary_points[0] + auxiliary_points[1] + auxiliary_points[2]) / 3.0f; // TODO: generalize above 3 patches
		for (int i = 0; i < patches.size(); ++i)
		{
			auxiliary_points[i] = (2.0f * auxiliary_points[i] + central) / 3.0f;
		}
		for (int i = 0; i < patches.size(); ++i)
		{
			int prev_i = i == 0 ? (patches.size() - 1) : (i - 1);
			int next_i = i == patches.size() - 1 ? 0 : (i + 1);
			//const auto c = central - auxiliary_points[i];
			//a = central - auxiliary_points[prev_i],
			//	b = auxiliary_points[next_i] - central,

			const auto g0 = second_patches[i].edge[1] - second_patches[i].edge[0],
				g2 = 0.5f * (auxiliary_points[next_i] - auxiliary_points[prev_i]);
			const auto& P0 = second_patches[i].edge[0];
			const auto P1 = 2 * second_patches[i].edge[0] - second_patches[i].edge[1];
			const auto& P2 = auxiliary_points[i];
			const auto& P3 = central;

			const auto base = P2 + (4.0f * P3 - 3.0f * P1 - P0) / (9.0f * 3.0f); // P2 + h(2/3)c(2/3)
			const auto plusMinus = (g0 / 3.0f + 2.0f / 3.0f * g2); // +- k(2/3)g(2/3)
			internal_to_first[i] = base - plusMinus;
			internal_to_second[i] = base + plusMinus;
		}

		int prev_i = patches.size() - 1;
		for (int i = 0; i < patches.size(); ++i)
		{
			auto prev_patch = second_patches[prev_i];
			auto current_patch = first_patches[i];

			points.push_back(prev_patch.edge[3]);
			points.push_back(prev_patch.edge[2]);
			points.push_back(prev_patch.edge[1]);
			points.push_back(prev_patch.edge[0]);

			points.push_back(current_patch.edge[1]);
			points.push_back(2.0f * current_patch.edge[1] - current_patch.second[1]);
			points.push_back(2.0f * prev_patch.edge[2] - prev_patch.second[2]);
			points.push_back(2.0f * prev_patch.edge[1] - prev_patch.second[1]);
			points.push_back(2.0f * prev_patch.edge[1] - prev_patch.second[1]);
			points.push_back(2.0f * prev_patch.edge[0] - prev_patch.second[0]);

			points.push_back(current_patch.edge[2]);
			points.push_back(2.0f * current_patch.edge[2] - current_patch.second[2]);
			points.push_back(2.0f * current_patch.edge[2] - current_patch.second[2]);
			points.push_back(internal_to_first[i]);
			points.push_back(internal_to_second[prev_i]);
			points.push_back(auxiliary_points[prev_i]);

			points.push_back(current_patch.edge[3]);
			points.push_back(2.0f * current_patch.edge[3] - current_patch.second[3]);
			points.push_back(auxiliary_points[i]);
			points.push_back(central);

			prev_i = i;
		}

		surf.draw_contour = contour_visible;
		surf.draw_patch = patch_visible;
		surf.color = color;
		surf.set_data(points);
	}

	void Gregory20ParamSurface::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_gregory_20_param_surface_settings(*this, parent);
		//ObjectSettings::build_parametric_surface_settings(*this, parent);
	}

	std::vector<ObjectHandle> Gregory20ParamSurface::clone() const
	{
		return std::vector<ObjectHandle>(); // Gregory20ParamSurface is a top-level structure (observes C0 surfaces) and thus can't be cloned (surfaces should have been cloned too)
	}

	PatchEdgePosition PatchEdgePosition::bisect_get_first() const
	{
		PatchEdgePosition e;
		e.edge[0] = edge[0];
		e.second[0] = second[0];

		e.edge[1] = 0.5f * (edge[0] + edge[1]);
		e.second[1] = 0.5f * (second[0] + second[1]);

		e.edge[2] = 0.25f * edge[0] + 0.5f * edge[1] + 0.25f * edge[2];
		e.second[2] = 0.25f * second[0] + 0.5f * second[1] + 0.25f * second[2];

		e.edge[3] = 0.125f * edge[0] + 0.375f * edge[1] + 0.375f * edge[2] + 0.125f * edge[3];
		e.second[3] = 0.125f * second[0] + 0.375f * second[1] + 0.375f * second[2] + 0.125f * second[3];

		return e;
	}
	PatchEdgePosition PatchEdgePosition::bisect_get_second() const
	{
		PatchEdgePosition e;
		e.edge[3] = edge[3];
		e.second[3] = second[3];

		e.edge[2] = 0.5f * (edge[2] + edge[3]);
		e.second[2] = 0.5f * (second[2] + second[3]);

		e.edge[1] = 0.25f * edge[1] + 0.5f * edge[2] + 0.25f * edge[3];
		e.second[1] = 0.25f * second[1] + 0.5f * second[2] + 0.25f * second[3];

		e.edge[0] = 0.125f * edge[0] + 0.375f * edge[1] + 0.375f * edge[2] + 0.125f * edge[3];
		e.second[0] = 0.125f * second[0] + 0.375f * second[1] + 0.375f * second[2] + 0.125f * second[3];

		return e;
	}

	PatchEdgePosition PatchEdge::to_position() const
	{
		PatchEdgePosition e;
		for (int i = 0; i < 4; ++i)
		{
			e.edge[i] = edge[i]->get_const_position();
			e.second[i] = second[i]->get_const_position();
		}
		return e;
	}

	PatchEdge PatchEdge::from_surface_array(const std::vector<Point*>& points, Object& patch, int idx_from, int step_to, int step_second)
	{
		PatchEdge e;
		const int idx_from_second = idx_from + step_second;
		for (int i = 0; i < 4; ++i)
		{
			e.edge[i] = points[idx_from + i * step_to];
			e.second[i] = points[idx_from_second + i * step_to];
			e.patch_ref = &patch;
		}
		return e;
	}

	void IntersectionCurve::generate_renderable()
	{
		std::vector<Vector3> points(uvs1.size());
		for (int i = 0; i < uvs1.size(); ++i)
			points[i] = surf2.evaluate(uvs2[i].x, uvs2[i].y);
		line.color = color;
		line.set_data(points);
	}

	void IntersectionCurve::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_intersection_curve_settings(*this, parent);
	}

	std::pair<std::vector<Object::Handle<Point>>, Object::Handle<InterpolationSpline>> IntersectionCurve::to_spline() const
	{
		std::vector<Object::Handle<Point>> points(uvs1.size());
		std::vector<Point*> point_ptrs(uvs1.size() + (line.looped ? 1 : 0));
		for (int i = 0; i < uvs1.size(); ++i)
		{
			points[i] = Object::create<Point>();
			points[i]->transformation.position = surf2.evaluate(uvs2[i].x, uvs2[i].y);
			point_ptrs[i] = points[i].get();
		}
		if (line.looped)
			point_ptrs[point_ptrs.size() - 1] = point_ptrs[0];
		return { std::move(points), Object::create<InterpolationSpline>(point_ptrs) };
	}

	inline void check_timeout(const std::clock_t& start_time)
	{
		if constexpr (ApplicationSettings::BREAK_ON_TIMEOUT)
		{
			auto current_time = std::clock();
			if (current_time - start_time > ApplicationSettings::COMPUTATION_TIMEOUT)
				throw TimeoutException();
		}
	}

	Object::Handle<IntersectionCurve> IntersectionCurve::intersect_surfaces(ParametricSurface& surf1, ParametricSurface& surf2, float step, size_t max_steps, const Vector2& uv1start, const Vector2& uv2start)
	{
		static constexpr float EPS = 1e-5f;

		//const auto xx1 = surf1.evaluate(uv1start.x, uv1start.y), xx2 = surf2.evaluate(uv2start.x, uv2start.y), xx3 = xx1 - xx2;
		if (!surf1.get_u_range().contains(uv1start.x)
			|| !surf1.get_v_range().contains(uv1start.y)
			|| !surf2.get_u_range().contains(uv2start.x)
			|| !surf2.get_v_range().contains(uv2start.y)
			|| (surf1.evaluate(uv1start.x, uv1start.y) - surf2.evaluate(uv2start.x, uv2start.y)).length() >= 1e-4f)
			throw CommonIntersectionPointNotFoundException();

		std::list<Vector2> uvs1, uvs2;
		//uvs1.reserve(max_steps);
		//uvs2.reserve(max_steps);

		uvs1.push_back(uv1start);
		uvs2.push_back(uv2start);

		bool inverse_direction = false;
		bool reverse_tangent_after_singular_point = false; // used by singular points handling
		bool singular_point_crossed = false;

		int step_count = 1;
		bool looped = false;

		while (step_count++ < max_steps)
		{
			//const auto idx = uvs1.size() - 1;
			const auto uv1 = inverse_direction ? uvs1.front() : uvs1.back(),
				uv2 = inverse_direction ? uvs2.front() : uvs2.back();

			const auto norm1 = surf1.normal(uv1.x, uv1.y), norm2 = surf2.normal(uv2.x, uv2.y);
			const auto cr = cross(norm1, norm2);

			if (cr.length() == 0.0f)
				throw new CommonIntersectionPointNotFoundException();

			auto tangent = normalize(cr);

			if (inverse_direction)
				tangent = -tangent;

			if (reverse_tangent_after_singular_point)
			{
				//perpendicular_direction = false;
				tangent = -tangent;//normalize(cross(norm1, tangent));
			}

			const auto P = surf1.evaluate(uv1.x, uv1.y);

			bool should_change_direction = false;

			// Newton iterations
			Vector4 F;
			Matrix4x4 jacobian;
			Vector4 sol = { uv1.x, uv1.y, uv2.x, uv2.y }, prev_sol;

			const auto start_time = std::clock();

			do
			{
				check_timeout(start_time);

				const auto P1 = surf1.evaluate(sol.x, sol.y), P2 = surf2.evaluate(sol.z, sol.w);
				F = Vector4::extend(P1 - P2, dot(P1 - P, tangent) - step);
				const auto dP1u = surf1.du(sol.x, sol.y), dP1v = surf1.dv(sol.x, sol.y),
					dP2u = -surf2.du(sol.z, sol.w), dP2v = -surf2.dv(sol.z, sol.w);

				jacobian.elem[0][0] = dP1u.x;
				jacobian.elem[0][1] = dP1v.x;
				jacobian.elem[0][2] = dP2u.x;
				jacobian.elem[0][3] = dP2v.x;

				jacobian.elem[1][0] = dP1u.y;
				jacobian.elem[1][1] = dP1v.y;
				jacobian.elem[1][2] = dP2u.y;
				jacobian.elem[1][3] = dP2v.y;

				jacobian.elem[2][0] = dP1u.z;
				jacobian.elem[2][1] = dP1v.z;
				jacobian.elem[2][2] = dP2u.z;
				jacobian.elem[2][3] = dP2v.z;

				jacobian.elem[3][0] = dot(dP1u, tangent);
				jacobian.elem[3][1] = dot(dP1v, tangent);
				jacobian.elem[3][2] = 0.0f;
				jacobian.elem[3][3] = 0.0f;

				prev_sol = sol;
				auto l = LinearEquationSystem4x4::solve(jacobian, -F);
				auto d = jacobian * l;
				//if ((d + F).length() > EPS)
					//THROW_EXCEPTION;
				if (!isfinite(l.x))
					throw CommonIntersectionPointNotFoundException();
				//return Object::create<IntersectionCurve>(surf1, surf2, uvs1, uvs2, looped);
				sol += l;
				if (!surf1.get_u_range().contains(sol.x) || !surf1.get_v_range().contains(sol.y)
					|| !surf2.get_u_range().contains(sol.z) || !surf2.get_v_range().contains(sol.w))
					break;
			} while (F.length() >= EPS);
			//while ((sol - prev_sol).length() >= EPS);

			if (!surf1.get_u_range().contains(sol.x))
			{
				if (surf1.u_wraps_at_v(sol.y))
					sol.x = surf1.get_u_range().wrap(sol.x);
				else
				{
					sol.x = surf1.get_u_range().clamp(sol.x);
					/*if (inverse_direction)
						break;
					inverse_direction = true;
					continue;*/
					should_change_direction = true;
				}
			}
			if (!surf1.get_v_range().contains(sol.y))
			{
				if (surf1.v_wraps_at_u(sol.x))
					sol.y = surf1.get_v_range().wrap(sol.y);
				else
				{
					sol.y = surf1.get_v_range().clamp(sol.y);
					/*if (inverse_direction)
						break;
					inverse_direction = true;
					continue;*/
					should_change_direction = true;
				}
			}
			if (!surf2.get_u_range().contains(sol.z))
			{
				if (surf2.u_wraps_at_v(sol.w))
					sol.z = surf2.get_u_range().wrap(sol.z);
				else
				{
					sol.z = surf2.get_u_range().clamp(sol.z);
					/*if (inverse_direction)
						break;
					inverse_direction = true;
					continue;*/
					should_change_direction = true;
				}
			}
			if (!surf2.get_v_range().contains(sol.w))
			{
				if (surf2.v_wraps_at_u(sol.z))
					sol.w = surf2.get_v_range().wrap(sol.w);
				else
				{
					sol.w = surf2.get_v_range().clamp(sol.w);
					/*if (inverse_direction)
						break;
					inverse_direction = true;
					continue;*/
					should_change_direction = true;
				}
			}

			/*	if (!surf1.get_u_range().contains(sol.x) || !surf1.get_v_range().contains(sol.y) || !surf2.get_u_range().contains(sol.z) || !surf2.get_v_range().contains(sol.w))
				{
					if (inverse_direction)
						break;
					inverse_direction = true;
					continue;
				}*/

				// check singular points
			if (uvs1.size() >= 2)
			{
				const auto pprev_uv1 = inverse_direction ? *(++uvs1.cbegin()) : *(++uvs1.crbegin());
				const auto pprev_uv2 = inverse_direction ? *(++uvs2.cbegin()) : *(++uvs2.crbegin());
				if ((surf1.evaluate(pprev_uv1.x, pprev_uv1.y) - surf1.evaluate(sol.x, sol.y)).length() < 0.5f * step && (surf2.evaluate(pprev_uv2.x, pprev_uv2.y) - surf2.evaluate(sol.z, sol.w)).length() < 0.5f * step // points are near in world space
					&& (pprev_uv1 - Vector2{ sol.x,sol.y }).length() < (pprev_uv1 - uv1).length() && (pprev_uv2 - Vector2{ sol.z,sol.w }).length() < (pprev_uv2 - uv2).length()) // points are near in parameter space
				{
					// Newton turned back -- passed through singular point
					reverse_tangent_after_singular_point = !reverse_tangent_after_singular_point;
					singular_point_crossed = true;
					continue;
				}
			}

			if (inverse_direction)
			{
				if ((surf1.evaluate(sol.x, sol.y) - surf1.evaluate(uvs1.back().x, uvs1.back().y)).length() < 0.5f * step)
				{
					looped = true;
					break;
				}
				uvs1.push_front({ sol.x, sol.y });
				uvs2.push_front({ sol.z, sol.w });
			}
			else
			{
				if ((surf1.evaluate(sol.x, sol.y) - surf1.evaluate(uvs1.front().x, uvs1.front().y)).length() < 0.5f * step)
				{
					looped = true;
					break;
				}
				uvs1.push_back({ sol.x, sol.y });
				uvs2.push_back({ sol.z, sol.w });
			}

			if (should_change_direction)
			{
				if (inverse_direction)
					break;
				inverse_direction = true;
			}
		}

		return Object::create<IntersectionCurve>(surf1, surf2, uvs1, uvs2, looped, singular_point_crossed, uvs1.size() == max_steps);
	}

	Vector2 IntersectionCurve::find_nearest_point(const ParametricSurface& surf, const Vector3& point)
	{
		return find_nearest_point(surf, point, { surf.get_u_range().middle(), surf.get_v_range().middle() });
	}

	Vector2 IntersectionCurve::find_nearest_point(const ParametricSurface& surf, const Vector3& point, const Vector2& uvstart)
	{
		static constexpr float EPS = 1e-6f;

		// gradienty proste (TODO sprz�one)
		Vector2 result = uvstart;
		auto surf_point = surf.evaluate(result.x, result.y);
		float fval = dot(surf_point - point, surf_point - point);
		Vector2 grad = { 2.0f * dot(surf_point - point, surf.du(result.x, result.y)), 2.0f * dot(surf_point - point, surf.dv(result.x, result.y)) };
		float alpha = 1.0f;
		if (grad.length() < EPS)
			return result;

		const auto start_time = std::clock();

		while (true)
		{
			check_timeout(start_time);

			const auto new_result = result - alpha * grad;
			if (!surf.get_u_range().contains(new_result.x) || !surf.get_v_range().contains(new_result.y))
			{
				alpha /= 2;
				continue;
			}
			//new_result.x = surf.get_u_range().clamp(new_result.x);
			//new_result.y = surf.get_v_range().clamp(new_result.y);
			const auto old_surf_point = surf_point;
			surf_point = surf.evaluate(new_result.x, new_result.y);
			const float new_fval = dot(surf_point - point, surf_point - point);
			const Vector2 new_grad = { 2.0f * dot(surf_point - point, surf.du(new_result.x, new_result.y)), 2.0f * dot(surf_point - point, surf.dv(new_result.x, new_result.y)) };
			//if (grad.length() < EPS)
			//if ((new_result - result).length() < EPS)
			if ((surf_point - old_surf_point).length() < EPS)
			{
				result = new_result;
				break;
			}
			if (new_fval >= fval)
				alpha /= 2;
			else
			{
				fval = new_fval;
				result = new_result;
				grad = new_grad;
			}
		}
		return result;
		//return { surf.get_u_range().clamp(result.x), surf.get_v_range().clamp(result.y) };
	}

	Vector2 IntersectionCurve::find_nearest_point_far_from(const ParametricSurface& surf, const Vector2& uv_far, float step)
	{
		static constexpr int MAX_TRIES = 100;

		static std::random_device dev;
		std::uniform_real_distribution<float> random_u(surf.get_u_range().from, surf.get_u_range().to);
		std::uniform_real_distribution<float> random_v(surf.get_v_range().from, surf.get_v_range().to);

		const auto point_far = surf.evaluate(uv_far.x, uv_far.y);

		for (int i = 0; i < MAX_TRIES; ++i)
		{
			const Vector2 uv = { random_u(dev), random_v(dev) };
			const auto result = find_nearest_point(surf, point_far, uv);

			const auto dist = (result - uv_far).length();
			if (dist > 0.5f * step)
				return result;
		}
		throw CommonIntersectionPointNotFoundException();
	}

	std::pair<Vector2, Vector2> IntersectionCurve::find_first_common_point(const ParametricSurface& surf1, const ParametricSurface& surf2, const Vector2& uv1start, const Vector2& uv2start)
	{
		static constexpr float EPS = 1e-6f;

		// gradienty proste (TODO sprz�one)
		Vector4 result = { uv1start.x, uv1start.y, uv2start.x, uv2start.y };
		auto surf_point1 = surf1.evaluate(result.x, result.y),
			surf_point2 = surf2.evaluate(result.z, result.w);
		float fval = dot(surf_point1 - surf_point2, surf_point1 - surf_point2);
		Vector4 grad = {
			2.0f * dot(surf_point1 - surf_point2, surf1.du(result.x, result.y)),
			2.0f * dot(surf_point1 - surf_point2, surf1.dv(result.x, result.y)),
			-2.0f * dot(surf_point1 - surf_point2, surf2.du(result.z, result.w)),
			-2.0f * dot(surf_point1 - surf_point2, surf2.dv(result.z, result.w)),
		};
		float alpha = 1.0f;
		if (grad.length() < EPS)
			return { {result.x, result.y}, {result.z, result.w} };

		const auto start_time = std::clock();

		while (true)
		{
			check_timeout(start_time);

			const auto new_result = result - alpha * grad;
			const auto old_surf_point1 = surf_point1, old_surf_point2 = surf_point2;
			surf_point1 = surf1.evaluate(new_result.x, new_result.y);
			surf_point2 = surf2.evaluate(new_result.z, new_result.w);
			const float new_fval = dot(surf_point1 - surf_point2, surf_point1 - surf_point2);
			const Vector4 new_grad = {
				2.0f * dot(surf_point1 - surf_point2, surf1.du(result.x, result.y)),
				2.0f * dot(surf_point1 - surf_point2, surf1.dv(result.x, result.y)),
				-2.0f * dot(surf_point1 - surf_point2, surf2.du(result.z, result.w)),
				-2.0f * dot(surf_point1 - surf_point2, surf2.dv(result.z, result.w)),
			};
			//if (grad.length() < EPS)
			auto dif = new_result - result;
			float l = dif.length();
			//if ((new_result - result).length() < EPS)
			if ((old_surf_point1 - surf_point1).length() + (old_surf_point2 - surf_point2).length() < EPS)
			{
				result = new_result;
				break;
			}
			if (new_fval >= fval)
				alpha /= 2;
			else
			{
				fval = new_fval;
				result = new_result;
				grad = new_grad;
			}
		}

		// wrap arguments
		if (surf1.u_wraps_at_v(result.y))
			result.x = surf1.get_u_range().wrap(result.x);
		if (surf1.v_wraps_at_u(result.x))
			result.y = surf1.get_v_range().wrap(result.y);
		if (surf2.u_wraps_at_v(result.w))
			result.z = surf2.get_u_range().wrap(result.z);
		if (surf2.v_wraps_at_u(result.z))
			result.w = surf2.get_v_range().wrap(result.w);

		return { {result.x, result.y}, {result.z, result.w} };
	}

	std::pair<Vector2, Vector2> IntersectionCurve::find_first_common_point(const ParametricSurface& surf1, const ParametricSurface& surf2, const Vector3& hint)
	{
		auto nearest1 = find_nearest_point(surf1, hint);
		auto nearest2 = find_nearest_point(surf2, hint);

		return find_first_common_point(surf1, surf2, nearest1, nearest2);
	}

	std::pair<Vector2, Vector2> IntersectionCurve::find_first_common_point_on_self_intersection(const ParametricSurface& surf, const Vector3& hint, float step)
	{
		const auto uv1 = find_nearest_point(surf, hint);
		const auto uv2 = find_nearest_point_far_from(surf, uv1, step);

		return find_first_common_point(surf, surf, uv1, uv2);
	}

	Object::Handle<IntersectionCurve> IntersectionCurve::intersect_surfaces_with_hint(ParametricSurface& surf1, ParametricSurface& surf2, float step, size_t max_steps, const Vector3& hint)
	{
		auto start = find_first_common_point(surf1, surf2, hint);
		return intersect_surfaces(surf1, surf2, step, max_steps, start.first, start.second);
	}

	Object::Handle<IntersectionCurve> IntersectionCurve::intersect_surfaces_without_hint(ParametricSurface& surf1, ParametricSurface& surf2, float step, size_t max_steps, size_t sample_count_x, size_t sample_count_y)
	{
		Vector2 uv1min, uv2min;
		auto urange1 = surf1.get_u_range(), vrange1 = surf1.get_v_range(),
			urange2 = surf2.get_u_range(), vrange2 = surf2.get_v_range();
		float dmin = INFINITY;
		for (int i1 = 0; i1 < sample_count_x; ++i1)
			for (int j1 = 0; j1 < sample_count_y; ++j1)
				for (int i2 = 0; i2 < sample_count_x; ++i2)
					for (int j2 = 0; j2 < sample_count_y; ++j2)
					{
						const float u1 = urange1.from + i1 * (urange1.to - urange1.from) / (sample_count_x - 1),
							v1 = vrange1.from + j1 * (vrange1.to - vrange1.from) / (sample_count_y - 1),
							u2 = urange2.from + i2 * (urange2.to - urange2.from) / (sample_count_x - 1),
							v2 = vrange2.from + j2 * (vrange2.to - vrange2.from) / (sample_count_y - 1);
						const float d = (surf1.evaluate(u1, v1) - surf2.evaluate(u2, v2)).length();
						if (d < dmin)
						{
							dmin = d;
							uv1min = { u1,v1 };
							uv2min = { u2,v2 };
						}
					}

		auto start = find_first_common_point(surf1, surf2, uv1min, uv2min);
		return intersect_surfaces(surf1, surf2, step, max_steps, start.first, start.second);
	}

	Object::Handle<IntersectionCurve> IntersectionCurve::self_intersect_surface_with_hint(ParametricSurface& surf, float step, size_t max_steps, const Vector3& hint)
	{
		auto start = find_first_common_point_on_self_intersection(surf, hint, step);
		return intersect_surfaces(surf, surf, step, max_steps, start.first, start.second);
	}

	Object::Handle<IntersectionCurve> IntersectionCurve::self_intersect_surface_without_hint(ParametricSurface& surf, float step, size_t max_steps, size_t sample_count_x, size_t sample_count_y)
	{
		Vector2 uv1min, uv2min;
		auto urange = surf.get_u_range(), vrange = surf.get_v_range();
		float dmin = INFINITY;
		for (int i1 = 0; i1 < sample_count_x; ++i1)
			for (int j1 = 0; j1 < sample_count_y; ++j1)
				for (int i2 = 0; i2 < sample_count_x; ++i2)
					for (int j2 = 0; j2 < sample_count_y; ++j2)
					{
						const float ustep = (urange.to - urange.from) / sample_count_x, vstep = (vrange.to - vrange.from) / sample_count_y;
						const float u1 = (i1 + 0.5f) * ustep, v1 = (j1 + 0.5f) * vstep,
							u2 = (i2 + 0.5f) * ustep, v2 = (j2 + 0.5f) * vstep;
						/*const float u1 = urange.from + i1 * (urange.to - urange.from) / (sample_count_x - 1),
							v1 = vrange.from + j1 * (vrange.to - vrange.from) / (sample_count_y - 1),
							u2 = urange.from + i2 * (urange.to - urange.from) / (sample_count_x - 1),
							v2 = vrange.from + j2 * (vrange.to - vrange.from) / (sample_count_y - 1);*/
						if ((u1 - u2) * (u1 - u2) + (v1 - v2) * (v1 - v2) < 0.25 * step * step)
							continue;
						const float d = (surf.evaluate(u1, v1) - surf.evaluate(u2, v2)).length();
						if (d < dmin)
						{
							dmin = d;
							uv1min = { u1,v1 };
							uv2min = { u2,v2 };
						}
					}

		if (!isfinite(dmin))
			throw CommonIntersectionPointNotFoundException();
		auto start = find_first_common_point(surf, surf, uv1min, uv2min);
		if ((start.first - start.second).length() < 0.5f * step)
			throw CommonIntersectionPointNotFoundException();
		return intersect_surfaces(surf, surf, step, max_steps, start.first, start.second);
	}

	std::vector<ObjectHandle> IntersectionCurve::clone() const
	{
		return std::vector<ObjectHandle>(); // IntersectionCurve is a special object (top-level, but independent, but still because it relies on ParametricSurfaces) and thus can't be cloned
	}
}