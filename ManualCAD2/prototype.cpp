#include "prototype.h"
#include "object_settings.h"
#include "polygon_envelope.h"
#include "plane_xz.h"
#include "zig_zag_path.h"
#include "surface_path.h"
#include "offset_surface.h"
#include "rough_path.h"
#include "curve_path.h"

namespace ManualCAD
{
	int Prototype::counter = 0;

	void Prototype::generate_renderable()
	{
		update_view();
		view.color = color;
	}

	void Prototype::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_prototype_settings(*this, parent);
	}

	std::vector<Vector3> Prototype::leap(const Vector2& from, const Vector2& to)
	{
		return { {from.x, safe_height(), from.y}, {to.x,safe_height(),to.y} };
	}

	Vector3 Prototype::elevate(const Vector3& point)
	{
		return { point.x,safe_height(),point.z };
	}

	Vector3 Prototype::elevate_to(const Vector3& point, const float h)
	{
		return { point.x,h,point.z };
	}

	Vector3 Prototype::elevate_to_rough(const Vector3& point)
	{
		return { point.x,height_after_rough(),point.z };
	}

	Vector3 Prototype::elevate(const Vector2& point)
	{
		return { point.x,safe_height(),point.y };
	}

	std::vector<Vector3> Prototype::link_flat_paths(const std::vector<std::vector<Vector2>>& paths)
	{
		const Vector2 min = { view_boundary_points[0].x, view_boundary_points[0].z },
			max = { view_boundary_points[2].x, view_boundary_points[2].z };
		const float height = view_boundary_points[0].y;

		size_t point_count = 3; // start path
		for (const auto& p : paths)
			point_count += p.size() + 2; // path plus leap

		std::vector<Vector3> result;
		result.reserve(point_count);

		Vector2 to_leap;
		const auto& start_point = paths.front().front();
		if (start_point.x <= center.x)
		{
			to_leap = { min.x - 2.0f * scale, 0.5f * (min.y + max.y) };
		}
		else if (start_point.x >= center.x)
		{
			to_leap = { max.x + 2.0f * scale, 0.5f * (min.y + max.y) };
		}
		/*else if (start_point.y <= min.y)
		{
			to_leap = { 0.5f * (min.x + max.x), min.y - scale };
		}
		else if (start_point.y >= max.y)
		{
			to_leap = { 0.5f * (min.x + max.x), max.y + scale };
		}*/

		auto l = leap(0.5f * (min + max), to_leap);
		result.insert(result.end(), l.begin(), l.end());
		result.push_back({ to_leap.x, height, to_leap.y });
		for (int i = 0; i < paths.size() - 1; ++i)
		{
			for (const auto& v : paths[i])
			{
				result.push_back({ v.x, height, v.y });
			}
			l = leap(paths[i].back(), paths[i + 1].front());
			result.insert(result.end(), l.begin(), l.end());
		}
		for (const auto& v : paths.back())
			result.push_back({ v.x, height, v.y });
		l = leap(paths.back().back(), 0.5f * (min + max));
		result.insert(result.end(), l.begin(), l.end());
		to_workpiece_coords(result);

		if constexpr (ApplicationSettings::DEBUG)
			assert(result.size() == point_count);

		return result;
	}

	std::vector<Vector3> Prototype::link_paths(const std::vector<std::vector<Vector3>>& paths)
	{
		const Vector2 min = { view_boundary_points[0].x, view_boundary_points[0].z },
			max = { view_boundary_points[2].x, view_boundary_points[2].z };
		float height = view_boundary_points[0].y + scale * signature_depth;

		std::vector<Vector3> result;

		size_t point_count = 2;
		for (const auto& path : paths)
			point_count += path.size() + 2;
		result.reserve(point_count);

		result.push_back(elevate(0.5f * (min + max)));
		
		for (const auto& path : paths)
		{
			result.push_back(elevate_to(path.front(), height));
			result.insert(result.end(), path.begin(), path.end());
			result.push_back(elevate_to(path.back(), height));
		}

		result.push_back(elevate(0.5f * (min + max)));

		// correct y coordinate of second and before-last element (elevate to safe height)
		result[1].y = safe_height();
		result[result.size() - 2].y = safe_height();

		to_workpiece_coords(result);

		if constexpr (ApplicationSettings::DEBUG)
			assert(result.size() == point_count);

		return result;
	}

	std::vector<Vector3> Prototype::link_surface_ball_cutter_paths(const std::vector<std::vector<std::vector<Vector2>>>& paths, const float radius, const PlaneXZ& plane)
	{
		const Vector2 min = { view_boundary_points[0].x, view_boundary_points[0].z },
			max = { view_boundary_points[2].x, view_boundary_points[2].z };
		const float height = view_boundary_points[0].y;

		std::vector<std::list<Vector3>> surface_results(surfaces.size() + 1); // plus 1 because we also make paths for plane

		// paths for surfaces
		int i = 0;
		for (const auto* surf : surfaces)
		{
			auto offset_surf = OffsetSurface{ *surf, radius };
			for (const auto& path : paths[i])
			{
				std::list<Vector3> path3d;
				constexpr float step = 0.05f;
				for (int j = 0; j < path.size() - 1; ++j)
				{
					const float d = (path[j + 1] - path[j]).length();
					for (float t = 0.0f; t <= d; t += step)
					{
						auto uv = lerp(path[j], path[j + 1], t);
						path3d.push_back(offset_surf.evaluate(uv.x, uv.y) - Vector3{0.0f, radius, 0.0f}); // VERY important: subtract radius from position (because of ball cutter: curves mark movement of its center)
					}	
				}
				//for (int j = 0; j < path.size(); ++j) // improper: only for zig-zag testing
				//	path3d.push_back(offset_surf.evaluate(path[j].x, path[j].y));
				path3d.push_front(elevate_to_rough(path3d.front()));
				path3d.push_back(elevate_to_rough(path3d.back()));
				surface_results[i].insert(surface_results[i].begin(), path3d.begin(), path3d.end());
			}
			++i;
		}

		// paths for plane
		for (const auto& path : paths.back())
		{
			std::list<Vector3> path3d;
			for (int j = 0; j < path.size(); ++j)
				path3d.push_back(plane.evaluate(path[j].x, path[j].y) - Vector3{ 0.0f, radius, 0.0f });
			path3d.push_front(elevate_to_rough(path3d.front()));
			path3d.push_back(elevate_to_rough(path3d.back()));
			surface_results[i].insert(surface_results[i].begin(), path3d.begin(), path3d.end());
		}

		size_t point_count = 2;
		for (const auto& r : surface_results)
			point_count += r.size();

		std::vector<Vector3> result;
		result.reserve(point_count);

		result.push_back(elevate(0.5f * (min + max)));

		for (auto& r : surface_results)
			result.insert(result.end(), r.begin(), r.end());

		result.push_back(elevate(0.5f * (min + max)));

		to_workpiece_coords(result);

		if constexpr (ApplicationSettings::DEBUG)
			assert(result.size() == point_count);

		return result;
	}

	std::vector<Vector3> Prototype::link_single_flat_loop(const std::vector<Vector2>& loop)
	{
		const Vector2 min = { view_boundary_points[0].x, view_boundary_points[0].z },
			max = { view_boundary_points[2].x, view_boundary_points[2].z };
		const float height = view_boundary_points[0].y;

		const size_t point_count = loop.size() + 6;  // 3 for start, 1 for closed loop and 2 for end
		std::vector<Vector3> result;
		result.reserve(point_count);

		int start_i = 0;
		for (int i = 1; i < loop.size(); ++i)
		{
			if (loop[i].x < loop[start_i].x)
				start_i = i;
		}

		Vector2 to_leap{ min.x - scale, loop[start_i].y };
		auto l = leap(0.5f * (min + max), to_leap);

		result.insert(result.end(), l.begin(), l.end());
		result.push_back({ to_leap.x, height, to_leap.y });

		for (int i = start_i; i < loop.size(); ++i)
			result.push_back({ loop[i].x, height, loop[i].y });
		for (int i = 0; i <= start_i; ++i)
			result.push_back({ loop[i].x, height, loop[i].y });
		l = leap(loop[start_i], 0.5f * (min + max));
		result.insert(result.end(), l.begin(), l.end());
		to_workpiece_coords(result);

		if constexpr (ApplicationSettings::DEBUG)
			assert(result.size() == point_count);

		return result;
	}

	std::vector<Vector3> Prototype::compact_path(const std::vector<Vector3>& path)
	{
		constexpr float EPS = 1e-6;

		if (path.size() < 2)
			return path;

		// merge consecutive parallel fragments
		std::vector<Vector3> result;
		result.reserve(path.size());
		auto start = path[0], end = path[1];
		result.push_back(start);
		for (int i = 2; i < path.size(); ++i)
		{
			if (cross(end - start, path[i] - end).length() < EPS)
			{
				end = path[i];
			}
			else
			{
				result.push_back(end);
				start = end;
				end = path[i];
			}
		}
		result.push_back(end);
		return result; // TODO lines at the beginning and the end may also be parallel
	}

	void Prototype::show_envelope_experimental()
	{
		const Vector2 min = { view_boundary_points[0].x, view_boundary_points[0].z },
			max = { view_boundary_points[2].x, view_boundary_points[2].z };
		const float height = view_boundary_points[0].y;
		PlaneXZ plane{ min, max, height };
		//PolygonEnvelope::Builder envelope_builder;

		//for (const auto* surf : surfaces)
		//{
		//	auto intersection = ParametricSurfaceIntersection::intersect_surfaces_without_hint(*surf, plane, 0.01f, 2500, 20, 20, true);
		//	//envelope_builder.add_polygon(intersection.get_uvs2());
		//}
		//auto envelope = envelope_builder.build();
		//envelope.expand(0.2f);

		//ZigZagPath zigzag;
		//zigzag.add_loop(envelope.get_points());
		//auto paths = zigzag.generate_paths(min, max, 0.2f, 0.02f);

		//std::vector<Vector3> points = link_flat_paths(paths);
		//std::vector<Vector3> points = link_single_flat_loop(envelope.get_points());
		//points = compact_path(points);
		//auto intersections = ParametricSurfaceIntersection::find_many_intersections(*surfaces.front(), plane, 0.01f, 2500, 20, 20, false);
		//std::vector<Vector3> points = link_single_flat_loop(intersections.back().get_uvs2());
		auto lines = SurfacePath{ surfaces }.generate_paths(plane, scale * 0.4f, scale * 0.724f, scale * mill_height, size);
		std::vector<Vector3> points;

		int i = 0;
		for (auto* surf : surfaces)
		{
			auto s = OffsetSurface{ *surf, scale * 0.4f };
			for (const auto& l : lines[i])
			{
				const float step = 0.05f;
				for (int j = 0; j < l.size() - 1; ++j)
				{
					const float d = (l[j + 1] - l[j]).length();
					for (float t = 0.0f; t <= d; t += step)
					{
						auto uv = lerp(l[j], l[j + 1], t);
						points.push_back(s.evaluate(uv.x, uv.y));
					}
				}
			}
			++i;
		}
		view.set_data(points);
	}

	void Prototype::generate_rough_program(const Cutter& cutter)
	{
		const Vector2 min = { view_boundary_points[0].x, view_boundary_points[0].z },
			max = { view_boundary_points[2].x, view_boundary_points[2].z };
		RoughPath path{ surfaces, center, min, max, size.y - mill_height, size.y, scale };
		auto points = path.generate_path(2, size, cutter.get_radius(), cutter.get_radius() * rough_epsilon_factor, rough_height_offset);

		compact_path(points);

		generated_program = MillingProgram{ "Rough" };
		generated_program.value().add_move({ 3,false,{0.0f, safe_height_unscaled(), 0.0f}, {points[0].x, safe_height_unscaled(), points[0].z} });
		generated_program.value().add_move({ 4,false,{points[0].x, safe_height_unscaled(), points[0].z}, points[0] });
		int i = 0;
		for (; i < points.size() - 1; ++i)
			generated_program.value().add_move({ i + 5,false,points[i],points[i + 1] });

		generated_program.value().add_move({ i + 5,false, points.back(), {points.back().x, safe_height_unscaled(), points.back().z} });
		generated_program.value().add_move({ i + 6,false,{points.back().x, safe_height_unscaled(), points.back().z}, {0.0f, safe_height_unscaled(), 0.0f} });
	}

	void Prototype::generate_flat_plane_program(const Cutter& cutter)
	{
		const Vector2 min = { view_boundary_points[0].x, view_boundary_points[0].z },
			max = { view_boundary_points[2].x, view_boundary_points[2].z },
			cutter_offset = { scale * cutter.get_radius(), scale * cutter.get_radius() };
		const float height = view_boundary_points[0].y;
		PlaneXZ plane{ min - cutter_offset, max + cutter_offset, height };
		PolygonEnvelope::Builder envelope_builder;

		const float radius = scale * cutter.get_radius();

		for (const auto* surf : surfaces)
		{
			OffsetSurface offset_surf{ *surf, radius };
			try
			{
				auto intersection = ParametricSurfaceIntersection::intersect_surfaces_without_hint(offset_surf, plane, 0.01f, 2500, 20, 20, true);
				envelope_builder.add_polygon(intersection.get_uvs2());
			}
			catch (const CommonIntersectionPointNotFoundException&)
			{
				// nothing
			}
		}
		auto envelope = envelope_builder.build();
		//envelope.expand(scale * cutter.get_radius() * (1.0f + epsilon_factor)); // TODO coœ lepszego ¿eby nie podcina³o

		ZigZagPath zigzag;
		zigzag.add_line(envelope.get_points(), true);
		auto paths = zigzag.generate_paths_outside_loops(min - cutter_offset, max + cutter_offset, scale * cutter.get_radius(), scale * cutter.get_radius() * flat_epsilon_factor);

		std::vector<Vector3> points = link_flat_paths(paths);
		points = compact_path(points);

		generated_program = MillingProgram{ "Flat plane" };
		for (int i = 0; i < points.size() - 1; ++i)
			generated_program.value().add_move({ i + 3,false,points[i],points[i + 1] });
	}

	void Prototype::generate_envelope_program(const Cutter& cutter)
	{
		const Vector2 min = { view_boundary_points[0].x, view_boundary_points[0].z },
			max = { view_boundary_points[2].x, view_boundary_points[2].z },
			cutter_offset = { scale * cutter.get_radius(), scale * cutter.get_radius() };
		const float height = view_boundary_points[0].y;
		PlaneXZ plane{ min - cutter_offset, max + cutter_offset, height };
		PolygonEnvelope::Builder envelope_builder;

		const float radius = scale * cutter.get_radius();

		for (const auto* surf : surfaces)
		{
			OffsetSurface offset_surf{ *surf, radius };
			try
			{
				auto intersection = ParametricSurfaceIntersection::intersect_surfaces_without_hint(offset_surf, plane, 0.01f, 2500, 20, 20, true);
				envelope_builder.add_polygon(intersection.get_uvs2());
			}
			catch (const CommonIntersectionPointNotFoundException&)
			{
				// nothing
			}
		}
		auto envelope = envelope_builder.build();
		//envelope.expand(scale * cutter.get_radius()); we do not expane if we use offset surfaces

		std::vector<Vector3> points = link_single_flat_loop(envelope.get_points());
		points = compact_path(points);

		generated_program = MillingProgram{ "Envelope" };
		for (int i = 0; i < points.size() - 1; ++i)
			generated_program.value().add_move({ i + 3,false,points[i],points[i + 1] });
	}

	void Prototype::generate_detailed_program(const Cutter& cutter)
	{
		const Vector2 min = { view_boundary_points[0].x, view_boundary_points[0].z },
			max = { view_boundary_points[2].x, view_boundary_points[2].z };
		const float height = view_boundary_points[0].y;
		const float radius = scale * cutter.get_radius();
		PlaneXZ plane{ min, max, height + radius }; // we offset plane by radius (because cutter must not cut the flat plane)

		auto lines = SurfacePath{ surfaces }.generate_paths(plane, radius, radius * detailed_epsilon_factor, scale * mill_height, size);
		auto points = link_surface_ball_cutter_paths(lines, radius, plane);

		points = compact_path(points);
		generated_program = MillingProgram{ "Detailed" };
		for (int i = 0; i < points.size() - 1; ++i)
			generated_program.value().add_move({ i + 3,false,points[i],points[i + 1] });
	}

	void Prototype::generate_signature_program(const Cutter& cutter)
	{
		if (signature_curves.size() == 0)
			return;

		CurvePath path;

		for (const auto* curve : signature_curves)
			path.add_curve_bezier_points(curve->get_bezier_points());

		const float base_height = view_boundary_points[0].y - scale * signature_depth;

		path.project_curves(base_height);

		auto lines = path.generate_paths();
		auto points = link_paths(lines);

		generated_program = MillingProgram{ "Signature" };
		for (int i = 0; i < points.size() - 1; ++i)
			generated_program.value().add_move({ i + 3,false,points[i],points[i + 1] });
	}

	void Prototype::update_view()
	{
		if (surfaces.empty())
		{
			view.set_data({});
			return;
		}

		if (!box_valid)
		{
			bounding_box = Box::degenerate();
			for (const auto* s : surfaces)
			{
				auto surf_box = s->get_bounding_box();
				bounding_box.merge_with(surf_box);
			}
			box_valid = true;
		}
		Box box = bounding_box;
		box.offset_by({ offset, 0.0f, offset });

		center = box.center();
		// TODO wpasowaæ w size
		float size_scale = size.x / size.z;
		float box_scale = (box.x_max - box.x_min) / (box.z_max - box.z_min);
		if (box_scale > size_scale)
			scale = (box.x_max - box.x_min) / size.x;
		else
			scale = (box.z_max - box.z_min) / size.z;

		center.y = box.y_max - size.y * scale; // center of a workpiece is at its bottom
		float h = box.y_max - mill_height * scale;
		float xm = center.x - 0.5f * size.x * scale, xM = center.x + 0.5f * size.x * scale,
			zm = center.z - 0.5f * size.z * scale, zM = center.z + 0.5f * size.z * scale;
		view_boundary_points = {
			{xm,h,zm}, {xm, h,zM}, {xM,h,zM}, {xM,h,zm}
		};
		view.set_data(view_boundary_points);
	}

	void Prototype::generate_program(ProgramType type, std::unique_ptr<Cutter>&& cutter)
	{
		switch (type)
		{
		case ProgramType::Rough:
			generate_rough_program(*cutter);
			break;
		case ProgramType::FlatPlane:
			generate_flat_plane_program(*cutter);
			break;
		case ProgramType::Envelope:
			generate_envelope_program(*cutter);
			break;
		case ProgramType::Detailed:
			generate_detailed_program(*cutter);
			break;
		case ProgramType::Signature:
			generate_signature_program(*cutter);
			break;
		default:
			throw std::runtime_error("Invalid cutter type");
		}

		generated_program->set_cutter(std::move(cutter));
		view.set_data(generated_program.value().get_cutter_positions());
	}

	std::vector<ObjectHandle> Prototype::clone() const
	{
		return std::vector<ObjectHandle>(); // Prototype is a special object and thus can't be cloned
	}
}