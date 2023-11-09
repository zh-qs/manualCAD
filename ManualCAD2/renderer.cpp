#include "renderer.h"
#include "exception.h"
#include "settings_window.h"
#include "application_settings.h"
#include "render_step.h"
#include "shader_library.h"
#include <glad/glad.h>
#include <algorithm>
#include <GLFW/glfw3.h>

namespace ManualCAD
{
	static const GLfloat screen_vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};

	static const GLfloat screen_uvs[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	Renderer::Renderer()
	{
	}

	void Renderer::init() {
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			THROW_EXCEPTION;

		// stuff for stereoscopy

		quad_vao.init();
		quad_screen_vbo.init();
		quad_uvs_vbo.init();
		quad_vao.bind();
		quad_screen_vbo.bind();
		quad_screen_vbo.set_static_data(screen_vertices, sizeof(screen_vertices));
		quad_screen_vbo.attrib_buffer(0, 3, GL_FLOAT);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		//glEnableVertexAttribArray(0);
		quad_uvs_vbo.bind();
		quad_uvs_vbo.set_static_data(screen_uvs, sizeof(screen_uvs));
		quad_uvs_vbo.attrib_buffer(1, 2, GL_FLOAT);
		//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		//glEnableVertexAttribArray(1);
		quad_vao.unbind();

		quad_shader.init("raycasting_vertex_shader.glsl", "stereoscopy_quad_fragment_shader.glsl");

		for (int i = 0; i < STEREOSCOPY_POVS; ++i)
		{
			fbo[i].init();
			fbo[i].bind();
			texture[i].init();
			texture[i].bind();
			texture[i].configure();
			fbo[i].unbind();
		}
		uq_tex_location[0] = quad_shader.get_uniform_location("u_tex0");
		uq_tex_location[1] = quad_shader.get_uniform_location("u_tex1");
		uq_lcolor_location = quad_shader.get_uniform_location("u_lcolor");
		uq_saturation_location = quad_shader.get_uniform_location("u_saturation");

		// replacement texture for non-trimmed surfaces
		white_texture.init();
		white_texture.bind();
		const Vector4 white{ 1.0f,1.0f,1.0f,1.0f };
		white_texture.set_image(1, 1, &white);
		white_texture.unbind();

		glPointSize(ApplicationSettings::RENDER_POINT_SIZE);
		glEnable(GL_POINT_SPRITE);
		//glEnable(GL_POINT_SMOOTH); <- this draws circled points with line_shader, but i use other shader in case i want to draw more complicated patterns

		glEnable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void Renderer::add(const Renderable& renderable, float thickness)
	{
		render_steps.push_back(std::make_unique<RenderObjectStep>(renderable, thickness));
	}

	void Renderer::render_all(int width, int height)
	{
		if (stereoscopy_settings.enabled)
		{
			float eye_d[2] = { -stereoscopy_settings.interocular_distance, stereoscopy_settings.interocular_distance };
			for (int i = 0; i < STEREOSCOPY_POVS; ++i)
			{
				// bind framebuffer and set render viewport
				fbo[i].bind();
				texture[i].bind();
				texture[i].set_size(width, height);
				glViewport(0, 0, width, height);

				// clear texture
				glClearColor(0, 0, 0, 0);
				glClear(GL_COLOR_BUFFER_BIT);

				// render steps
				camera.eye_distance = eye_d[i];
				camera.focus_plane = stereoscopy_settings.focus_plane;
				for (const auto& step : render_steps)
				{
					step->do_render_step(*this, width, height);
				}

				// bind screen framebuffer
				fbo[i].unbind();
				//glViewport(0, 0, width, height);
			}
			camera.eye_distance = 0;

			// use shader and send uniform data (texture sampler)
			quad_shader.use();
			glActiveTexture(GL_TEXTURE0); // TODO for loop
			texture[0].bind();
			glActiveTexture(GL_TEXTURE1);
			texture[1].bind();
			glUniform1i(uq_tex_location[0], 0);
			glUniform1i(uq_tex_location[1], 1);
			glUniform4f(uq_lcolor_location, stereoscopy_settings.left_eye_color.x, stereoscopy_settings.left_eye_color.y, stereoscopy_settings.left_eye_color.z, stereoscopy_settings.left_eye_color.w);
			glUniform1f(uq_saturation_location, stereoscopy_settings.view_saturation);

			// render screen
			quad_vao.bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
			quad_vao.unbind();
		}
		else
		{
			for (const auto& step : render_steps)
			{
				step->do_render_step(*this, width, height);
			}
		}
		render_steps.clear();
	}

	void Renderer::render_wireframe(const WireframeMesh& mesh, const Vector4& color, int width, int height, float thickness)
	{
		/*vbo.bind();
		vbo.set_dynamic_data(mesh.points_data(), mesh.points_size_bytes());
		ebo.bind();
		ebo.set_dynamic_data(mesh.line_indices_data(), mesh.line_indices_size_bytes());*/
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * mesh.get_model_matrix();

		set.line_shader.use();
		glUniformMatrix4fv(lib.ul_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
		glUniform4f(lib.ul_color_location, color.x, color.y, color.z, color.w);

		//mesh.vao.bind();
		mesh.bind_to_render();

		glLineWidth(thickness);
		glDrawElements(GL_LINES, 2 * mesh.get_line_count(), GL_UNSIGNED_INT, NULL);

		mesh.unbind_from_render();
	}

	void Renderer::render_textured_wireframe(const TexturedWireframeMesh& mesh, const Vector4& color, int width, int height, float thickness)
	{
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * mesh.get_model_matrix();

		set.trim_line_shader.use();
		glActiveTexture(GL_TEXTURE0);
		mesh.get_texture().bind();
		glUniform1i(lib.utl_trim_texture_location, 0);
		glUniformMatrix4fv(lib.utl_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
		glUniform4f(lib.utl_color_location, color.x, color.y, color.z, color.w);

		//mesh.vao.bind();
		mesh.bind_to_render();

		glLineWidth(thickness);
		glDrawElements(GL_LINES, 2 * mesh.get_line_count(), GL_UNSIGNED_INT, NULL);

		mesh.unbind_from_render();
	}

	void Renderer::render_points(const PointSet& mesh, const Vector4& color, int width, int height, float thickness)
	{
		/*vbo.bind();
		vbo.set_dynamic_data(mesh.points_data(), mesh.points_size_bytes());*/
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * mesh.get_model_matrix();

		set.point_shader.use();
		glUniformMatrix4fv(lib.up_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
		glUniform4f(lib.up_color_location, color.x, color.y, color.z, color.w);

		//vao.bind();
		mesh.bind_to_render();

		glPointSize(thickness * ApplicationSettings::RENDER_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, mesh.get_point_count());

		mesh.unbind_from_render();
	}

	void Renderer::render_curve_and_polyline(const CurveWithPolyline& curve, const Vector4& color, int width, int height, float thickness)
	{
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * curve.get_model_matrix();

		if (curve.draw_polyline) {
			set.line_shader.use();
			glUniformMatrix4fv(lib.ul_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(lib.ul_color_location, curve.polyline_color.x, curve.polyline_color.y, curve.polyline_color.z, curve.polyline_color.w);

			curve.bind_to_render();

			glLineWidth(thickness);
			glDrawArrays(GL_LINE_STRIP, 0, curve.get_point_count());

			curve.unbind_from_render();
		}
		if (curve.draw_curve) {
			set.bezier_shader.use();
			glUniformMatrix4fv(lib.ub_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(lib.ub_color_location, color.x, color.y, color.z, color.w);
			glUniform1f(lib.ub_width_location, width);
			glUniform1f(lib.ub_height_location, height);

			curve.bind_to_render();

			glLineWidth(thickness);
			glDrawElements(GL_LINES_ADJACENCY, curve.get_line_indices_count(), GL_UNSIGNED_INT, NULL);

			curve.unbind_from_render();
		}
	}

	void Renderer::render_axes_cursor(const AxesCursor& ac, int width, int height, float thickness)
	{
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		auto invs = 1.0f / camera.get_scale();
		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * ac.get_model_matrix() * Matrix4x4::scale(invs, invs, invs);

		set.cursor_shader.use();
		glUniformMatrix4fv(lib.uc_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);

		//mesh.vao.bind();
		ac.bind_to_render();

		glLineWidth(thickness);
		glDrawArrays(GL_LINES, 0, ac.get_vertex_count());

		ac.unbind_from_render();
	}

	void Renderer::render_surface_and_bezier_contour(const SurfaceWithBezierContour& surf, const Vector4& color, int width, int height, float thickness)
	{
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix();// * surf.get_model_matrix(); -> patch is not transformable

		if (surf.draw_contour)
		{
			set.line_shader.use();
			glUniformMatrix4fv(lib.ul_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(lib.ul_color_location, surf.contour_color.x, surf.contour_color.y, surf.contour_color.z, surf.contour_color.w);

			surf.bind_contour_to_render();

			glLineWidth(thickness);
			glDrawElements(GL_LINES, surf.get_contour_indices_count(), GL_UNSIGNED_INT, NULL);

			surf.unbind_from_render();
		}
		if (surf.draw_patch)
		{
			set.bezier_patch_shader.use();
			glActiveTexture(GL_TEXTURE0);
			if (surf.has_texture())
				surf.get_texture().bind();
			else
				white_texture.bind();
			glUniform1i(lib.upa_trim_texture_location, 0);
			glPatchParameteri(GL_PATCH_VERTICES, 16);
			glUniformMatrix4fv(lib.upa_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(lib.upa_color_location, color.x, color.y, color.z, color.w);
			glUniform1i(lib.upa_divisions_x_location, surf.divisions_x);
			glUniform1i(lib.upa_divisions_y_location, surf.divisions_y);
			glUniform1f(lib.upa_patches_x_location, surf.get_patches_x_count());
			glUniform1f(lib.upa_patches_y_location, surf.get_patches_y_count());

			surf.bind_patch_to_render();

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(thickness);
			glDrawElements(GL_PATCHES, surf.get_patch_indices_count(), GL_UNSIGNED_INT, NULL);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			surf.unbind_from_render();
		}
	}

	void Renderer::render_surface_and_de_boor_contour(const SurfaceWithDeBoorContour& surf, const Vector4& color, int width, int height, float thickness)
	{
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix();// * surf.get_model_matrix(); -> patch is not transformable

		if (surf.draw_contour)
		{
			set.line_shader.use();
			glUniformMatrix4fv(lib.ul_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(lib.ul_color_location, surf.contour_color.x, surf.contour_color.y, surf.contour_color.z, surf.contour_color.w);

			surf.bind_contour_to_render();

			glLineWidth(thickness);
			glDrawElements(GL_LINES, surf.get_contour_indices_count(), GL_UNSIGNED_INT, NULL);

			surf.unbind_from_render();
		}
		if (surf.draw_patch)
		{
			set.de_boor_patch_shader.use();
			glActiveTexture(GL_TEXTURE0);
			if (surf.has_texture())
				surf.get_texture().bind();
			else
				white_texture.bind();
			glUniform1i(lib.upb_trim_texture_location, 0);
			glPatchParameteri(GL_PATCH_VERTICES, 16);
			glUniformMatrix4fv(lib.upb_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(lib.upb_color_location, color.x, color.y, color.z, color.w);
			glUniform1i(lib.upb_divisions_x_location, surf.divisions_x);
			glUniform1i(lib.upb_divisions_y_location, surf.divisions_y);
			glUniform1f(lib.upb_patches_x_location, surf.get_patches_x_count());
			glUniform1f(lib.upb_patches_y_location, surf.get_patches_y_count());

			surf.bind_patch_to_render();

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(thickness);
			glDrawElements(GL_PATCHES, surf.get_patch_indices_count(), GL_UNSIGNED_INT, NULL);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			surf.unbind_from_render();
		}
	}

	void Renderer::render_rational_20_param_surface(const Rational20ParamSurface& surf, const Vector4& color, int width, int height, float thickness)
	{
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix();// * surf.get_model_matrix(); -> patch is not transformable

		if (surf.draw_contour)
		{
			set.line_shader.use();
			glUniformMatrix4fv(lib.ul_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(lib.ul_color_location, surf.contour_color.x, surf.contour_color.y, surf.contour_color.z, surf.contour_color.w);

			surf.bind_to_render();

			glLineWidth(thickness);
			glDrawElements(GL_LINES, surf.get_contour_indices_count(), GL_UNSIGNED_INT, NULL);

			surf.unbind_from_render();
		}
		if (surf.draw_patch)
		{
			set.rational_20_param_patch_shader.use();
			glPatchParameteri(GL_PATCH_VERTICES, 20);
			glUniformMatrix4fv(lib.upr_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(lib.upr_color_location, color.x, color.y, color.z, color.w);
			glUniform1i(lib.upr_divisions_x_location, surf.divisions_x);
			glUniform1i(lib.upr_divisions_y_location, surf.divisions_y);

			surf.bind_to_render();

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(thickness);
			glDrawArrays(GL_PATCHES, 0, surf.get_point_count());
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			surf.unbind_from_render();
		}
		
		//// debug
		//m = camera.get_projection_matrix(width, height) * camera.get_view_matrix();

		//point_shader.use();
		//glUniformMatrix4fv(up_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
		//glUniform4f(up_color_location, 1.0f, 0.0f, 0.0f, color.w);

		////vao.bind();
		//surf.bind_to_render();

		//glPointSize(thickness * ApplicationSettings::RENDER_POINT_SIZE);
		//glDrawArrays(GL_POINTS, 0, surf.get_point_count());
	}

	void Renderer::render_simple_rect(const SimpleRect& rect, int width, int height, float thickness)
	{
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		set.simple_shader.use();

		glUniform2f(lib.us_scale_location, rect.scale_x, rect.scale_y);
		glUniform2f(lib.us_pos_location, rect.position.x, rect.position.y);

		glUniform4f(lib.us_color_location, rect.color.x, rect.color.y, rect.color.z, rect.color.w * 0.5f);
		rect.bind_to_render();
		glDrawArrays(GL_TRIANGLES, 0, rect.get_quad_vertices_count());

		glUniform4f(lib.us_color_location, rect.color.x, rect.color.y, rect.color.z, rect.color.w);
		rect.bind_to_render();
		glLineWidth(thickness);
		glDrawArrays(GL_LINE_STRIP, 0, rect.get_line_strip_vertices_count());

		rect.unbind_from_render();
	}

	void Renderer::render_line(const Line& line, const Vector4& color, int width, int height, float thickness)
	{
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * line.get_model_matrix();

		set.line_shader.use();

		glUniformMatrix4fv(lib.ul_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
		glUniform4f(lib.ul_color_location, color.x, color.y, color.z, color.w);

		line.bind_to_render();

		glLineWidth(thickness);
		glDrawArrays(line.looped ? GL_LINE_LOOP : GL_LINE_STRIP, 0, line.get_point_count());

		line.unbind_from_render();
	}

	void Renderer::render_line_2d(const Line2D& line, const Vector4& color, int width, int height, float thickness)
	{
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		set.two_dim_shader.use();

		auto urange = line.get_u_range(), vrange = line.get_v_range();

		glUniform2f(lib.u2d_urange_location, urange.from, urange.to);
		glUniform2f(lib.u2d_vrange_location, vrange.from, vrange.to);
		glUniform4f(lib.u2d_color_location, color.x, color.y, color.z, color.w);

		line.bind_to_render();

		glLineWidth(thickness);
		glDrawArrays(line.looped ? GL_LINE_LOOP : GL_LINE_STRIP, 0, line.get_point_count());

		line.unbind_from_render();
	}

	void Renderer::render_workpiece_renderable(const WorkpieceRenderable& workpiece_renderable, const Vector4& color, int width, int height, float thickness)
	{
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		auto pv = camera.get_projection_matrix(width, height) * camera.get_view_matrix();

		set.workpiece_shader.use();
		glActiveTexture(GL_TEXTURE0);
		workpiece_renderable.get_texture().bind();
		glUniform1i(lib.uw_height_map_location, 0);
		glUniformMatrix4fv(lib.uw_pv_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(pv).elem);
		glUniformMatrix4fv(lib.uw_m_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(workpiece_renderable.get_model_matrix()).elem);
		glUniform3f(lib.uw_size_location, workpiece_renderable.parent_size.x, workpiece_renderable.parent_size.y, workpiece_renderable.parent_size.z);
		glUniform4f(lib.uw_color_location, color.x, color.y, color.z, color.w);
		auto offset = workpiece_renderable.get_uv_offset();
		glUniform2f(lib.uw_uv_offset_location, offset.x, offset.y);

		//mesh.vao.bind();
		workpiece_renderable.bind_to_render();

		//glLineWidth(thickness);
		glDrawElements(GL_TRIANGLES, workpiece_renderable.get_indices_count(), GL_UNSIGNED_INT, NULL);

		workpiece_renderable.unbind_from_render();
	}

	void Renderer::render_triangle_mesh(const TriangleMesh& mesh, const Vector4& color, int width, int height, float thickness)
	{
		const auto& lib = ShaderLibrary::get();
		const auto& set = lib.get_shaders(0); // default

		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * mesh.get_model_matrix();

		set.triangle_shader.use();
		glUniformMatrix4fv(lib.ut_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
		glUniformMatrix4fv(lib.ut_m_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(mesh.get_model_matrix()).elem);
		glUniform4f(lib.ut_color_location, color.x, color.y, color.z, color.w);

		mesh.bind_to_render();

		glLineWidth(thickness);
		glDrawElements(GL_TRIANGLES, 3 * mesh.get_triangle_count(), GL_UNSIGNED_INT, NULL);

		mesh.unbind_from_render();
	}

	void Renderer::enable_depth_testing()
	{
		render_steps.push_back(std::make_unique<EnableDepthTestStep>());
	}

	void Renderer::disable_depth_testing()
	{
		render_steps.push_back(std::make_unique<DisableDepthTestStep>());
	}

	void Renderer::enable_depth_buffer_write()
	{
		render_steps.push_back(std::make_unique<EnableDepthWriteStep>());
	}

	void Renderer::disable_depth_buffer_write()
	{
		render_steps.push_back(std::make_unique<DisableDepthWriteStep>());
	}

	void Renderer::dispose()
	{
		quad_vao.dispose();
		quad_screen_vbo.dispose();
		quad_uvs_vbo.dispose();
		for (int i = 0; i < STEREOSCOPY_POVS; ++i)
		{
			fbo[i].dispose();
			texture[i].dispose();
		}
	}
}
