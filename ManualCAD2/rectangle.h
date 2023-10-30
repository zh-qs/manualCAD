#pragma once

#include "algebra.h"

namespace ManualCAD
{
	struct Rectangle {
		float x_min, x_max, y_min, y_max;

		inline bool contains(float x, float y) const { return x >= x_min && x <= x_max && y >= y_min && y <= y_max; }
		inline bool contains(const Vector2& v) const { return v.x >= x_min && v.x <= x_max && v.y >= y_min && v.y <= y_max; }

		static inline Rectangle infinite() {
			const float inf = INFINITY;
			return { -inf,inf,-inf,inf };
		}

		static inline Rectangle degenerate() {
			const float inf = INFINITY;
			return { inf,-inf,inf,-inf };
		}
	};
}