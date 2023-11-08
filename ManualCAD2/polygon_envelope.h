#pragma once

#include "algebra.h"
#include <vector>
#include <list>

namespace ManualCAD
{
	// Calculates and encapsulates envelope of a set of 2D polygons
	class PolygonEnvelope {
		std::vector<Vector2> points;

		void remove_loops();
		void remove_sharp_corners();
	public:
		// polygons should be oriented ccw
		PolygonEnvelope(const std::vector<std::vector<Vector2>>& polygons_vecs);
		const std::vector<Vector2>& get_points() const { return points; }

		void expand(float d);

		class Builder {
			std::vector<std::vector<Vector2>> polygons;
		public:
			void add_polygon(const std::vector<Vector2>& polygon) { polygons.push_back(polygon); /* TODO orientation */ }
			PolygonEnvelope build() const {
				return { polygons };
			}
		};
	};
}