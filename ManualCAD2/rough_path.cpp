#include "rough_path.h"
#include "renderer.h"
#include "shader_library.h"
#include "height_map.h"
#include "height_map_renderer.h"

namespace ManualCAD
{
	HeightMap RoughPath::render_height_map(const Vector3& size)
	{
		HeightMapRenderer r{ surfaces, box };
		return r.render_height_map(size);
	}

	RoughPath::RoughPath(const std::list<const ParametricSurfaceObject*>& surfaces, const Vector3& center, const Vector2& min, const Vector2& max, const float bottom_height, const float height, const float scale) : surfaces(surfaces), bottom_height(bottom_height)
	{
		box.x_min = min.x;
		box.x_max = max.x;
		box.z_min = min.y;
		box.z_max = max.y;
		box.y_min = center.y + scale * bottom_height;
		box.y_max = center.y + scale * height;
	}

	float max_sample(const HeightMap& map, const Vector2& v, const float radius)
	{
		float result = 0;
		const float radius45deg = sqrtf(0.5f) * radius;
		const Vector2 offsets[] = {
			{0.0f,0.0f},
			{radius, 0.0f},
			{radius45deg, radius45deg},
			{0.0f, radius},
			{-radius45deg, radius45deg},
			{-radius, 0.0f},
			{-radius45deg,-radius45deg},
			{0.0f, -radius},
			{radius45deg, -radius45deg}
		};
		for (int i = 0; i < sizeof(offsets) / sizeof(Vector2); ++i)
		{
			const auto coords = map.position_to_pixel(v + offsets[i]);
			result = std::max(result, map.get_pixel(coords.y, coords.x));
		}
		return result;
	}

	std::vector<Vector3> RoughPath::generate_path(int levels, const Vector3& size, const float radius, const float r_epsilon, const float h_epsilon)
	{
		const float level_height = (size.y - bottom_height) / levels;
		Vector2 min = { -0.5f * size.x - radius - r_epsilon, -0.5f * size.z - radius - r_epsilon },
			max = -min;

		const float row_width = 2 * radius - r_epsilon;
		int rows = static_cast<int>((max.y - min.y) / row_width) + 1;

		Vector3 map_size = { size.x, size.y - bottom_height, size.z };
		auto height_map = render_height_map(map_size);

		std::list<Vector3> path;
		for (int i = levels - 1; i >= 0; --i)
		{
			const float h = bottom_height + i * level_height;
			for (int j = 0; j < rows; ++j)
			{
				Vector2 start, end;
				int real_j = (i & 1) ? (rows - j - 1) : j;
				if ((i + real_j) & 1) // non-parite
				{
					//path.push_back({ max.x, h, min.y + real_j * row_width });
					//path.push_back({ min.x, h, min.y + real_j * row_width });
					start = { max.x, min.y + real_j * row_width };
					end = { min.x, min.y + real_j * row_width };
				}
				else
				{
					//path.push_back({ min.x, h, min.y + real_j * row_width });
					//path.push_back({ max.x, h, min.y + real_j * row_width });
					start = { min.x, min.y + real_j * row_width };
					end = { max.x, min.y + real_j * row_width };
				}
				int last_p_on_h = 0;
				for (int p = 0; p < TEX_DIM; ++p)
				{
					const auto v = lerp(start, end, static_cast<float>(p) / (TEX_DIM - 1));
					const auto coords = height_map.position_to_pixel(v);
					//const float map_h = bottom_height + height_map.get_pixel(coords.y, coords.x);
					const float map_h = bottom_height + max_sample(height_map, v, radius);
					if (h >= map_h)
					{
						if (last_p_on_h != p - 1 || p == TEX_DIM - 1)
							path.push_back({ v.x, h + h_epsilon, v.y });
						last_p_on_h = p;
					}
					else
					{
						path.push_back({ v.x, map_h + h_epsilon, v.y });
					}
				}
			}
		}

		return { path.begin(),path.end() };
	}
}
