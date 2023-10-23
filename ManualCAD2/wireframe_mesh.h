#pragma once

#include <vector>
#include "algebra.h"
#include "drawable.h"

namespace ManualCAD
{
	class WireframeMesh : public Drawable {
		//std::vector<Vector3> points;
		//std::vector<IndexPair> line_indices;
		ElementBuffer ebo;

		size_t point_count = 0, line_count = 0;

		static const Matrix4x4 IDENTITY;
	public:
		WireframeMesh(const Matrix4x4& model) : Drawable(model), ebo() { ebo.init(); ebo.bind(); }
		WireframeMesh() : Drawable(), ebo() { ebo.init(); ebo.bind(); }
		/*WireframeMesh(size_t points_count, size_t lines_count, const Matrix4x4& model) : Drawable(model), points(points_count), line_indices(lines_count) {}
		WireframeMesh() : WireframeMesh(0, 0, Matrix4x4::identity()) {}
		WireframeMesh(WireframeMesh&& mesh) noexcept : Drawable(std::move(mesh)), points(std::move(mesh.points)), line_indices(std::move(mesh.line_indices)) {}
		WireframeMesh& operator=(WireframeMesh&& mesh) noexcept {
			points = std::move(mesh.points);
			line_indices = std::move(mesh.line_indices);
			model = std::move(mesh.model);
			return *this;
		}*/

		/*inline const float* points_data() const { return reinterpret_cast<const float*>(points.data()); }
		inline const unsigned int* line_indices_data() const { return reinterpret_cast<const unsigned int*>(line_indices.data()); }
		inline size_t points_size_bytes() const { return points.size() * sizeof(Vector3); }
		inline size_t points_count() const { return points.size(); }
		inline size_t line_indices_size_bytes() const { return line_indices.size() * sizeof(IndexPair); }
		inline size_t line_count() const { return line_indices.size(); }
		inline Vector3& get_point(int idx) { return points[idx]; }
		inline IndexPair& get_line(int idx) { return line_indices[idx]; }*/
		inline size_t get_point_count() const { return point_count; }
		inline size_t get_line_count() const { return line_count; }

		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;
		void dispose() override { Drawable::dispose(); ebo.dispose(); }

		void set_data(const std::vector<Vector3>& points, const std::vector<IndexPair>& line_indices);
		void generate_torus(float large_radius, float small_radius, unsigned int divisions_x, unsigned int divisions_y);
		void generate_axis_from_origin(const Vector3& direction);
		void generate_grid(unsigned int half_x_count, unsigned int half_z_count, float x_length, float z_length);

		/*static WireframeMesh torus(float large_radius, float small_radius, unsigned int divisions_x, unsigned int divisions_y, const Matrix4x4& model);
		static WireframeMesh axis_from_origin(const Vector3& direction, const Matrix4x4& model);
		static WireframeMesh cross3d(const Matrix4x4& model);
		static WireframeMesh grid(unsigned int half_x_count, unsigned int half_z_count, float x_length, float z_length, const Matrix4x4& model);
		static WireframeMesh polyline(const std::vector<Vector3>& points, const Matrix4x4& model);*/

		static void generate_cursor(WireframeMesh meshes[3], const Matrix4x4& model);
	};
}