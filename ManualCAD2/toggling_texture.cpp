#include "toggling_texture.h"
#include "object.h"
#include <glad/glad.h>
#include <stack>

namespace ManualCAD
{
	class Image {
	public:
		std::vector<Vector4> pixels;
		int width, height;

		Image(int width, int height) : width(width), height(height), pixels(width* height) {}
		inline Vector4& pixel(int x, int y) { return pixels[x + y * width]; }
	};

	float pix_to_coord(const Range<float>& range, int pixel, int dim)
	{
		return pixel * (range.to - range.from) / dim + range.from;
	}

	void flood_fill(Image& image, int x, int y, const Vector4& color, const ParametricSurfaceObject& surf)
	{
		const auto old = image.pixel(x, y);
		if (color == old) return;

		const auto urange = surf.get_u_range(), vrange = surf.get_v_range();

		std::stack<std::pair<int, int>> stack;
		stack.push({ x, y });
		while (!stack.empty())
		{
			const auto coords = stack.top();
			stack.pop();
			image.pixel(coords.first, coords.second) = color;
			if (coords.first > 0 && image.pixel(coords.first - 1, coords.second) == old)
				stack.push({ coords.first - 1, coords.second });
			if (coords.first < image.width - 1 && image.pixel(coords.first + 1, coords.second) == old)
				stack.push({ coords.first + 1, coords.second });
			if (coords.second > 0 && image.pixel(coords.first, coords.second - 1) == old)
				stack.push({ coords.first, coords.second - 1 });
			if (coords.second < image.height - 1 && image.pixel(coords.first, coords.second + 1) == old)
				stack.push({ coords.first, coords.second + 1 });

			if (coords.first == 0 && surf.u_wraps_at_v(pix_to_coord(vrange, coords.second, image.height)) && image.pixel(image.width - 1, coords.second) == old)
				stack.push({ image.width - 1, coords.second });
			if (coords.first == image.width - 1 && surf.u_wraps_at_v(pix_to_coord(vrange, coords.second, image.height)) && image.pixel(0, coords.second) == old)
				stack.push({ 0, coords.second });

			if (coords.second == 0 && surf.v_wraps_at_u(pix_to_coord(urange, coords.first, image.width)) && image.pixel(coords.first, image.height - 1) == old)
				stack.push({ coords.first, image.height - 1 });
			if (coords.second == image.height - 1 && surf.v_wraps_at_u(pix_to_coord(urange, coords.first, image.width)) && image.pixel(coords.first, 0) == old)
				stack.push({ coords.first, 0 });
		}
	}

	void TogglingTexture::clear()
	{
		GLint old_viewport[4];
		glGetIntegerv(GL_VIEWPORT, old_viewport);
		fbo.bind();
		glViewport(0, 0, tex_width, tex_height);
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		fbo.unbind();
		glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);
	}

	void TogglingTexture::add_line(const Line2D& line)
	{
		/*GLint old_viewport[4];
		glGetIntegerv(GL_VIEWPORT, old_viewport);
		fbo.bind();
		glViewport(0, 0, tex_width, tex_height);
		line.render(renderer, tex_width, tex_height, thickness);
		fbo.unbind();
		glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);*/
		lines.push_back(&line);
		valid = false;
	}

	void TogglingTexture::remove_line(const Line2D& line)
	{
		lines.remove(&line);
		clear();
		valid = false;
	}

	void TogglingTexture::render(Renderer& renderer)
	{
		if (valid) return;
		// draw curves to texture
		GLint old_viewport[4];
		glGetIntegerv(GL_VIEWPORT, old_viewport);
		fbo.bind();
		glViewport(0, 0, tex_width, tex_height);
		//glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clearing is not necessary since we clear while creating texture or removing a line
		for (const auto* line : lines)
			line->render(renderer, tex_width, tex_height);
		fbo.unbind();
		glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);
		valid = true;
	}

	void TogglingTexture::toggle_region(int x, int y)
	{
		Image image(tex_width, tex_height);
		texture.bind();
		texture.get_image(tex_width, tex_height, image.pixels.data());
		Vector4 color = image.pixel(x, y), new_color;
		if (color.x == 1.0f) // border
			return;
		if (color.w > 0.0f)
			new_color = { 0.0f, 0.0f, 0.0f, 0.0f };
		else
			new_color = { 0.0f, 0.0f, 1.0f, 1.0f };
		flood_fill(image, x, y, new_color, surf);
		texture.set_image(tex_width, tex_height, image.pixels.data());
	}
}