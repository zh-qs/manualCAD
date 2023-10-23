#pragma once
#include "algebra.h"
#include "vertex_array.h"
#include "buffer.h"
#include "transformation.h"

namespace ManualCAD
{
	class Renderer;

	class Renderable {
	public:
		bool visible = true;
		Vector4 color = { 0.0f,0.0f,0.0f,0.0f };

		virtual void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const = 0;
		virtual void set_transformation(const Transformation& transformation) = 0;
		virtual void set_combined_transformation(const Transformation& transformation, const Transformation& combine_transformation, const Vector3& center) = 0;
		virtual void dispose() {}
	};
}