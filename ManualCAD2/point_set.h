#pragma once

#include <vector>
#include "algebra.h"
#include "drawable.h"

namespace ManualCAD
{
	class PointSet : public Drawable {
		//std::vector<Vector3> points;
		size_t point_count = 0;
	public:
		PointSet(const Matrix4x4& model) : Drawable(model) {}
		PointSet() : Drawable() {}
		/*PointSet(size_t points_count, const Matrix4x4& model) : Renderable(model), points(points_count) {}
		PointSet() : PointSet(0, Matrix4x4::identity()) {}
		PointSet(PointSet&& set) noexcept : Renderable(std::move(set)), points(std::move(set.points)) {}
		PointSet& operator=(PointSet&& set) noexcept {
			points = std::move(set.points);
			model = std::move(set.model);
			return *this;
		}

		inline const float* points_data() const { return reinterpret_cast<const float*>(points.data()); }
		inline size_t points_size_bytes() const { return points.size() * sizeof(Vector3); }
		inline size_t points_count() const { return points.size(); }
		inline Vector3& get_point(int idx) { return points[idx]; }*/
		inline size_t get_point_count() const { return point_count; }

		void set_data(const std::vector<Vector3>& points);
		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;

		void generate_point() { set_data({ {0.0f,0.0f,0.0f} }); };

		//static PointSet point(const Matrix4x4& model);
	};
}