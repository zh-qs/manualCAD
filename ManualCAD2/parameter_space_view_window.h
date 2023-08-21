#pragma once

#include "window.h"
#include "object.h"
#include "texture.h"
#include "frame_buffer.h"

namespace ManualCAD
{
	class ParameterSpaceViewWindow : public Window
	{
		ParametricSurface* surface = nullptr;
		Renderer& renderer;
		//FrameBuffer fbo;
		//Texture texture;
	public:
		ParameterSpaceViewWindow(Renderer& renderer) : Window(), renderer(renderer)
		{
			visible = false;
			/*fbo.init();
			fbo.bind();
			texture.init();
			texture.bind();
			texture.configure();
			fbo.unbind();*/
		}
		void build() override;
		void show_surface(ParametricSurface& surf)
		{
			surface = &surf;
			visible = true;
		}
		void dispose() override {
			//texture.dispose();
			//fbo.dispose();
		}
	};
}