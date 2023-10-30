#include "renderer.h"
#include "exception.h"
#include "settings_window.h"
#include "application_settings.h"
#include "render_step.h"
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

	Renderer::Renderer() : line_shader(), point_shader(), cursor_shader(), bezier_shader(), bezier_patch_shader(), de_boor_patch_shader()
	{
	}

	void Renderer::init() {
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			THROW_EXCEPTION;

		line_shader.init("line_vertex_shader.glsl", "line_fragment_shader.glsl");
		trim_line_shader.init("line_tex_vertex_shader.glsl", "line_trim_fragment_shader.glsl");
		point_shader.init("line_vertex_shader.glsl", "point_fragment_shader.glsl");
		cursor_shader.init("cursor_vertex_shader.glsl", "cursor_fragment_shader.glsl");
		bezier_shader.init("line_vertex_shader.glsl", "bezier_curve_geometry_shader.glsl", "line_fragment_shader.glsl");
		bezier_patch_shader.init("dummy_vertex_shader.glsl", "patch_tess_control_shader.glsl", "bezier_patch_tess_eval_shader.glsl", "patch_remove_diagonal_geometry_shader.glsl", "line_trim_fragment_shader.glsl");
		de_boor_patch_shader.init("dummy_vertex_shader.glsl", "patch_tess_control_shader.glsl", "de_boor_patch_tess_eval_shader.glsl", "patch_remove_diagonal_geometry_shader.glsl", "line_trim_fragment_shader.glsl");
		rational_20_param_patch_shader.init("dummy_vertex_shader.glsl", "rational_20_param_patch_tess_control_shader.glsl", "rational_20_param_patch_tess_eval_shader.glsl", "patch_remove_diagonal_geometry_shader.glsl", "line_fragment_shader.glsl");
		simple_shader.init("simple_rect_vertex_shader.glsl", "line_fragment_shader.glsl");
		two_dim_shader.init("2d_vertex_shader.glsl", "wrap_around_parameters_geometry_shader.glsl", "line_fragment_shader.glsl");
		//workpiece_shader.init("workpiece_vertex_shader.glsl", "phong_fragment_shader.glsl");
		workpiece_shader.init("workpiece_vertex_shader_g.glsl", "workpiece_geometry_shader.glsl", "phong_fragment_shader.glsl");
		triangle_shader.init("triangle_vertex_shader.glsl", "phong_fragment_shader.glsl");

		ul_pvm_location = line_shader.get_uniform_location("u_pvm");
		ul_color_location = line_shader.get_uniform_location("u_color");

		utl_pvm_location = trim_line_shader.get_uniform_location("u_pvm");
		utl_color_location = trim_line_shader.get_uniform_location("u_color");
		utl_trim_texture_location = trim_line_shader.get_uniform_location("u_trim_texture");

		up_pvm_location = point_shader.get_uniform_location("u_pvm");
		up_color_location = point_shader.get_uniform_location("u_color");

		uc_pvm_location = cursor_shader.get_uniform_location("u_pvm");

		ub_pvm_location = bezier_shader.get_uniform_location("u_pvm");
		ub_color_location = bezier_shader.get_uniform_location("u_color");
		ub_width_location = bezier_shader.get_uniform_location("u_width");
		ub_height_location = bezier_shader.get_uniform_location("u_height");

		upa_pvm_location = bezier_patch_shader.get_uniform_location("u_pvm");
		upa_color_location = bezier_patch_shader.get_uniform_location("u_color");
		upa_divisions_x_location = bezier_patch_shader.get_uniform_location("u_divisions_x");
		upa_divisions_y_location = bezier_patch_shader.get_uniform_location("u_divisions_y");
		upa_patches_x_location = bezier_patch_shader.get_uniform_location("u_patches_x");
		upa_patches_y_location = bezier_patch_shader.get_uniform_location("u_patches_y");
		upa_trim_texture_location = bezier_patch_shader.get_uniform_location("u_trim_texture");

		upb_pvm_location = de_boor_patch_shader.get_uniform_location("u_pvm");
		upb_color_location = de_boor_patch_shader.get_uniform_location("u_color");
		upb_divisions_x_location = de_boor_patch_shader.get_uniform_location("u_divisions_x");
		upb_divisions_y_location = de_boor_patch_shader.get_uniform_location("u_divisions_y");
		upb_patches_x_location = de_boor_patch_shader.get_uniform_location("u_patches_x");
		upb_patches_y_location = de_boor_patch_shader.get_uniform_location("u_patches_y");
		upb_trim_texture_location = de_boor_patch_shader.get_uniform_location("u_trim_texture");

		upr_pvm_location = rational_20_param_patch_shader.get_uniform_location("u_pvm");
		upr_color_location = rational_20_param_patch_shader.get_uniform_location("u_color");
		upr_divisions_x_location = rational_20_param_patch_shader.get_uniform_location("u_divisions_x");
		upr_divisions_y_location = rational_20_param_patch_shader.get_uniform_location("u_divisions_y");

		us_color_location = simple_shader.get_uniform_location("u_color");
		us_scale_location = simple_shader.get_uniform_location("u_scale");
		us_pos_location = simple_shader.get_uniform_location("u_pos");

		u2d_color_location = two_dim_shader.get_uniform_location("u_color");
		u2d_urange_location = two_dim_shader.get_uniform_location("u_urange");
		u2d_vrange_location = two_dim_shader.get_uniform_location("u_vrange");

		uw_pv_location = workpiece_shader.get_uniform_location("u_pv");
		uw_m_location = workpiece_shader.get_uniform_location("u_m");
		uw_size_location = workpiece_shader.get_uniform_location("u_size");
		uw_height_map_location = workpiece_shader.get_uniform_location("u_height_map");
		uw_color_location = workpiece_shader.get_uniform_location("u_color");
		uw_uv_offset_location = workpiece_shader.get_uniform_location("u_uv_offset");

		ut_pvm_location = triangle_shader.get_uniform_location("u_pvm");
		ut_m_location = triangle_shader.get_uniform_location("u_m");
		ut_color_location = triangle_shader.get_uniform_location("u_color");

		// stuff for stereoscopy

		quad_vao.init();
		quad_screen_vbo.init();
		quad_uvs_vbo.init();
		quad_vao.bind();
		quad_screen_vbo.bind();
		quad_screen_vbo.set_static_data(screen_vertices, sizeof(screen_vertices));
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
		quad_uvs_vbo.bind();
		quad_uvs_vbo.set_static_data(screen_uvs, sizeof(screen_uvs));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(1);

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

		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * mesh.get_model_matrix();

		line_shader.use();
		glUniformMatrix4fv(ul_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
		glUniform4f(ul_color_location, color.x, color.y, color.z, color.w);

		//mesh.vao.bind();
		mesh.bind_to_render();

		glLineWidth(thickness);
		glDrawElements(GL_LINES, 2 * mesh.get_line_count(), GL_UNSIGNED_INT, NULL);
	}

	void Renderer::render_textured_wireframe(const TexturedWireframeMesh& mesh, const Vector4& color, int width, int height, float thickness)
	{
		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * mesh.get_model_matrix();

		trim_line_shader.use();
		glActiveTexture(GL_TEXTURE0);
		mesh.get_texture().bind();
		glUniform1i(utl_trim_texture_location, 0);
		glUniformMatrix4fv(utl_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
		glUniform4f(utl_color_location, color.x, color.y, color.z, color.w);

		//mesh.vao.bind();
		mesh.bind_to_render();

		glLineWidth(thickness);
		glDrawElements(GL_LINES, 2 * mesh.get_line_count(), GL_UNSIGNED_INT, NULL);
	}

	void Renderer::render_points(const PointSet& mesh, const Vector4& color, int width, int height, float thickness)
	{
		/*vbo.bind();
		vbo.set_dynamic_data(mesh.points_data(), mesh.points_size_bytes());*/

		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * mesh.get_model_matrix();

		point_shader.use();
		glUniformMatrix4fv(up_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
		glUniform4f(up_color_location, color.x, color.y, color.z, color.w);

		//vao.bind();
		mesh.bind_to_render();

		glPointSize(thickness * ApplicationSettings::RENDER_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, mesh.get_point_count());
	}

	void Renderer::render_curve_and_polyline(const CurveWithPolyline& curve, const Vector4& color, int width, int height, float thickness)
	{
		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * curve.get_model_matrix();

		if (curve.draw_polyline) {
			line_shader.use();
			glUniformMatrix4fv(ul_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(ul_color_location, curve.polyline_color.x, curve.polyline_color.y, curve.polyline_color.z, curve.polyline_color.w);

			curve.bind_to_render();

			glLineWidth(thickness);
			glDrawArrays(GL_LINE_STRIP, 0, curve.get_point_count());
		}
		if (curve.draw_curve) {
			bezier_shader.use();
			glUniformMatrix4fv(ub_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(ub_color_location, color.x, color.y, color.z, color.w);
			glUniform1f(ub_width_location, width);
			glUniform1f(ub_height_location, height);

			curve.bind_to_render();

			glLineWidth(thickness);
			glDrawElements(GL_LINES_ADJACENCY, curve.get_line_indices_count(), GL_UNSIGNED_INT, NULL);
		}
	}

	void Renderer::render_axes_cursor(const AxesCursor& ac, int width, int height, float thickness)
	{
		auto invs = 1.0f / camera.get_scale();
		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * ac.get_model_matrix() * Matrix4x4::scale(invs, invs, invs);

		cursor_shader.use();
		glUniformMatrix4fv(uc_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);

		//mesh.vao.bind();
		ac.bind_to_render();

		glLineWidth(thickness);
		glDrawArrays(GL_LINES, 0, ac.get_vertex_count());
	}

	void Renderer::render_surface_and_bezier_contour(const SurfaceWithBezierContour& surf, const Vector4& color, int width, int height, float thickness)
	{
		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix();// * surf.get_model_matrix(); -> patch is not transformable

		if (surf.draw_contour)
		{
			line_shader.use();
			glUniformMatrix4fv(ul_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(ul_color_location, surf.contour_color.x, surf.contour_color.y, surf.contour_color.z, surf.contour_color.w);

			surf.bind_contour_to_render();

			glLineWidth(thickness);
			glDrawElements(GL_LINES, surf.get_contour_indices_count(), GL_UNSIGNED_INT, NULL);
		}
		if (surf.draw_patch)
		{
			bezier_patch_shader.use();
			glActiveTexture(GL_TEXTURE0);
			if (surf.has_texture())
				surf.get_texture().bind();
			else
				white_texture.bind();
			glUniform1i(upa_trim_texture_location, 0);
			glPatchParameteri(GL_PATCH_VERTICES, 16);
			glUniformMatrix4fv(upa_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(upa_color_location, color.x, color.y, color.z, color.w);
			glUniform1i(upa_divisions_x_location, surf.divisions_x);
			glUniform1i(upa_divisions_y_location, surf.divisions_y);
			glUniform1f(upa_patches_x_location, surf.get_patches_x_count());
			glUniform1f(upa_patches_y_location, surf.get_patches_y_count());

			surf.bind_patch_to_render();

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(thickness);
			glDrawElements(GL_PATCHES, surf.get_patch_indices_count(), GL_UNSIGNED_INT, NULL);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	void Renderer::render_surface_and_de_boor_contour(const SurfaceWithDeBoorContour& surf, const Vector4& color, int width, int height, float thickness)
	{
		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix();// * surf.get_model_matrix(); -> patch is not transformable

		if (surf.draw_contour)
		{
			line_shader.use();
			glUniformMatrix4fv(ul_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(ul_color_location, surf.contour_color.x, surf.contour_color.y, surf.contour_color.z, surf.contour_color.w);

			surf.bind_contour_to_render();

			glLineWidth(thickness);
			glDrawElements(GL_LINES, surf.get_contour_indices_count(), GL_UNSIGNED_INT, NULL);
		}
		if (surf.draw_patch)
		{
			de_boor_patch_shader.use();
			glActiveTexture(GL_TEXTURE0);
			if (surf.has_texture())
				surf.get_texture().bind();
			else
				white_texture.bind();
			glUniform1i(upb_trim_texture_location, 0);
			glPatchParameteri(GL_PATCH_VERTICES, 16);
			glUniformMatrix4fv(upb_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(upb_color_location, color.x, color.y, color.z, color.w);
			glUniform1i(upb_divisions_x_location, surf.divisions_x);
			glUniform1i(upb_divisions_y_location, surf.divisions_y);
			glUniform1f(upb_patches_x_location, surf.get_patches_x_count());
			glUniform1f(upb_patches_y_location, surf.get_patches_y_count());

			surf.bind_patch_to_render();

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(thickness);
			glDrawElements(GL_PATCHES, surf.get_patch_indices_count(), GL_UNSIGNED_INT, NULL);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	void Renderer::render_rational_20_param_surface(const Rational20ParamSurface& surf, const Vector4& color, int width, int height, float thickness)
	{
		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix();// * surf.get_model_matrix(); -> patch is not transformable

		if (surf.draw_contour)
		{
			line_shader.use();
			glUniformMatrix4fv(ul_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(ul_color_location, surf.contour_color.x, surf.contour_color.y, surf.contour_color.z, surf.contour_color.w);

			surf.bind_to_render();

			glLineWidth(thickness);
			glDrawElements(GL_LINES, surf.get_contour_indices_count(), GL_UNSIGNED_INT, NULL);
		}
		if (surf.draw_patch)
		{
			rational_20_param_patch_shader.use();
			glPatchParameteri(GL_PATCH_VERTICES, 20);
			glUniformMatrix4fv(upr_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
			glUniform4f(upr_color_location, color.x, color.y, color.z, color.w);
			glUniform1i(upr_divisions_x_location, surf.divisions_x);
			glUniform1i(upr_divisions_y_location, surf.divisions_y);

			surf.bind_to_render();

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(thickness);
			glDrawArrays(GL_PATCHES, 0, surf.get_point_count());
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
		simple_shader.use();

		glUniform2f(us_scale_location, rect.scale_x, rect.scale_y);
		glUniform2f(us_pos_location, rect.position.x, rect.position.y);

		glUniform4f(us_color_location, rect.color.x, rect.color.y, rect.color.z, rect.color.w * 0.5f);
		rect.bind_to_render();
		glDrawArrays(GL_TRIANGLES, 0, rect.get_quad_vertices_count());

		glUniform4f(us_color_location, rect.color.x, rect.color.y, rect.color.z, rect.color.w);
		rect.bind_to_render();
		glLineWidth(thickness);
		glDrawArrays(GL_LINE_STRIP, 0, rect.get_line_strip_vertices_count());
	}

	void Renderer::render_line(const Line& line, const Vector4& color, int width, int height, float thickness)
	{
		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * line.get_model_matrix();

		line_shader.use();

		glUniformMatrix4fv(ul_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
		glUniform4f(ul_color_location, color.x, color.y, color.z, color.w);

		line.bind_to_render();

		glLineWidth(thickness);
		glDrawArrays(line.looped ? GL_LINE_LOOP : GL_LINE_STRIP, 0, line.get_point_count());
	}

	void Renderer::render_line_2d(const Line2D& line, const Vector4& color, int width, int height, float thickness)
	{
		two_dim_shader.use();

		auto urange = line.get_u_range(), vrange = line.get_v_range();

		glUniform2f(u2d_urange_location, urange.from, urange.to);
		glUniform2f(u2d_vrange_location, vrange.from, vrange.to);
		glUniform4f(u2d_color_location, color.x, color.y, color.z, color.w);

		line.bind_to_render();

		glLineWidth(thickness);
		glDrawArrays(line.looped ? GL_LINE_LOOP : GL_LINE_STRIP, 0, line.get_point_count());
	}

	void Renderer::render_workpiece_renderable(const WorkpieceRenderable& workpiece_renderable, const Vector4& color, int width, int height, float thickness)
	{
		auto pv = camera.get_projection_matrix(width, height) * camera.get_view_matrix();

		workpiece_shader.use();
		glActiveTexture(GL_TEXTURE0);
		workpiece_renderable.get_texture().bind();
		glUniform1i(uw_height_map_location, 0);
		glUniformMatrix4fv(uw_pv_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(pv).elem);
		glUniformMatrix4fv(uw_m_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(workpiece_renderable.get_model_matrix()).elem);
		glUniform3f(uw_size_location, workpiece_renderable.parent_size.x, workpiece_renderable.parent_size.y, workpiece_renderable.parent_size.z);
		glUniform4f(uw_color_location, color.x, color.y, color.z, color.w);
		auto offset = workpiece_renderable.get_uv_offset();
		glUniform2f(uw_uv_offset_location, offset.x, offset.y);

		//mesh.vao.bind();
		workpiece_renderable.bind_to_render();

		//glLineWidth(thickness);
		glDrawElements(GL_TRIANGLES, workpiece_renderable.get_indices_count(), GL_UNSIGNED_INT, NULL);
	}

	void Renderer::render_triangle_mesh(const TriangleMesh& mesh, const Vector4& color, int width, int height, float thickness)
	{
		auto m = camera.get_projection_matrix(width, height) * camera.get_view_matrix() * mesh.get_model_matrix();

		triangle_shader.use();
		glUniformMatrix4fv(ut_pvm_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(m).elem);
		glUniformMatrix4fv(ut_m_location, 1, GL_FALSE, GLColumnOrderMatrix4x4(mesh.get_model_matrix()).elem);
		glUniform4f(ut_color_location, color.x, color.y, color.z, color.w);

		mesh.bind_to_render();

		glLineWidth(thickness);
		glDrawElements(GL_TRIANGLES, 3 * mesh.get_triangle_count(), GL_UNSIGNED_INT, NULL);
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
