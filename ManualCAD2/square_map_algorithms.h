#pragma once
#include "parametric_surface.h"
#include <tuple>
#include <stack>

namespace ManualCAD
{
	class SquareMapAlgorithms {
		inline static float pix_to_coord(const Range<float>& range, int pixel, int dim)
		{
			return pixel * (range.to - range.from) / dim + range.from;
		}

	public:
		template <class SqMap, class T> 
		inline static void flood_fill(SqMap& map, int x, int y, const T& value, const ParametricSurface& surf) {
			const auto old = map.get_pixel(x, y);
			if (value == old) return;

			const auto urange = surf.get_u_range(), vrange = surf.get_v_range();

			std::stack<std::pair<int, int>> stack;
			stack.push({ x, y });
			while (!stack.empty())
			{
				const auto coords = stack.top();
				stack.pop();
				map.set_pixel(coords.first, coords.second, value);
				if (coords.first > 0 && map.get_pixel(coords.first - 1, coords.second) == old)
					stack.push({ coords.first - 1, coords.second });
				if (coords.first < map.width - 1 && map.get_pixel(coords.first + 1, coords.second) == old)
					stack.push({ coords.first + 1, coords.second });
				if (coords.second > 0 && map.get_pixel(coords.first, coords.second - 1) == old)
					stack.push({ coords.first, coords.second - 1 });
				if (coords.second < map.height - 1 && map.get_pixel(coords.first, coords.second + 1) == old)
					stack.push({ coords.first, coords.second + 1 });

				if (coords.first == 0 && surf.u_wraps_at_v(pix_to_coord(vrange, coords.second, map.height)) && map.get_pixel(map.width - 1, coords.second) == old)
					stack.push({ map.width - 1, coords.second });
				if (coords.first == map.width - 1 && surf.u_wraps_at_v(pix_to_coord(vrange, coords.second, map.height)) && map.get_pixel(0, coords.second) == old)
					stack.push({ 0, coords.second });

				if (coords.second == 0 && surf.v_wraps_at_u(pix_to_coord(urange, coords.first, map.width)) && map.get_pixel(coords.first, map.height - 1) == old)
					stack.push({ coords.first, map.height - 1 });
				if (coords.second == map.height - 1 && surf.v_wraps_at_u(pix_to_coord(urange, coords.first, map.width)) && map.get_pixel(coords.first, 0) == old)
					stack.push({ coords.first, 0 });
			}
		}
	};
}