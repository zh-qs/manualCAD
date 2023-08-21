#include "raycaster.h"
#include "exception.h"
#include "settings_window.h"
#include <glad/glad.h>
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

	Raycaster::Raycaster()
		: ellipsoid(0.25f, 0.75f, 0.5f, { 1.0f, 1.0f, 0.0f, 1.0f }), vao(), screen_vbo(), uvs_vbo(), texture(), fbo() {}

	void Raycaster::init(int initial_downsampling_scale) {
		downsampling_scale = initial_downsampling_scale;
		current_downsampling_scale = downsampling_scale;

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			THROW_EXCEPTION;
		vao.init();
		screen_vbo.init();
		uvs_vbo.init();
		vao.bind();
		screen_vbo.bind();
		screen_vbo.set_static_data(screen_vertices, sizeof(screen_vertices));
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
		uvs_vbo.bind();
		uvs_vbo.set_static_data(screen_uvs, sizeof(screen_uvs));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(1);

		shader.init("raycasting_vertex_shader.glsl", "raycasting_fragment_shader.glsl");
		quad_shader.init("raycasting_vertex_shader.glsl", "quad_fragment_shader.glsl");

		fbo.init();
		fbo.bind();
		texture.init();
		texture.bind();
		texture.configure();
		fbo.unbind();

		u_drawable_color_location = shader.get_uniform_location("u_drawable.color");
		u_drawable_form_location = shader.get_uniform_location("u_drawable.form");
		u_screen_dims_location = shader.get_uniform_location("u_screen_dims");
		u_specular_exponent_location = shader.get_uniform_location("u_specular_exponent");
		u_texture_location = quad_shader.get_uniform_location("u_rendered_texture");
	}

	void Raycaster::render(int width, int height) {
		int smallWidth = width / current_downsampling_scale, smallHeight = height / current_downsampling_scale;

		auto ellipsoid_form = ellipsoid.get_transformed_form(camera.get_bilinear_form_transformation_matrix());
		// bind framebuffer and set render viewport
		fbo.bind();
		texture.bind();
		texture.set_size(smallWidth, smallHeight);
		glViewport(0, 0, smallWidth, smallHeight);

		// clear texture
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		// send uniform data
		shader.use();
		glUniform4f(u_drawable_color_location, ellipsoid.color.x, ellipsoid.color.y, ellipsoid.color.z, ellipsoid.color.w);
		glUniformMatrix4fv(u_drawable_form_location, 1, GL_FALSE, ellipsoid_form.elem);
		glUniform2f(u_screen_dims_location, width, height);
		glUniform1f(u_specular_exponent_location, camera.specular_exponent);

		// render texture
		vao.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// bind screen framebuffer
		fbo.unbind();
		glViewport(0, 0, width, height);

		// use shader and send uniform data (texture sampler)
		quad_shader.use();
		glActiveTexture(GL_TEXTURE0);
		texture.bind();
		glUniform1i(u_texture_location, 0);

		// render screen
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//// bind our and screen framebuffers
		//fbo.bind_to_read();
		//texture.set_as_read();
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		////glFramebufferTexture(GL_DRAW_BUFFER, GL_COLOR_ATTACHMENT0, 0, 0);

		//glBlitFramebuffer(0, 0, smallWidth, smallHeight, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// divide downsampling scale if greater than 1
		if (current_downsampling_scale > 1) current_downsampling_scale /= 2;
	}

	void Raycaster::dispose() {
		uvs_vbo.dispose();
		screen_vbo.dispose();
		vao.dispose();
		fbo.dispose();
		texture.dispose();
	}

	WindowHandle Raycaster::create_settings_window()
	{
		return make_window<RaycasterSettingsWindow>(*this, camera, ellipsoid, downsampling_scale);
	}
}