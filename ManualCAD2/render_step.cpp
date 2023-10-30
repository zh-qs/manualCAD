#include "render_step.h"
#include "renderer.h"
#include <glad/glad.h>

namespace ManualCAD
{
	void RenderObjectStep::do_render_step(Renderer& renderer, int width, int height)
	{
		renderable.render(renderer, width, height, thickness);
		if constexpr (ApplicationSettings::DEBUG)
			Renderable::assert_unbound();
	}

	void EnableDepthTestStep::do_render_step(Renderer& renderer, int width, int height)
	{
		glDepthFunc(GL_LESS);
	}

	void DisableDepthTestStep::do_render_step(Renderer& renderer, int width, int height)
	{
		glDepthFunc(GL_ALWAYS);
	}

	void EnableDepthWriteStep::do_render_step(Renderer& renderer, int width, int height)
	{
		glDepthMask(GL_TRUE);
	}

	void DisableDepthWriteStep::do_render_step(Renderer& renderer, int width, int height)
	{
		glDepthMask(GL_FALSE);
	}
}