#pragma once

#include <vector>
#include "algebra.h"
#include "drawable.h"

namespace ManualCAD
{
	class Rational20ParamSurface : public Drawable {
		size_t point_count = 0, contour_indices_count = 0;
		ElementBuffer ebo;
		bool ebo_set = false; // ebos will have constant value in the lifetime of a patch (except preview -> fn reset_ebos()), this flag is for setting them only once
	public:
		Vector4 contour_color = { 1.0f,1.0f,1.0f,1.0f };
		bool draw_patch = true;
		bool draw_contour = false;
		unsigned int divisions_x = 4;
		unsigned int divisions_y = 4;

		Rational20ParamSurface(const Matrix4x4& model) : Drawable(model), ebo() { ebo.init(); ebo.bind(); }
		Rational20ParamSurface() : Drawable(), ebo() { ebo.init(); ebo.bind(); }

		inline size_t get_point_count() const { return point_count; }
		inline size_t get_contour_indices_count() const { return contour_indices_count; }

		void dispose() override { Drawable::dispose(); ebo.dispose(); }
		void set_data(const std::vector<Vector3>& points);
		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;
	};
}