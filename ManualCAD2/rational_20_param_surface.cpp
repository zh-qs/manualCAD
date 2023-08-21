#include "rational_20_param_surface.h"
#include "renderer.h"

namespace ManualCAD
{
	void Rational20ParamSurface::set_data(const std::vector<Vector3>& points)
	{
		// points size have to be divisible by 20!
		vbo.bind();
		vbo.set_dynamic_data(reinterpret_cast<const float*>(points.data()), points.size() * sizeof(Vector3));
		point_count = points.size();

		if (!ebo_set)
		{
			size_t patches = points.size() / 20;
			std::vector<unsigned int> indices;
			indices.reserve(40 * patches);
			for (int i = 0; i < patches; ++i)
			{
				const int offset = 20 * i;
				indices.push_back(offset); indices.push_back(offset + 1);
				indices.push_back(offset + 1); indices.push_back(offset + 2);
				indices.push_back(offset + 2); indices.push_back(offset + 3);

				indices.push_back(offset); indices.push_back(offset + 4);
				indices.push_back(offset + 4); indices.push_back(offset + 5);
				indices.push_back(offset + 1); indices.push_back(offset + 6);
				indices.push_back(offset + 2); indices.push_back(offset + 7);
				indices.push_back(offset + 3); indices.push_back(offset + 9);
				indices.push_back(offset + 8); indices.push_back(offset + 9);

				indices.push_back(offset + 4); indices.push_back(offset + 10);
				indices.push_back(offset + 9); indices.push_back(offset + 15);

				indices.push_back(offset + 10); indices.push_back(offset + 11);
				indices.push_back(offset + 10); indices.push_back(offset + 16);
				indices.push_back(offset + 12); indices.push_back(offset + 17);
				indices.push_back(offset + 13); indices.push_back(offset + 18);
				indices.push_back(offset + 14); indices.push_back(offset + 15);
				indices.push_back(offset + 15); indices.push_back(offset + 19);

				indices.push_back(offset + 16); indices.push_back(offset + 17);
				indices.push_back(offset + 17); indices.push_back(offset + 18);
				indices.push_back(offset + 18); indices.push_back(offset + 19);
			}
			ebo_set = true;
			contour_indices_count = indices.size();
			ebo.bind();
			ebo.set_static_data(indices.data(), indices.size() * sizeof(unsigned int));
		}
	}

	void Rational20ParamSurface::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_rational_20_param_surface(*this, color, width, height, thickness);
	}
}
