#include "line.h"
#include "renderer.h"

namespace ManualCAD
{
	void Line::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_line(*this, color, width, height, thickness);
	}

	void Line::set_data(const std::vector<Vector3>& points)
	{
		vbo.bind();
		vbo.set_static_data(reinterpret_cast<const float*>(points.data()), points.size() * sizeof(Vector3));
		point_count = points.size();
	}

	void Line2D::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_line_2d(*this, color, width, height, thickness);
	}

	void Line2D::set_data(const std::vector<Vector2>& points)
	{
		vbo.bind();
		vbo.set_static_data(reinterpret_cast<const float*>(points.data()), points.size() * sizeof(Vector2));
		point_count = points.size();
	}
}
