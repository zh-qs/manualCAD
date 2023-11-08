#pragma once

#include "parametric_surface.h"
#include <vector>
#include <list>
#include "constant_parameter.h"

namespace ManualCAD 
{
	class ParametricSurfaceIntersection {
		const ParametricSurface& surf1;
		const ParametricSurface& surf2;
		std::vector<Vector2> uvs1;
		std::vector<Vector2> uvs2;

		bool looped;
		bool singular_crossed;
		bool too_short;

		ParametricSurfaceIntersection(const ParametricSurface& surf1, const ParametricSurface& surf2, const std::list<Vector2>& uvs1, const std::list<Vector2>& uvs2, bool looped = false, bool singular_crossed = false, bool too_short = false)
			: surf1(surf1), surf2(surf2), uvs1(uvs1.begin(), uvs1.end()), uvs2(uvs2.begin(), uvs2.end()), looped(looped), singular_crossed(singular_crossed), too_short(too_short) {}
		ParametricSurfaceIntersection(const ParametricSurface& surf1, const ParametricSurface& surf2, std::vector<Vector2>&& uvs1, std::vector<Vector2>&& uvs2, bool looped = false, bool singular_crossed = false, bool too_short = false)
			: surf1(surf1), surf2(surf2), uvs1(std::move(uvs1)), uvs2(std::move(uvs2)), looped(looped), singular_crossed(singular_crossed), too_short(too_short) {}
	public:
		const std::vector<Vector2>& get_uvs1() const { return uvs1; }
		const std::vector<Vector2>& get_uvs2() const { return uvs2; }
		std::vector<Vector2>&& get_uvs1() { return std::move(uvs1); }
		std::vector<Vector2>&& get_uvs2() { return std::move(uvs2); }
		std::vector<Vector3> get_points() const;
		bool is_looped() const { return looped; }
		bool is_singular_crossed() const { return singular_crossed; }
		bool is_too_short() const { return too_short; }
		const ParametricSurface& get_surf1() const { return surf1; }
		const ParametricSurface& get_surf2() const { return surf2; }

		// checks if points starts the same curve as this
		bool can_be_started_by(const Vector2& uv1, const Vector2& uv2, const float step) const;
		// TODO approach start point with bounding box intersection
		static ParametricSurfaceIntersection intersect_surfaces(const ParametricSurface& surf1, const ParametricSurface& surf2, float step, size_t max_steps, const Vector2& uv1start, const Vector2& uv2start, const bool force_loop);
		static Vector2 find_nearest_point(const ParametricSurface& surf, const Vector3& point);
		static Vector2 find_nearest_point(const ParametricSurface& surf, const Vector3& point, const Vector2& uvstart, const ConstantParameter is_constant = ConstantParameter::None);
		static Vector2 find_nearest_point_far_from(const ParametricSurface& surf, const Vector2& uv_far, float step);
		static std::pair<Vector2, Vector2> find_first_common_point(const ParametricSurface& surf1, const ParametricSurface& surf2, const Vector2& uv1start, const Vector2& uv2start);
		static std::pair<Vector2, Vector2> find_first_common_point(const ParametricSurface& surf1, const ParametricSurface& surf2, const Vector3& hint);
		static std::pair<Vector2, Vector2> find_first_common_point_on_self_intersection(const ParametricSurface& surf, const Vector3& hint, float step);
		static ParametricSurfaceIntersection intersect_surfaces_with_hint(const ParametricSurface& surf1, const ParametricSurface& surf2, float step, size_t max_steps, const Vector3& hint, const bool force_loop = false);
		static ParametricSurfaceIntersection intersect_surfaces_without_hint(const ParametricSurface& surf1, const ParametricSurface& surf2, float step, size_t max_steps, size_t sample_count_x, size_t sample_count_y, const bool force_loop = false);
		static ParametricSurfaceIntersection self_intersect_surface_with_hint(const ParametricSurface& surf, float step, size_t max_steps, const Vector3& hint);
		static ParametricSurfaceIntersection self_intersect_surface_without_hint(const ParametricSurface& surf, float step, size_t max_steps, size_t sample_count_x, size_t sample_count_y);
		static std::list<ParametricSurfaceIntersection> find_many_intersections(const ParametricSurface& surf1, const ParametricSurface& surf2, float step, size_t max_steps, size_t sample_count_x, size_t sample_count_y, const bool force_loop = false);
	};
}