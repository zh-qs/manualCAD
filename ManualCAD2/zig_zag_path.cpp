#include "zig_zag_path.h"
#include "planimetrics.h"

namespace ManualCAD
{
	std::vector<std::list<ZigZagPath::LoopIntersection>> ZigZagPath::calculate_intersection_of_loops_with_rows(const int& rows, const Vector2& min, const Vector2& max, const float& row_width) const
	{
		std::vector<std::list<LoopIntersection>> intersections(rows);
		// calculate intersections of loops with rows
		for (int i = 0; i < rows; ++i)
		{
			const Vector2 seg1start{ min.x, min.y + (i + 1) * row_width },
				seg1end{ max.x, min.y + (i + 1) * row_width };
			for (const auto& line : lines)
			{
				for (int j = 0; j < line.points.size() - 1; ++j)
				{
					const auto& seg2start = line.points[j];
					//const auto& seg2end = j == loop.size() - 1 ? loop[0] : loop[j + 1];
					const auto& seg2end = line.points[j + 1];
					Vector2 intersection;
					if (Planimetrics::intersect(seg1start, seg1end, seg2start, seg2end, intersection)) { // TODO optimize for special case: zigzags are horizontal lines
						intersections[i].push_back({ intersection.x, &line, j });
					}
				}
				if (line.looped)
				{
					const auto& seg2start = line.points.back();
					const auto& seg2end = line.points.front();
					Vector2 intersection;
					if (Planimetrics::intersect(seg1start, seg1end, seg2start, seg2end, intersection)) { // TODO optimize for special case: zigzags are horizontal lines
						intersections[i].push_back({ intersection.x, &line, static_cast<int>(line.points.size() - 1) });
					}
				}
			}
			intersections[i].sort([](const auto& a, const auto& b) { return a.x < b.x; });
		}

		return intersections;
	}

	std::vector<std::vector<ZigZagPath::PathSegment>> ZigZagPath::make_segment_list_outside_loops(const std::vector<std::list<LoopIntersection>>& intersections, const Vector2& min, const Vector2& max, const float& row_width) const
	{
		std::vector<std::vector<PathSegment>> zigzag_segments(intersections.size());
		for (int i = 0; i < intersections.size(); ++i)
		{
			PathSegment s;
			s.y = min.y + (i + 1) * row_width;
			s.start = { min.x, nullptr, -1 };
			bool complete = false;
			for (const auto& isec : intersections[i])
			{
				if (complete)
				{
					s.start = isec;
					complete = false;
				}
				else
				{
					s.end = isec;
					zigzag_segments[i].push_back(s);
					complete = true;
				}
			}

			if (ApplicationSettings::DEBUG)
				assert(!complete);

			s.end = { max.x, nullptr, -1 };
			zigzag_segments[i].push_back(s);
		}
		return zigzag_segments;
	}

	std::vector<std::vector<ZigZagPath::PathSegment>> ZigZagPath::make_segment_list_with_check(const std::vector<std::list<LoopIntersection>>& intersections, const SegmentCheck& check, const Vector2& min, const Vector2& max, const float& row_width) const
	{
		std::vector<std::vector<PathSegment>> zigzag_segments(intersections.size());
		for (int i = 0; i < intersections.size(); ++i)
		{
			PathSegment s;
			s.y = min.y + (i + 1) * row_width;
			s.start = { min.x, nullptr, -1 };

			for (const auto& isec : intersections[i])
			{
				s.end = isec;
				if (check({ s.start.x, s.y }, { s.end.x,s.y }))
					zigzag_segments[i].push_back(s);
				s.start = isec;
			}

			s.end = { max.x, nullptr, -1 };
			if (check({ s.start.x, s.y }, { s.end.x,s.y }))
				zigzag_segments[i].push_back(s);
		}
		return zigzag_segments;
	}

	std::vector<std::vector<Vector2>> ZigZagPath::link_segments_and_create_paths(std::vector<std::vector<PathSegment>>& zigzag_segments, const Vector2& min, const Vector2& max, const float& row_width) const
	{
		const size_t rows = zigzag_segments.size();
		std::vector<std::vector<Vector2>> lists;
		std::list<Vector2> current_list;
		int next_idx;
		int first_not_empty = 0;
		bool left_to_right;

		while (first_not_empty < rows && zigzag_segments[first_not_empty].empty())
			++first_not_empty;

		while (first_not_empty < zigzag_segments.size())
		{
			next_idx = 0;
			left_to_right = true;
			int i = first_not_empty;
			bool early_break = false;
			for (; i < rows - 1; ++i)
			{
				const float next_ycoord = min.y + (i + 2) * row_width;
				const PathSegment seg = zigzag_segments[i][next_idx];
				zigzag_segments[i].erase(zigzag_segments[i].begin() + next_idx);
				while (first_not_empty < rows && zigzag_segments[first_not_empty].empty())
					++first_not_empty;

				if (left_to_right)
				{
					current_list.push_back({ seg.start.x, seg.y });
					current_list.push_back({ seg.end.x, seg.y });

					if (zigzag_segments[i + 1].empty())
					{
						early_break = true;
						break;
					}

					if (seg.end.line == nullptr)
						next_idx = zigzag_segments[i + 1].size() - 1;
					else
					{
						const auto& line = *seg.end.line;
						int idx = seg.end.idx;
						int dir;
						if (line.looped)
						{
							if (line.points[(idx + 1) % line.points.size()].y >= line.points[idx].y)
								dir = 1;
							else
								dir = -1;
						}
						else
						{
							if (line.points[idx + 1].y >= line.points[idx].y)
								dir = 1;
							else
								dir = -1;
						}
						int prev_idx = (idx - dir) % line.points.size(); // TODO is non-looped also ok?
						while (line.points[idx].y < next_ycoord && line.points[prev_idx].y < line.points[idx].y)
						{
							prev_idx = idx;
							const int new_idx = line.looped ? ((idx + dir) % line.points.size()) : (idx + dir);
							if (new_idx < 0 || new_idx >= line.points.size())
								break;
							idx = new_idx;
						}
						if (idx < 0 || idx >= line.points.size() || line.points[prev_idx].y >= line.points[idx].y) // end of zigzag
						{
							early_break = true;
							break;
						}

						const float target_x = line.points[idx].x;

						float nearest_x = -INFINITY;
						for (int j = 0; j < zigzag_segments[i + 1].size(); ++j)
						{
							if (std::abs(target_x - zigzag_segments[i + 1][j].end.x) < std::abs(target_x - nearest_x))
							{
								nearest_x = zigzag_segments[i + 1][j].end.x;
								next_idx = j;
							}
						}
					}
					left_to_right = false;
				}
				else
				{
					current_list.push_back({ seg.end.x, seg.y });
					current_list.push_back({ seg.start.x, seg.y });

					if (zigzag_segments[i + 1].empty())
					{
						early_break = true;
						break;
					}

					if (seg.start.line == nullptr)
						next_idx = 0;
					else
					{
						const auto& line = *seg.start.line;
						int idx = seg.start.idx;
						int dir;
						if (line.looped)
						{
							if (line.points[(idx + 1) % line.points.size()].y >= line.points[idx].y)
								dir = 1;
							else
								dir = -1;
						}
						else
						{
							if (line.points[idx + 1].y >= line.points[idx].y)
								dir = 1;
							else
								dir = -1;
						}
						int prev_idx = (idx - dir) % line.points.size(); // TODO is non-looped also ok?
						while (line.points[idx].y < next_ycoord && line.points[prev_idx].y < line.points[idx].y)
						{
							prev_idx = idx;
							const int new_idx = line.looped ? ((idx + dir) % line.points.size()) : (idx + dir);
							if (new_idx < 0 || new_idx >= line.points.size())
								break;
							idx = new_idx;
						}
						if (/*idx < 0 || idx >= line.points.size() || */line.points[prev_idx].y >= line.points[idx].y) // end of zigzag
						{
							early_break = true;
							break;
						}

						const float target_x = line.points[idx].x;

						float nearest_x = -INFINITY;
						for (int j = 0; j < zigzag_segments[i + 1].size(); ++j)
						{
							if (std::abs(target_x - zigzag_segments[i + 1][j].start.x) < std::abs(target_x - nearest_x))
							{
								nearest_x = zigzag_segments[i + 1][j].start.x;
								next_idx = j;
							}
						}
					}
					left_to_right = true;
				}
			}
			if (early_break || i < rows - 1)
			{
				lists.push_back({ current_list.begin(), current_list.end() });
				current_list.clear();
				continue;
			}

			if (left_to_right)
			{
				const PathSegment seg = zigzag_segments[rows - 1][next_idx];
				current_list.push_back({ seg.start.x, seg.y });
				current_list.push_back({ seg.end.x, seg.y });
			}
			else
			{
				const PathSegment seg = zigzag_segments[rows - 1][next_idx];
				current_list.push_back({ seg.end.x, seg.y });
				current_list.push_back({ seg.start.x, seg.y });
			}
			zigzag_segments[rows - 1].erase(zigzag_segments[rows - 1].begin() + next_idx);
			while (first_not_empty < rows && zigzag_segments[first_not_empty].empty())
				++first_not_empty;
			lists.push_back({ current_list.begin(), current_list.end() });
			current_list.clear();
		}

		return lists;
	}

	std::vector<std::vector<Vector2>> ZigZagPath::generate_paths_outside_loops(const Vector2& min, const Vector2& max, const float radius, const float epsilon) const
	{
		const float row_width = radius - epsilon;
		int rows = static_cast<int>((max.y - min.y) / row_width) + 1;

		// calculate intersections of loops with rows
		auto intersections = calculate_intersection_of_loops_with_rows(rows, min, max, row_width);

		// make segments lists from left to right
		auto zigzag_segments = make_segment_list_outside_loops(intersections, min, max, row_width);

		// link segments and create paths
		return link_segments_and_create_paths(zigzag_segments, min, max, row_width);
	}

	std::vector<std::vector<Vector2>> ZigZagPath::generate_paths_excluding_segments(const Vector2& min, const Vector2& max, const float radius, const float epsilon, const SegmentCheck& check) const
	{
		const float row_width = radius - epsilon;
		int rows = static_cast<int>((max.y - min.y) / row_width) + 1;

		// calculate intersections of loops with rows
		auto intersections = calculate_intersection_of_loops_with_rows(rows, min, max, row_width);

		// make segments lists from left to right
		auto zigzag_segments = make_segment_list_with_check(intersections, check, min, max, row_width);

		// link segments and create paths
		return link_segments_and_create_paths(zigzag_segments, min, max, row_width);
	}
}
