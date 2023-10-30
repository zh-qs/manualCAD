#pragma once
#include "renderable.h"
#include "algebra.h"
#include <vector>

namespace ManualCAD
{
	class Point;

	class CurveWithPolyline : public Renderable {
		//std::vector<const Point*> point_ptrs;
		size_t actual_point_count = 0;
		size_t point_count = 0;
		size_t group_count = 0;
		ElementBuffer ebo;

		void init_additional_buffers() {
			ebo.init(); 
			ebo.bind();
			vao.unbind();
		}
	public:
		Vector4 polyline_color = { 0.0f,0.0f,0.0f,0.0f };
		bool draw_polyline = false;
		bool draw_curve = true;

		CurveWithPolyline(const Matrix4x4& model) : Renderable(model), ebo() { init_additional_buffers(); }
		CurveWithPolyline() : Renderable(), ebo() { init_additional_buffers(); }
		/*CurveWithPolyline(const std::vector<const Point*>& point_ptrs, const Matrix4x4& model, bool draw_polyline, bool draw_curve) : Renderable(model), point_ptrs(point_ptrs), draw_polyline(draw_polyline), draw_curve(draw_curve) {}
		CurveWithPolyline() : CurveWithPolyline({}, Matrix4x4::identity(), false, true) {}
		CurveWithPolyline(CurveWithPolyline&& cwp) noexcept : Renderable(std::move(cwp.model)), point_ptrs(std::move(cwp.point_ptrs)), draw_polyline(std::move(cwp.draw_polyline)), draw_curve(std::move(cwp.draw_curve)) {}
		CurveWithPolyline& operator=(CurveWithPolyline&& cwp) noexcept {
			model = std::move(cwp.model);
			point_ptrs = std::move(cwp.point_ptrs);
			draw_polyline = std::move(cwp.draw_polyline);
			draw_curve = std::move(cwp.draw_curve);
			return *this;
		}

		std::vector<Vector3> points() const;
		inline size_t points_count() const { return point_ptrs.size(); }
		inline size_t points_size_bytes() const { return point_ptrs.size() * sizeof(Vector3); }*/
		void set_data(const std::vector<Vector3>& points);
		inline size_t get_point_count() const { return actual_point_count; }
		inline size_t get_line_indices_count() const { return group_count * 4; }
		void dispose() override { Renderable::dispose(); ebo.dispose(); }

		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;
		void generate_curve(const std::vector<const Point*>& points, bool draw_polyline, bool draw_curve);
		void generate_curve(const std::vector<Vector3>& points, bool draw_polyline, bool draw_curve);
	};
}