#pragma once
#include "renderable.h"

namespace ManualCAD
{
	class SimpleRect : public Renderable {
	public:
		bool visible = false;
		Vector4 color = { 1.0f,0.647f,0.1f,1.0f };
		float scale_x = 0.0f, scale_y = 0.0f;
		Vector2 position;

		SimpleRect();

		inline size_t get_quad_vertices_count() const { return 6; }
		inline size_t get_line_strip_vertices_count() const { return 5; }

		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;
	};
}