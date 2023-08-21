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

		Renderable(const Matrix4x4& model, GLint size = 3, GLenum type = GL_FLOAT) : model(model), vao(), vbo() {
			vao.init();
			vbo.init();
			vao.bind();
			vbo.bind();
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, size, type, GL_FALSE, 0, nullptr);
		}
		Renderable(GLint size = 3, GLenum type = GL_FLOAT) : Renderable(Matrix4x4::identity(), size, type) {}

		inline const Matrix4x4& get_model_matrix() const { return model; }
		inline Matrix4x4& get_model_matrix() { return model; }
		inline void set_model_matrix(const Matrix4x4& mat) { model = mat; }
		inline void set_model_matrix(Matrix4x4&& mat) { model = std::move(mat); }
		inline void bind_to_render() const { vao.bind(); }

		virtual void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const = 0;
		virtual void dispose() { vao.dispose(); vbo.dispose(); }
	};
}