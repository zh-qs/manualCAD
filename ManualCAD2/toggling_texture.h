#pragma once

#include "algebra.h"
#include "frame_buffer.h"
#include "texture.h"
#include <vector>
#include <list>
#include "line.h"
#include "application_settings.h"

namespace ManualCAD
{
	class ParametricSurface;
	class TogglingTexture
	{
		FrameBuffer fbo;
		Texture texture;
		int tex_width = 0, tex_height = 0;
		bool valid = false;
		std::list<const Line2D*> lines;
		const ParametricSurface& surf;
	public:
		TogglingTexture(int width, int height, const ParametricSurface& surf) : tex_width(width), tex_height(height), surf(surf)
		{
			fbo.init();
			fbo.bind();
			texture.init();
			texture.bind();
			texture.configure(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			texture.set_size(width, height);
			fbo.unbind();

			clear();
		}

		void clear();
		void add_line(const Line2D& line);
		void remove_line(const Line2D& line);
		void render(Renderer& renderer);
		void toggle_region(int x, int y);
		int get_width() const { return tex_width; }
		int get_height() const { return tex_height; }
		const Texture& get_texture() const { return texture; }
	};
}