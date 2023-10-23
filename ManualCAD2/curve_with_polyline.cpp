#include "curve_with_polyline.h"
#include "renderer.h"
#include "object.h"

namespace ManualCAD
{
	//std::vector<Vector3> CurveWithPolyline::points() const
	//{
	//	std::vector<Vector3> res(point_ptrs.size());
	//	for (int i = 0; i < point_ptrs.size(); ++i)
	//	{
	//		const auto& m = point_ptrs[i]->get_const_renderable().get_model_matrix();
	//		res[i] = { m.elem[0][3], m.elem[1][3], m.elem[2][3] };
	//	}
	//	return res;
	//}

	void CurveWithPolyline::set_data(const std::vector<Vector3>& points)
	{
		vbo.bind();
		vbo.set_dynamic_data(reinterpret_cast<const float*>(points.data()), points.size() * sizeof(Vector3));
		std::vector<unsigned int> line_adj_indices(4 * (points.size() / 3));
		actual_point_count = points.size();
		group_count = points.size() / 3;
		for (int i = 0; i < group_count; ++i)
		{
			const int ii = 4 * i;
			line_adj_indices[ii] = 3 * i;
			line_adj_indices[ii + 1] = 3 * i + 1;
			line_adj_indices[ii + 2] = 3 * i + 2;
			line_adj_indices[ii + 3] = 3 * i + 3;
		}
		ebo.bind();
		ebo.set_dynamic_data(line_adj_indices.data(), line_adj_indices.size() * sizeof(unsigned int));
	}

	void CurveWithPolyline::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_curve_and_polyline(*this, color, width, height, thickness);
	}

	void CurveWithPolyline::generate_curve(const std::vector<const Point*>& points, bool draw_polyline, bool draw_curve)
	{
		point_count = points.size();
		if (points.empty()) {
			set_data({});
			return;
		}
		int mod = points.size() % 3;
		if (mod < 2) mod = 1 - mod; // number of nan vertices we need to add to vector
		std::vector<Vector3> res(points.size() + mod);
		int i;
		for (i = 0; i < points.size(); ++i)
		{
			const auto& m = points[i]->get_const_drawable().get_model_matrix();
			res[i] = { m.elem[0][3], m.elem[1][3], m.elem[2][3] };
		}
		for (int j = 0; j < mod; ++j)
			res[i + j] = Vector3::nan(); // fill remaining space with nans
		set_data(res);
		this->draw_polyline = draw_polyline;
		this->draw_curve = draw_curve;
	}

	void CurveWithPolyline::generate_curve(const std::vector<Vector3>& points, bool draw_polyline, bool draw_curve)
	{
		set_data(points);
		this->draw_polyline = draw_polyline;
		this->draw_curve = draw_curve;
	}
}