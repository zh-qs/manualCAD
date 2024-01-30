#pragma once

#include <list>
#include <vector>
#include "algebra.h"

namespace ManualCAD
{
	class CurvePath {
		std::list<std::vector<Vector3>> beziers;
	public:
		void add_curve_bezier_points(const std::vector<Vector3>& points) { beziers.push_back(points); }
		void add_curve_bezier_points(std::vector<Vector3>&& points) { beziers.push_back(std::move(points)); }

		std::vector<std::vector<Vector3>> generate_paths() const;
		void project_curves(float y);
	};
}