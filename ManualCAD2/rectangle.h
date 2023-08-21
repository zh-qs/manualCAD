#pragma once

namespace ManualCAD
{
	struct Rectangle {
		float x_min, x_max, y_min, y_max;

		inline bool contains(float x, float y) const { return x >= x_min && x <= x_max && y >= y_min && y <= y_max; }
	};
}