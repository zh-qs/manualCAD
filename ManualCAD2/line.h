#pragma once

#include <vector>
#include "algebra.h"
#include "drawable.h"

namespace ManualCAD
{
	class Line : public Drawable {

		size_t point_count = 0;
	public:
		bool looped = false;

		Line(const Matrix4x4& model) : Drawable(model) {}
		Line() : Drawable() {}
		
		inline size_t get_point_count() const { return point_count; }

		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;

		void set_data(const std::vector<Vector3>& points);
	};

	class Line2D : public Drawable {
		size_t point_count = 0;
		Range<float> urange, vrange;

	public:
		bool looped = false;

		Line2D(const Range<float>& urange, const Range<float>& vrange) : Drawable(2), urange(urange), vrange(vrange) {}

		inline size_t get_point_count() const { return point_count; }
		inline Range<float> get_u_range() const { return urange; }
		inline Range<float> get_v_range() const { return vrange; }

		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;

		void set_data(const std::vector<Vector2>& points);
	};
}