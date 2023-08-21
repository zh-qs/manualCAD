#include "textured_wireframe_mesh.h"
#include "renderer.h"
#include <cmath>

namespace ManualCAD
{
	void TexturedWireframeMesh::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_textured_wireframe(*this, color, width, height, thickness);
	}

	void TexturedWireframeMesh::set_data(const std::vector<Vector3>& points, const std::vector<Vector2>& tex_coords, const std::vector<IndexPair>& line_indices)
	{
		vbo.bind();
		vbo.set_dynamic_data(reinterpret_cast<const float*>(points.data()), points.size() * sizeof(Vector3));
		texvbo.bind();
		texvbo.set_dynamic_data(reinterpret_cast<const float*>(tex_coords.data()), tex_coords.size() * sizeof(Vector2));
		ebo.bind();
		ebo.set_dynamic_data(reinterpret_cast<const unsigned int*>(line_indices.data()), line_indices.size() * sizeof(IndexPair));

		point_count = points.size();
		line_count = line_indices.size();
	}

	void TexturedWireframeMesh::generate_torus(float large_radius, float small_radius, unsigned int divisions_x, unsigned int divisions_y)
	{
		std::vector<Vector3> points((divisions_x + 1) * (divisions_y + 1));
		std::vector<Vector2> tex_coords((divisions_x + 1) * (divisions_y + 1));
		std::vector<IndexPair> line_indices(2 * divisions_x * divisions_y);
		const float step_x = TWO_PI / divisions_x;
		const float step_y = TWO_PI / divisions_y;
		int p_idx = 0, l_idx = 0;
		for (unsigned int x = 0; x < divisions_x + 1; ++x) {
			for (unsigned int y = 0; y < divisions_y + 1; ++y) {
				const float ang_x = x * step_x, ang_y = y * step_y;
				const float Rrcos = large_radius + small_radius * cosf(ang_x);
				// create point for angles (ang_x, ang_y)
				points[x * (divisions_y + 1) + y] = { Rrcos * cosf(ang_y), small_radius * sinf(ang_x), Rrcos * sinf(ang_y) };
				tex_coords[x * (divisions_y + 1) + y] = { ang_x / TWO_PI, ang_y / TWO_PI };
			}
		}
		for (unsigned int x = 0; x < divisions_x; ++x) {
			for (unsigned int y = 0; y < divisions_y; ++y) {
				// connect lines on small circle
				line_indices[l_idx++] = { x * (divisions_y + 1) + y,(x + 1) * (divisions_y + 1) + y };
				// connect lines on large circle
				line_indices[l_idx++] = { x * (divisions_y + 1) + y, x * (divisions_y + 1) + y + 1 };
			}
		}

		set_data(points, tex_coords, line_indices);
	}
}
