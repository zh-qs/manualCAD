#pragma once

#include <list>
#include <vector>
#include <tuple>
#include <functional>
#include "algebra.h"

namespace ManualCAD 
{
	class ZigZagPath {
		struct PathLine {
			std::vector<Vector2> points;
			bool looped;
		};

		std::list<PathLine> lines;

		using SegmentCheck = std::function<bool(const Vector2&, const Vector2&)>;
		struct LoopIntersection {
			float x;
			const PathLine* line;
			int idx;
		};

		struct PathSegment {
			float y;
			LoopIntersection start, end;
		};

		std::vector<std::list<LoopIntersection>> calculate_intersection_of_loops_with_rows(const int& rows, const Vector2& min, const Vector2& max, const float& row_width) const;
		std::vector<std::vector<PathSegment>> make_segment_list_outside_loops(const std::vector<std::list<LoopIntersection>>& intersections, const Vector2& min, const Vector2& max, const float& row_width) const;
		std::vector<std::vector<PathSegment>> make_segment_list_with_check(const std::vector<std::list<LoopIntersection>>& intersections, const SegmentCheck& check, const Vector2& min, const Vector2& max, const float& row_width) const;
		std::vector<std::vector<Vector2>> link_segments_and_create_paths(std::vector<std::vector<PathSegment>>& zigzag_segments, const Vector2& min, const Vector2& max, const float& row_width) const;
	public:
		void add_line(const std::vector<Vector2>& line, bool looped) { lines.push_back({ line, looped }); }
		std::vector<std::vector<Vector2>> generate_paths_outside_loops(const Vector2& min, const Vector2& max, const float radius, const float epsilon) const;
		std::vector<std::vector<Vector2>> generate_paths_outside_loops(const Range<float>& urange, const Range<float>& vrange, const float radius, const float epsilon) {
			return generate_paths_outside_loops(Vector2{ urange.from, vrange.from }, Vector2{ urange.to, vrange.to }, radius, epsilon);
		}
		std::vector<std::vector<Vector2>> generate_paths_excluding_segments(const Vector2& min, const Vector2& max, const float radius, const float epsilon, const SegmentCheck& check) const;
		std::vector<std::vector<Vector2>> generate_paths_excluding_segments(const Range<float>& urange, const Range<float>& vrange, const float radius, const float epsilon, const SegmentCheck& check) const {
			return generate_paths_excluding_segments(Vector2{ urange.from, vrange.from }, Vector2{ urange.to, vrange.to }, radius, epsilon, check);
		}
	};
}