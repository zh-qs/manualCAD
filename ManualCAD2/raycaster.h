#pragma once

#include <memory>
#include "ellipsoid.h"
#include "vertex_array.h"
#include "buffer.h"
#include "shader.h"
#include <imgui.h>
#include "window.h"
#include "camera.h"
#include "frame_buffer.h"
#include "texture.h"

namespace ManualCAD
{
	class Raycaster {
		VertexArray vao;
		VertexBuffer screen_vbo;
		VertexBuffer uvs_vbo;
		FrameBuffer fbo;
		Texture texture;

		Ellipsoid ellipsoid;

		Shader shader;
		GLint u_drawable_color_location;
		GLint u_drawable_form_location;
		GLint u_screen_dims_location;
		GLint u_specular_exponent_location;

		Shader quad_shader;
		GLint u_texture_location;

		Camera camera;

		int downsampling_scale;
		int current_downsampling_scale;
	public:

		Raycaster();

		void init(int initial_downsampling_scale);
		void render(int width, int height);
		void reset_downsampling() { current_downsampling_scale = downsampling_scale; }
		WindowHandle create_settings_window();
		Camera& get_camera() { return camera; }
		void dispose();
	};
}