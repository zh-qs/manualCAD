#pragma once

#include "algebra.h"
#include "application_settings.h"

namespace ManualCAD
{
	struct Segment {
		Vector2 start;
		Vector2 end;
	};

	class Planimetrics {
	public:
		inline static bool intersect(const Segment& seg1, const Segment& seg2, Vector2& out_point) {
			return intersect(seg1.start, seg1.end, seg2.start, seg2.end, out_point);
		}
		inline static bool intersect(const Vector2& seg1start, const Vector2& seg1end, const Vector2& seg2start, const Vector2& seg2end, Vector2& out_point) {
			const float d1 = cross_sign(seg2end - seg2start, seg1start - seg2start),
				d2 = cross_sign(seg2end - seg2start, seg1end - seg2start),
				d3 = cross_sign(seg1end - seg1start, seg2start - seg1start),
				d4 = cross_sign(seg1end - seg1start, seg2end - seg1start);
			const float d12 = d1 * d2,
				d34 = d3 * d4;
			if (d12 > 0 || d34 > 0)
				return false;
			if (d12 < 0 || d34 < 0)
			{
				const float d = cross_sign(seg1end - seg1start, seg2start - seg2end);
				const float a = -d1 / d;
				if constexpr (ApplicationSettings::DEBUG)
				{
					assert(d != 0);
					assert(a >= 0);
					assert(a <= 1);
				}
				out_point = (1 - a) * seg1start + a * seg1end;
				return true;
			}
			// from now we consider parallel segments (very rare)
			// TODO add output attributes to handle parallel segments intersections correctly
			if (seg1start == seg2start)
			{
				out_point = seg1start;
				return true;
			}
			if (seg1start == seg2end)
			{
				out_point = seg1start;
				return true;
			}
			if (seg1end == seg2start)
			{
				out_point = seg1end;
				return true;
			}
			if (seg1end == seg2end)
			{
				out_point = seg1end;
				return true;
			}
			if (std::max(seg1start.x, seg1end.x) < std::min(seg2start.x, seg2end.x)
				|| std::max(seg2start.x, seg2end.x) < std::min(seg1start.x, seg1end.x))
				return false;
			if (std::max(seg1start.y, seg1end.y) < std::min(seg2start.y, seg2end.y)
				|| std::max(seg2start.y, seg2end.y) < std::min(seg1start.y, seg1end.y))
				return false;
			// from now we consider overlapping parallel segments (very very rare)
			// TODO add output attributes to handle parallel overlapping segments intersections correctly
			const auto max1 = max(seg1start, seg1end), max2 = max(seg2start, seg2end);
			out_point = 0.5f * (max1 + max2);
			return true;
		}
	};
}