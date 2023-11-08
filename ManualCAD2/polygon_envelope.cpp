#include "polygon_envelope.h"
#include "application_settings.h"
#include "planimetrics.h"
#include <vector>
#include <map>

namespace ManualCAD
{
	struct PolygonPoint {
		Vector2 coords;
		Vector2 intersecting_coords; // intersection point on segment following the point
		std::vector<PolygonPoint>* intersecting; // intersecting polygon
		int intersecting_at = -1; // index of point before intersection in other polygon
	};

	void PolygonEnvelope::remove_loops() // TODO remove other types if self-intersections
	{
		constexpr int MAX_POINTS_FORWARD = 200; // we assume that loops after expanding are small and there is no need to loop for whole polygon

		std::list<std::tuple<Vector2, int, int>> loop_skips;

		for (int i = 0; i < points.size(); ++i)
		{
			auto point1 = points[i],
				next1 = i == points.size() - 1 ? points[0] : points[i + 1];

			for (int j = 2; j <= MAX_POINTS_FORWARD; ++j) // 2 because we do not check adjacent edges
			{
				auto point2 = points[(i + j) % points.size()],
					next2 = points[(i + j + 1) % points.size()];
				
				Vector2 intersection;
				if (Planimetrics::intersect(point1, next1, point2, next2, intersection)) 
				{
					loop_skips.push_back({ intersection, i, (i + j + 1) % points.size() });
					i += j + 1;
					break;
				}
			}
		}

		if (loop_skips.empty())
			return;

		std::vector<Vector2> result;
		result.reserve(points.size());
		auto it = loop_skips.begin();

		int start = std::get<2>(*it), end = std::get<1>(*it);
		result.push_back(std::get<0>(*it));
		++it;
		int i;
		for (i = start; i != end; i = (i + 1) % points.size())
		{
			if (it == loop_skips.end())
				break;

			result.push_back(points[i]);

			const int loop_idx = std::get<1>(*it);
			if (i == loop_idx)
			{
				result.push_back(std::get<0>(*it));
				i = std::get<2>(*it) - 1; // minus one because it will be incremented in for loop
				++it;
			}
		}

		points = std::move(result);
	}

	void PolygonEnvelope::remove_sharp_corners()
	{
		constexpr float THRESHOLD = -0.5f;

		std::vector<Vector2> result;
		result.reserve(points.size());

		for (int i = 0; i < points.size(); ++i)
		{
			auto point = points[i],
				next = (i == points.size() - 1) ? points[0] : points[i + 1],
				prev = (i == 0) ? points[points.size() - 1] : points[i - 1];

			auto v1 = normalize(point - prev),
				v2 = normalize(next - point);

			if (dot(v1, v2) > THRESHOLD)
				result.push_back(point);
		}

		points = std::move(result);
		//constexpr int MAX_POINTS_FORWARD = 20; // we assume that loops after expanding are small and there is no need to loop for whole polygon
		//constexpr float THRESHOLD = 0.005f;

		//std::list<std::pair<int, int>> corner_skips;

		//for (int i = 0; i < points.size(); ++i)
		//{
		//	auto point1 = points[i];

		//	for (int j = 2; j <= MAX_POINTS_FORWARD; ++j) // 2 because we do not check adjacent edges
		//	{
		//		auto point2 = points[(i + j) % points.size()];

		//		if ((point2-point1).length() < THRESHOLD)
		//		{
		//			corner_skips.push_back({ i, (i + j + 1) % points.size() });
		//		}
		//	}
		//}

		//if (corner_skips.empty())
		//	return;

		//std::vector<Vector2> result;
		//result.reserve(points.size());
		//auto it = corner_skips.begin();

		//int start = it->second, end = it->first;
		//++it;
		//int i;
		//for (i = start; i != end; i = (i + 1) % points.size())
		//{
		//	if (it == corner_skips.end())
		//		break;

		//	result.push_back(points[i]);

		//	const int corner_idx = it->first;
		//	if (i == corner_idx)
		//	{
		//		i = it->second - 1; // minus one because it will be incremented in for corner
		//		++it;
		//	}
		//}

		//points = std::move(result);
	}

	PolygonEnvelope::PolygonEnvelope(const std::vector<std::vector<Vector2>>& polygons_vecs)
	{
		size_t point_count = 0;
		for (const auto& p : polygons_vecs)
			point_count += p.size();

		// copy polygons to a new structure
		std::vector<std::vector<PolygonPoint>> polygons(polygons_vecs.size());
		for (int i = 0; i < polygons.size(); ++i)
		{
			polygons[i].reserve(polygons_vecs[i].size());
			for (const auto& v : polygons_vecs[i])
			{
				polygons[i].push_back(PolygonPoint{ v,{},nullptr,-1 });
			}
		}

		// calculate intersections
		for (int i = 0; i < polygons.size(); ++i)
		{
			for (int j = i + 1; j < polygons.size(); ++j)
			{
				auto& p1 = polygons[i];
				auto& p2 = polygons[j];
				for (int k = 0; k < p1.size(); ++k)
				{
					for (int l = 0; l < p2.size(); ++l)
					{
						Vector2 intersection;
						Vector2 seg1start = p1[k].coords,
							seg2start = p2[l].coords;
						Vector2 seg1end = (k == p1.size() - 1) ? p1[0].coords : p1[k + 1].coords,
							seg2end = (l == p2.size() - 1) ? p2[0].coords : p2[l + 1].coords; // TODO optimize
						if (!Planimetrics::intersect(seg1start, seg1end, seg2start, seg2end, intersection))
							continue;

						p1[k].intersecting_coords = p2[l].intersecting_coords = intersection;
						p1[k].intersecting = &polygons[j];
						p1[k].intersecting_at = l;
						p2[l].intersecting = &polygons[i];
						p2[l].intersecting_at = k;
					}
				}
			}
		}

		// find x-most point of all polygons
		const std::vector<PolygonPoint>* start_polygon = &polygons[0];
		int start_point_idx = 0;

		for (const auto& p : polygons)
		{
			for (int i = 0; i < p.size(); ++i)
			{
				if (p[i].coords.x < (*start_polygon)[start_point_idx].coords.x
					|| (p[i].coords.x == (*start_polygon)[start_point_idx].coords.x && p[i].coords.y < (*start_polygon)[start_point_idx].coords.y))
				{
					start_polygon = &p;
					start_point_idx = i;
				}
			}
		}

		// go along envelope
		std::list<Vector2> envelope;
		const std::vector<PolygonPoint>* current_polygon = start_polygon;
		int current_idx = start_point_idx;
		do
		{
			const auto& point = (*current_polygon)[current_idx];
			envelope.push_back(point.coords);
			if (point.intersecting != nullptr) // we went to intersection point
			{
				envelope.push_back(point.intersecting_coords);
				current_polygon = point.intersecting;
				current_idx = point.intersecting_at + 1; // we add 1 to skip point before intersection on new polygon
			}
			else
			{
				++current_idx;
			}
			if (current_idx == current_polygon->size())
				current_idx = 0;
		} while (envelope.size() < point_count && (current_polygon != start_polygon || current_idx != start_point_idx));

		points.reserve(envelope.size());
		points.insert(points.begin(), envelope.begin(), envelope.end());

		remove_sharp_corners();
	}

	void PolygonEnvelope::expand(float d)
	{
		std::vector<Vector2> expanded_points(points.size());
		for (int i = 0; i < points.size(); ++i)
		{
			auto point = points[i],
				next = i == points.size() - 1 ? points[0] : points[i + 1],
				prev = (i == 0) ? points[points.size() - 1] : points[i - 1];

			auto from_prev = point - prev;

			Vector2 nprev = normalize(Vector2{ from_prev.y, -from_prev.x });
			expanded_points[i] = 0.5f * (point + prev) + d * nprev;
		}
		points = std::move(expanded_points);

		remove_loops();
	}
}
