#pragma once

#include "algebra.h"
#include "parametric_surface.h"
#include "object.h"
#include <list>
#include <vector>
#include "plane_xz.h"

namespace ManualCAD
{
	class SurfacePath {
		std::list<const ParametricSurfaceWithSecondDerivative*> surfaces;
	public:
		SurfacePath(const std::list<const ParametricSurfaceWithSecondDerivative*>& surfaces) : surfaces(surfaces) {}
		SurfacePath(const std::list<const ParametricSurfaceObject*>& surfaces) : surfaces(surfaces.begin(), surfaces.end()) {}

		std::vector<std::vector<std::vector<Vector2>>> generate_paths(const PlaneXZ& plane, const float radius, const float epsilon) const;
	};
}