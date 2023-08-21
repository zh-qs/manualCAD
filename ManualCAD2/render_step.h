#pragma once
#include "renderable.h"

namespace ManualCAD
{
	class Renderer;

	class RenderStep {
	public:
		virtual void do_render_step(Renderer& renderer, int width, int height) = 0;
	};

	class RenderObjectStep : public RenderStep {
		const Renderable& renderable;
		float thickness;
	public:
		RenderObjectStep(const Renderable& renderable, float thickness) : renderable(renderable), thickness(thickness) {}
		void do_render_step(Renderer& renderer, int width, int height) override;
	};

	class EnableDepthTestStep : public RenderStep {
	public:
		void do_render_step(Renderer& renderer, int width, int height) override;
	};

	class DisableDepthTestStep : public RenderStep {
	public:
		void do_render_step(Renderer& renderer, int width, int height) override;
	};

	class EnableDepthWriteStep : public RenderStep {
	public:
		void do_render_step(Renderer& renderer, int width, int height) override;
	};

	class DisableDepthWriteStep : public RenderStep {
	public:
		void do_render_step(Renderer& renderer, int width, int height) override;
	};
}