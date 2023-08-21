#include "wireframe_mesh.h"
#include "renderer.h"
#include <cmath>

namespace ManualCAD
{
	const Matrix4x4 WireframeMesh::IDENTITY = Matrix4x4::identity();

	void WireframeMesh::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_wireframe(*this, color, width, height, thickness);
	}

	void WireframeMesh::set_data(const std::vector<Vector3>& points, const std::vector<IndexPair>& line_indices)
	{
		vbo.bind();
		vbo.set_dynamic_data(reinterpret_cast<const float*>(points.data()), points.size() * sizeof(Vector3));
		ebo.bind();
		ebo.set_dynamic_data(reinterpret_cast<const unsigned int*>(line_indices.data()), line_indices.size() * sizeof(IndexPair));

		point_count = points.size();
		line_count = line_indices.size();
	}

	void WireframeMesh::generate_torus(float large_radius, float small_radius, unsigned int divisions_x, unsigned int divisions_y)
	{
		std::vector<Vector3> points(divisions_x * divisions_y);
		std::vector<IndexPair> line_indices(2 * divisions_x * divisions_y);
		const float step_x = TWO_PI / divisions_x;
		const float step_y = TWO_PI / divisions_y;
		int p_idx = 0, l_idx = 0;
		for (unsigned int x = 0; x < divisions_x - 1; ++x) {
			for (unsigned int y = 0; y < divisions_y; ++y) {
				const float ang_x = x * step_x, ang_y = y * step_y;
				const float Rrcos = large_radius + small_radius * cosf(ang_x);
				// create point for angles (ang_x, ang_y)
				points[x * divisions_y + y] = { Rrcos * cosf(ang_y), small_radius * sinf(ang_x), Rrcos * sinf(ang_y) };
				// connect lines on small circle
				line_indices[l_idx++] = { x * divisions_y + y,(x + 1) * divisions_y + y };
				// connect lines on large circle
				line_indices[l_idx++] = { x * divisions_y + y, x * divisions_y + y + 1 };
			}
			// correct last line indices
			line_indices[l_idx - 1].j = x * divisions_y;
		}
		for (unsigned int y = 0; y < divisions_y; ++y) {
			const float ang_x = (divisions_x - 1) * step_x, ang_y = y * step_y;
			const float Rrcos = large_radius + small_radius * cosf(ang_x);
			// create point for angles (ang_x, ang_y)
			points[(divisions_x - 1) * divisions_y + y] = { Rrcos * cosf(ang_y), small_radius * sinf(ang_x), Rrcos * sinf(ang_y) };
			// connect lines on small circle
			line_indices[l_idx++] = { (divisions_x - 1) * divisions_y + y, y };
			// connect lines on large circle
			line_indices[l_idx++] = { (divisions_x - 1) * divisions_y + y, (divisions_x - 1) * divisions_y + y + 1 };
		}
		// correct last line indices
		line_indices[l_idx - 1].j = (divisions_x - 1) * divisions_y;
		set_data(points, line_indices);
	}

	void WireframeMesh::generate_axis_from_origin(const Vector3& direction)
	{
		set_data({ { 0,0,0 }, direction }, { {1,0} });
	}

	void WireframeMesh::generate_grid(unsigned int half_x_count, unsigned int half_z_count, float x_length, float z_length)
	{
		std::vector<Vector3> points(4 * (half_x_count + half_z_count) + 4);
		std::vector<IndexPair> line_indices(2 * (half_x_count + half_z_count) + 2);
		unsigned int iv = 0, il = 0;
		float total_x = x_length * half_x_count, total_z = z_length * half_z_count;
		points[iv++] = { 0.0f,0.0f,total_z };
		points[iv++] = { 0.0f,0.0f,-total_z };
		line_indices[il++] = { 0,1 };
		for (unsigned int x = 1; x <= half_x_count; ++x)
		{
			points[iv++] = { x * x_length,0.0f,total_z };
			points[iv++] = { x * x_length,0.0f,-total_z };

			line_indices[il++] = { iv - 2,iv - 1 };

			points[iv++] = { x * -x_length,0.0f,total_z };
			points[iv++] = { x * -x_length,0.0f,-total_z };

			line_indices[il++] = { iv - 2,iv - 1 };
		}
		points[iv++] = { total_x,0.0f,0.0f };
		points[iv++] = { -total_x,0.0f,0.0f };
		line_indices[il++] = { iv - 2,iv - 1 };
		for (unsigned int z = 1; z <= half_z_count; ++z)
		{
			points[iv++] = { total_x,0.0f,z * z_length };
			points[iv++] = { -total_x,0.0f,z * z_length };

			line_indices[il++] = { iv - 2,iv - 1 };

			points[iv++] = { total_x,0.0f,z * -z_length };
			points[iv++] = { -total_x,0.0f,z * -z_length };

			line_indices[il++] = { iv - 2,iv - 1 };
		}
		set_data(points, line_indices);
	}

	//WireframeMesh WireframeMesh::torus(float large_radius, float small_radius, unsigned int divisions_x, unsigned int divisions_y, const Matrix4x4& model)
	//{
	//	WireframeMesh result(divisions_x * divisions_y, 2 * divisions_x * divisions_y, model);
	//	const float step_x = TWO_PI / divisions_x;
	//	const float step_y = TWO_PI / divisions_y;
	//	int p_idx = 0, l_idx = 0;
	//	for (unsigned int x = 0; x < divisions_x - 1; ++x) {
	//		for (unsigned int y = 0; y < divisions_y; ++y) {
	//			const float ang_x = x * step_x, ang_y = y * step_y;
	//			const float Rrcos = large_radius + small_radius * cosf(ang_x);
	//			// create point for angles (ang_x, ang_y)
	//			result.points[x * divisions_y + y] = { Rrcos * cosf(ang_y), small_radius * sinf(ang_x), Rrcos * sinf(ang_y) };
	//			// connect lines on small circle
	//			result.line_indices[l_idx++] = { x * divisions_y + y,(x + 1) * divisions_y + y };
	//			// connect lines on large circle
	//			result.line_indices[l_idx++] = { x * divisions_y + y, x * divisions_y + y + 1 };
	//		}
	//		// correct last line indices
	//		result.line_indices[l_idx - 1].j = x * divisions_y;
	//	}
	//	for (unsigned int y = 0; y < divisions_y; ++y) {
	//		const float ang_x = (divisions_x - 1) * step_x, ang_y = y * step_y;
	//		const float Rrcos = large_radius + small_radius * cosf(ang_x);
	//		// create point for angles (ang_x, ang_y)
	//		result.points[(divisions_x - 1) * divisions_y + y] = { Rrcos * cosf(ang_y), small_radius * sinf(ang_x), Rrcos * sinf(ang_y) };
	//		// connect lines on small circle
	//		result.line_indices[l_idx++] = { (divisions_x - 1) * divisions_y + y, y };
	//		// connect lines on large circle
	//		result.line_indices[l_idx++] = { (divisions_x - 1) * divisions_y + y, (divisions_x - 1) * divisions_y + y + 1 };
	//	}
	//	// correct last line indices
	//	result.line_indices[l_idx - 1].j = (divisions_x - 1) * divisions_y;
	//	return result;
	//}
	//
	//WireframeMesh WireframeMesh::axis_from_origin(const Vector3& direction, const Matrix4x4& model)
	//{
	//	WireframeMesh res(2, 1, model);
	//	res.points[0] = { 0,0,0 };
	//	res.points[1] = direction;
	//	res.line_indices[0] = { 1,0 };
	//	return res;
	//}
	//
	//WireframeMesh WireframeMesh::cross3d(const Matrix4x4& model)
	//{
	//	constexpr float size = 0.3f;
	//	WireframeMesh res(6, 3, model);
	//	res.points[0] = { -size,0,0 };
	//	res.points[1] = { size,0,0 };
	//	res.points[2] = { 0,-size,0 };
	//	res.points[3] = { 0,size,0 };
	//	res.points[4] = { 0,0,-size };
	//	res.points[5] = { 0,0,size };
	//	res.line_indices[0] = { 0,1 };
	//	res.line_indices[1] = { 2,3 };
	//	res.line_indices[2] = { 4,5 };
	//	return res;
	//}
	//
	//WireframeMesh WireframeMesh::grid(unsigned int half_x_count, unsigned int half_z_count, float x_length, float z_length, const Matrix4x4& model)
	//{
	//	WireframeMesh result(4 * (half_x_count + half_z_count) + 4, 2 * (half_x_count + half_z_count) + 2, model);
	//	unsigned int iv = 0, il = 0;
	//	float total_x = x_length * half_x_count, total_z = z_length * half_z_count;
	//	result.points[iv++] = { 0.0f,0.0f,total_z };
	//	result.points[iv++] = { 0.0f,0.0f,-total_z };
	//	result.line_indices[il++] = { 0,1 };
	//	for (unsigned int x = 1; x <= half_x_count; ++x)
	//	{
	//		result.points[iv++] = { x * x_length,0.0f,total_z };
	//		result.points[iv++] = { x * x_length,0.0f,-total_z };
	//
	//		result.line_indices[il++] = { iv - 2,iv - 1 };
	//
	//		result.points[iv++] = { x * -x_length,0.0f,total_z };
	//		result.points[iv++] = { x * -x_length,0.0f,-total_z };
	//
	//		result.line_indices[il++] = { iv - 2,iv - 1 };
	//	}
	//	result.points[iv++] = { total_x,0.0f,0.0f };
	//	result.points[iv++] = { -total_x,0.0f,0.0f };
	//	result.line_indices[il++] = { iv - 2,iv - 1 };
	//	for (unsigned int z = 1; z <= half_z_count; ++z)
	//	{
	//		result.points[iv++] = { total_x,0.0f,z * z_length };
	//		result.points[iv++] = { -total_x,0.0f,z * z_length };
	//
	//		result.line_indices[il++] = { iv - 2,iv - 1 };
	//
	//		result.points[iv++] = { total_x,0.0f,z * -z_length };
	//		result.points[iv++] = { -total_x,0.0f,z * -z_length };
	//
	//		result.line_indices[il++] = { iv - 2,iv - 1 };
	//	}
	//	return result;
	//}
	//
	//WireframeMesh WireframeMesh::polyline(const std::vector<Vector3>& points, const Matrix4x4& model)
	//{
	//	if (points.empty()) return { 0,0,model };
	//	WireframeMesh res(points.size(), points.size() - 1, model);
	//	res.points = points;
	//	for (unsigned int i = 0; i < res.line_indices.size(); ++i)
	//		res.line_indices[i] = { i,i + 1 };
	//	return res;
	//}
	//
	void WireframeMesh::generate_cursor(WireframeMesh meshes[3], const Matrix4x4& model)
	{
		constexpr float scale = 0.25f;
		meshes[0].generate_axis_from_origin({ scale,0,0 });
		meshes[1].generate_axis_from_origin({ 0,scale,0 });
		meshes[2].generate_axis_from_origin({ 0,0,scale });
		meshes[0].set_model_matrix(model);
		meshes[1].set_model_matrix(model);
		meshes[2].set_model_matrix(model);
	}
}
