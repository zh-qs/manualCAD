#include "simple_rect.h"
#include "renderer.h"

namespace ManualCAD
{
	static const float vertices[] = {
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f
	};

	SimpleRect::SimpleRect() : Renderable() {
		vbo.bind();
		vbo.set_static_data(vertices, sizeof(vertices));
		vao.unbind();
	}

	void SimpleRect::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_simple_rect(*this, width, height, thickness);
	}
}