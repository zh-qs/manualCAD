#pragma once
#include "algebra.h"
#include "vertex_array.h"
#include "buffer.h"

namespace ManualCAD
{
	class Renderer;

	class Renderable {
	protected:
		Matrix4x4 model;
		VertexArray vao;
		VertexBuffer vbo;
	public:
		bool visible = true;
		Vector4 color = { 0.0f,0.0f,0.0f,0.0f };

		// VAO must be unbound in derived class constructor!
		Renderable(const Matrix4x4& model, GLint size = 3, GLenum type = GL_FLOAT) : model(model), vao(), vbo() {
			vao.init();
			vao.bind();
			vbo.init();
			vbo.bind();
			vbo.attrib_buffer(0, size, type);
		}
		// VAO must be unbound in derived class constructor!
		Renderable(GLint size = 3, GLenum type = GL_FLOAT) : Renderable(Matrix4x4::identity(), size, type) {}

		inline const Matrix4x4& get_model_matrix() const { return model; }
		inline Matrix4x4& get_model_matrix() { return model; }
		inline void set_model_matrix(const Matrix4x4& mat) { model = mat; }
		inline void set_model_matrix(Matrix4x4&& mat) { model = std::move(mat); }
		inline void bind_to_render() const { vao.bind(); }
		inline void unbind_from_render() const { vao.unbind(); }

		virtual void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const = 0;
		virtual void dispose() { vao.dispose(); vbo.dispose(); }

		static void assert_unbound();
	};
}