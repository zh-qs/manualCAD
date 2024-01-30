#include "height_map_renderer.h"

namespace ManualCAD
{
	HeightMapRenderer::HeightMapRenderer(const std::list<const ParametricSurfaceObject*>& surfaces, const Box& box, float offset) : surfaces(surfaces), fbo(), texture(), offset(offset)
	{
		fbo.init();
		fbo.bind();
		texture.init();
		texture.bind();
		texture.configure();
		texture.set_size(TEX_DIM, TEX_DIM);
		fbo.unbind();

		renderer.default_shader_set = ShaderSet::Type::HeightMap;
		renderer.get_camera().set_rotation(HALF_PI, HALF_PI, 0.0f);
		// we set scale such that we transform box to [-1,1]x[0,1]x[-1,1] (order of axes after rotation: X, Z, -Y)
		renderer.get_camera().zoom(1.0f / (box.x_max - box.x_min), 1.0f / (box.z_max - box.z_min), 1.0f / (box.y_max - box.y_min));//scale * (height - bottom_height));
		Vector3 box_bottom_center = box.center();//{ center.x, center.y + scale * bottom_height, center.z };
		box_bottom_center.y = box.y_min;
		renderer.get_camera().look_at(box_bottom_center);
		renderer.get_camera().near = 3.0f;
		renderer.get_camera().far = 5.0f; // TODO values taken from camera object, not copy-pasted!
		renderer.get_camera().projection_type = Camera::ProjectionType::Ortographic;
		renderer.polygon_mode = GL_FILL;
	}

	HeightMapRenderer::~HeightMapRenderer()
	{
		//texture.dispose();
		//fbo.dispose();
		// we don't dispose renderer because we don't initialize it
	}

	HeightMap HeightMapRenderer::render_height_map(const Vector3& size)
	{
		HeightMap map(TEX_DIM, TEX_DIM, size);

		GLint old_viewport[4];
		glGetIntegerv(GL_VIEWPORT, old_viewport);
		fbo.bind();
		glViewport(0, 0, TEX_DIM, TEX_DIM);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clearing is not necessary since we clear while creating texture or removing a line

		renderer.set_additional_uniform_variable("is_idx_map", 0.0f);
		renderer.set_additional_uniform_variable("au_offset", offset);
		for (const auto* surf : surfaces)
			surf->get_const_renderable().render(renderer, TEX_DIM, TEX_DIM);

		fbo.unbind();
		glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);

		texture.bind();
		texture.get_image(map.data());

		return map;
	}
	HeightMap HeightMapRenderer::render_index_map(const Vector3& size)
	{
		Vector3 map_size = { size.x, surfaces.size(), size.z };
		HeightMap map(TEX_DIM, TEX_DIM, map_size);

		GLint old_viewport[4];
		glGetIntegerv(GL_VIEWPORT, old_viewport);
		fbo.bind();
		glViewport(0, 0, TEX_DIM, TEX_DIM);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clearing is not necessary since we clear while creating texture or removing a line

		int i = 1;
		renderer.set_additional_uniform_variable("is_idx_map", 1.0f);
		renderer.set_additional_uniform_variable("au_surf_count", surfaces.size());
		renderer.set_additional_uniform_variable("au_offset", offset);
		for (const auto* surf : surfaces)
		{
			renderer.set_additional_uniform_variable("au_surf_idx", i);
			surf->get_const_renderable().render(renderer, TEX_DIM, TEX_DIM);
			++i;
		}

		// render again with partial and zero offset to fill gaps in C0 fragments
		// TODO: there IS a better way to do it! (maybe exploit some empty patches in shaders)
		i = 1;
		renderer.set_additional_uniform_variable("au_offset", 0.8f * offset);
		for (const auto* surf : surfaces)
		{
			renderer.set_additional_uniform_variable("au_surf_idx", i);
			surf->get_const_renderable().render(renderer, TEX_DIM, TEX_DIM);
			++i;
		}

		i = 1;
		renderer.set_additional_uniform_variable("au_offset", 0.35f * offset);
		for (const auto* surf : surfaces)
		{
			renderer.set_additional_uniform_variable("au_surf_idx", i);
			surf->get_const_renderable().render(renderer, TEX_DIM, TEX_DIM);
			++i;
		}

		i = 1;
		renderer.set_additional_uniform_variable("au_offset", 0.0f);
		for (const auto* surf : surfaces)
		{
			renderer.set_additional_uniform_variable("au_surf_idx", i);
			surf->get_const_renderable().render(renderer, TEX_DIM, TEX_DIM);
			++i;
		}

		fbo.unbind();
		glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);

		texture.bind();
		texture.get_image(map.data());

		return map;
	}
}
