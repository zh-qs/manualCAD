#pragma once

#include "algebra.h"
#include <vector>

namespace ManualCAD
{
	class ParametricCurve {
	public:
		virtual std::vector<Vector3> get_bezier_points() const = 0;
	};
}
