#include "serializer.h"

#include "algebra.h"
#include <map>

MG1::Float3 from_vector3(const ManualCAD::Vector3& vec)
{
	return { vec.x,vec.y,vec.z };
}

MG1::Float2 from_vector3_2(const ManualCAD::Vector3& vec)
{
	return { vec.x,vec.y };
}

ManualCAD::Vector3 from_float3(const MG1::Float3& f)
{
	return { f.x,f.y,f.z };
}

MG1::PointRef ManualCAD::Serializer::get_point_ref_to(const Point& point)
{
	int idx = controller.get_index(point);
	if (idx == -1)
		throw std::invalid_argument("Point does not exist on an object list");
	return { static_cast<uint32_t>(idx) };
}

ManualCAD::Serializer::Serializer(ObjectController& controller, Camera& camera) : controller(controller), camera(camera), serializer()
{
	object_count = controller.objects.size();
}

void ManualCAD::Serializer::serialize(const std::filesystem::path& filepath)
{
	MG1::Scene& scene = MG1::Scene::Get();
	scene.Clear();

	// TODO serialize color
	controller.add_objects_to_serializer(*this);
	scene.camera.focusPoint = from_vector3(camera.target);
	auto& distance = *camera.get_screen_to_target_distance_handle();
	scene.camera.distance = distance / camera.scale;
	scene.camera.rotation = from_vector3_2(camera.get_rotation_deg());
	scene.isCameraSet = true;

	if (!MG1::Scene::Get().IsValid())
		throw std::logic_error("Invalid scene");

	serializer.SaveScene(filepath);
}

void ManualCAD::Serializer::add_point(const Point& point, int idx)
{
	MG1::Point p;
	p.name = point.name;
	p.position = from_vector3(point.transformation.position);
	p.SetId(idx);
	MG1::Scene::Get().points.push_back(p);
}

void ManualCAD::Serializer::add_torus(const Torus& torus, int idx)
{
	MG1::Torus t;
	t.name = torus.name;
	t.position = from_vector3(torus.transformation.position);
	//t.rotation = from_vector3(torus.transformation.rotation_vector); // TODO na pewno?
	t.rotation = from_vector3(torus.transformation.get_euler_rotation_deg());
	t.scale = from_vector3(torus.transformation.scale);
	t.largeRadius = torus.large_radius;
	t.smallRadius = torus.small_radius;
	t.samples = { torus.divisions_x, torus.divisions_y };
	t.SetId(idx);
	MG1::Scene::Get().tori.push_back(t);
}

void ManualCAD::Serializer::add_bezier_c0_curve(const BezierC0Curve& curve, int idx)
{
	MG1::BezierC0 c;
	c.name = curve.name;
	for (auto* p : curve.points)
		c.controlPoints.push_back(get_point_ref_to(*p));
	c.SetId(idx);
	MG1::Scene::Get().bezierC0.push_back(c);
}

void ManualCAD::Serializer::add_bezier_c2_curve(const BezierC2Curve& curve, int idx)
{
	MG1::BezierC2 c;
	c.name = curve.name;
	for (auto* p : curve.points)
		c.controlPoints.push_back(get_point_ref_to(*p));
	c.SetId(idx);
	MG1::Scene::Get().bezierC2.push_back(c);
}

void ManualCAD::Serializer::add_interpolation_spline(const InterpolationSpline& spline, int idx)
{
	MG1::InterpolatedC2 c;
	c.name = spline.name;
	for (auto* p : spline.points)
		c.controlPoints.push_back(get_point_ref_to(*p));
	c.SetId(idx);
	MG1::Scene::Get().interpolatedC2.push_back(c);
}

void ManualCAD::Serializer::add_bicubic_bezier_c0_surface(const BicubicC0BezierSurface& surf, int idx)
{
	MG1::BezierSurfaceC0 s;
	s.name = surf.name;
	s.vWrapped = false;
	s.uWrapped = surf.cylinder;
	s.size.x = surf.patches_x;
	s.size.y = surf.patches_y;
	s.patches.reserve(s.size.x * s.size.y);

	const int points_y = 3 * surf.patches_y + 1;
	const int points_x = 3 * surf.patches_x + 1;

	for (int j = 0; j < surf.patches_y; ++j)
	{
		for (int i = 0; i < surf.patches_x; ++i)
		{
			MG1::BezierPatchC0 p;
			p.name = surf.name + ", patch[" + std::to_string(i) + "," + std::to_string(j) + "]";
			p.samples = { surf.surf.divisions_x, surf.surf.divisions_y };
			p.controlPoints.reserve(16);
			p.SetId(object_count++);
			for (int y = 0; y < 4; ++y)
				for (int x = 0; x < 4; ++x)
				{
					int array_idx = (3 * i + x) + (3 * j + y) * points_x;
					MG1::PointRef pref = get_point_ref_to(*surf.points[array_idx]);
					p.controlPoints.push_back(pref);
				}

			s.patches.push_back(p);
		}
	}

	s.SetId(idx);
	MG1::Scene::Get().surfacesC0.push_back(s);
}

void ManualCAD::Serializer::add_bicubic_bezier_c2_surface(const BicubicC2BezierSurface& surf, int idx)
{
	MG1::BezierSurfaceC2 s;
	s.name = surf.name;
	s.vWrapped = false;
	s.uWrapped = surf.cylinder;
	s.size.x = surf.patches_x;
	s.size.y = surf.patches_y;
	s.patches.reserve(s.size.x * s.size.y);

	const int points_y = surf.patches_y + 3;
	const int points_x = surf.patches_x + 3;

	for (int j = 0; j < surf.patches_y; ++j)
	{
		for (int i = 0; i < surf.patches_x; ++i)
		{
			MG1::BezierPatchC2 p;
			p.name = surf.name + ", patch[" + std::to_string(i) + "," + std::to_string(j) + "]";
			p.samples = { surf.surf.divisions_x, surf.surf.divisions_y };
			p.controlPoints.reserve(16);
			p.SetId(object_count++); // TODO
			for (int y = 0; y < 4; ++y)
				for (int x = 0; x < 4; ++x)
				{
					int array_idx = (i + x) + (j + y) * points_x;
					MG1::PointRef pref = get_point_ref_to(*surf.points[array_idx]);
					p.controlPoints.push_back(pref);
				}

			s.patches.push_back(p);
		}
	}

	s.SetId(idx);
	MG1::Scene::Get().surfacesC2.push_back(s);
}

void ManualCAD::Serializer::deserialize(const std::filesystem::path& filepath)
{
	MG1::Scene& scene = MG1::Scene::Get();

	scene.Clear();

	serializer.LoadScene(filepath);

	if (!scene.IsValid())
		throw std::logic_error("Invalid scene");

	if (scene.isCameraSet)
	{
		camera.target = from_float3(scene.camera.focusPoint);
		auto& distance = *camera.get_screen_to_target_distance_handle();
		if constexpr (ApplicationSettings::ZOOM_POLICY == ApplicationSettings::CameraZoomPolicy::MultiplyScale)
			camera.scale = distance / scene.camera.distance;
		else
			distance = scene.camera.distance;
		camera.rotx = PI * scene.camera.rotation.x / 180.0f;
		camera.roty = PI * scene.camera.rotation.y / 180.0f;
	}

	auto p_it = scene.points.begin();
	auto t_it = scene.tori.begin();
	auto b0_it = scene.bezierC0.begin();
	auto b2_it = scene.bezierC2.begin();
	auto i_it = scene.interpolatedC2.begin();
	auto s0_it = scene.surfacesC0.begin();
	auto s2_it = scene.surfacesC2.begin();

	size_t size = scene.points.size() + scene.tori.size() + scene.bezierC0.size() + scene.bezierC2.size() + scene.interpolatedC2.size() + scene.surfacesC0.size() + scene.surfacesC2.size();

	uint32_t idx = 0;
	size_t inserted = 0;
	std::map<uint32_t, std::pair<Object::Handle<Point>, Point*>> point_map;

	for (const auto& p : scene.points)
	{
		auto point = Object::create<Point>();
		point->transformation.position = from_float3(p.position);
		point->update_renderable_matrix();
		point->name = p.name;
		auto* pptr = point.get();
		point_map.insert({ p.GetId(), std::pair{ std::move(point), pptr } });
	}

	while (inserted < size)
	{
		if (p_it != scene.points.end() && p_it->GetId() == idx) {
			controller.add_object(std::move(point_map[idx].first));
			++inserted;
			++p_it;
		}
		if (t_it != scene.tori.end() && t_it->GetId() == idx) {
			auto torus = Object::create<Torus>();
			torus->transformation.position = from_float3(t_it->position);
			Vector3 rotation = from_float3(t_it->rotation);
			torus->name = t_it->name;
			//torus->transformation.rotation_vector = rotation;
			//torus->transformation.rotation_angle_deg = rotation.length();
			torus->transformation.set_rotation_from_euler_deg(rotation);
			torus->transformation.scale = from_float3(t_it->scale);
			torus->divisions_x = t_it->samples.x;
			torus->divisions_y = t_it->samples.y;
			torus->large_radius = t_it->largeRadius;
			torus->small_radius = t_it->smallRadius;
			controller.add_object(std::move(torus));
			++inserted;
			++t_it;
		}
		if (b0_it != scene.bezierC0.end() && b0_it->GetId() == idx) {
			std::list<Point*> points;
			for (const auto& pref : b0_it->controlPoints)
			{
				points.push_back(point_map[pref.GetId()].second);
			}
			auto curve = Object::create<BezierC0Curve>(points);
			curve->name = b0_it->name;
			controller.add_object(std::move(curve));
			++inserted;
			++b0_it;
		}
		if (b2_it != scene.bezierC2.end() && b2_it->GetId() == idx) {
			std::vector<Point*> points;
			for (const auto& pref : b2_it->controlPoints)
			{
				points.push_back(point_map[pref.GetId()].second);
			}
			auto curve = Object::create<BezierC2Curve>(points);
			auto* cptr = curve.get();
			curve->name = b2_it->name;
			controller.add_object(std::move(curve));
			controller.add_object(cptr->make_linked_bernstein_points());
			++inserted;
			++b2_it;
		}
		if (i_it != scene.interpolatedC2.end() && i_it->GetId() == idx) {
			std::vector<Point*> points;
			for (const auto& pref : i_it->controlPoints)
			{
				points.push_back(point_map[pref.GetId()].second);
			}
			auto curve = Object::create<InterpolationSpline>(points);
			curve->name = i_it->name;
			controller.add_object(std::move(curve));
			++inserted;
			++i_it;
		}
		if (s0_it != scene.surfacesC0.end() && s0_it->GetId() == idx) {
			int patches_x = s0_it->size.x;
			int patches_y = s0_it->size.y;
			bool cylinder = s0_it->uWrapped || s0_it->vWrapped;

			if (s0_it->uWrapped && s0_it->vWrapped)
				throw std::invalid_argument("Surface can't be wrapped in both directions");

			int points_x = 3 * patches_x + 1;
			int points_y = 3 * patches_y + 1;
			std::vector<Point*> points(points_x * points_y);

			for (int i = 0; i < patches_x; ++i)
			{
				for (int j = 0; j < patches_y; ++j)
				{
					for (int x = 0; x < 4; ++x)
						for (int y = 0; y < 4; ++y)
						{
							auto* ptr = point_map[s0_it->patches[i + j * patches_x].controlPoints[x + y * 4].GetId()].second;
							points[(3 * i + x) + (3 * j + y) * points_x] = ptr;
						}
				}
			}
			/*for (auto* p : points)
				p->increment_persistence();*/ // done in constructor

			auto surf = Object::create<BicubicC0BezierSurface>(points, patches_x, patches_y, cylinder);

			if (cylinder)
				surf->real_children = surf->children = (points_x - 1) * points_y;
			else
				surf->real_children = surf->children = points_x * points_y;

			surf->surf.divisions_x = s0_it->patches[0].samples.x;
			surf->surf.divisions_y = s0_it->patches[0].samples.y;

			surf->name = s0_it->name;
			controller.add_object(std::move(surf));
			++inserted;
			++s0_it;
		}
		if (s2_it != scene.surfacesC2.end() && s2_it->GetId() == idx) {
			int patches_x = s2_it->size.x;
			int patches_y = s2_it->size.y;
			bool cylinder = s2_it->uWrapped || s2_it->vWrapped; // TODO do NOT ignore vWrapped!!!

			if (s2_it->uWrapped && s2_it->vWrapped)
				throw std::invalid_argument("Surface can't be wrapped in both directions");

			int points_x = patches_x + 3;
			int points_y = patches_y + 3;
			std::vector<Point*> points(points_x * points_y);

			for (int i = 0; i < patches_x; ++i)
			{
				for (int j = 0; j < patches_y; ++j)
				{
					for (int x = 0; x < 4; ++x)
						for (int y = 0; y < 4; ++y)
						{
							auto id = s2_it->patches[i + j * patches_x].controlPoints[x + y * 4].GetId();
							auto* ptr = point_map[id].second;
							points[(i + x) + (j + y) * points_x] = ptr;
						}
				}
			}
			/*for (auto* p : points)
				p->increment_persistence();*/ // done in constructor

			auto surf = Object::create<BicubicC2BezierSurface>(points, patches_x, patches_y, cylinder);
			
			if (cylinder)
				surf->real_children = surf->children = (points_x - 3) * points_y;
			else
				surf->real_children = surf->children = points_x * points_y;

			surf->surf.divisions_x = s2_it->patches[0].samples.x;
			surf->surf.divisions_y = s2_it->patches[0].samples.y;

			surf->name = s2_it->name;
			controller.add_object(std::move(surf));
			++inserted;
			++s2_it;
		}
		++idx;
	}
}
