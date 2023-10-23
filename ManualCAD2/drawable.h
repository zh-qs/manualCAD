#pragma once
#include "algebra.h"
#include "vertex_array.h"
#include "buffer.h"
#include "renderable.h"

namespace ManualCAD
{
	class Renderer;

	class Drawable : public Renderable {
	protected:
		Matrix4x4 model;
		VertexArray vao;
		VertexBuffer vbo;
	public:

		Drawable(const Matrix4x4& model, GLint size = 3, GLenum type = GL_FLOAT) : model(model), vao(), vbo() {
			vao.init();
			vbo.init();
			vao.bind();
			vbo.bind();
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, size, type, GL_FALSE, 0, nullptr);
		}
		Drawable(GLint size = 3, GLenum type = GL_FLOAT) : Drawable(Matrix4x4::identity(), size, type) {}

		inline const Matrix4x4& get_model_matrix() const { return model; }
		inline Matrix4x4& get_model_matrix() { return model; }
		inline void set_model_matrix(const Matrix4x4& mat) { model = mat; }
		inline void set_model_matrix(Matrix4x4&& mat) { model = std::move(mat); }
		inline void bind_to_render() const { vao.bind(); }

		//virtual void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const = 0;
		virtual void dispose() override { Renderable::dispose(); vao.dispose(); vbo.dispose(); }
		void set_transformation(const Transformation& transformation) override { set_model_matrix(transformation.get_matrix()); }
		void set_combined_transformation(const Transformation& transformation, const Transformation& combine_transformation, const Vector3& center) override { set_model_matrix(transformation.get_matrix_combined_with(combine_transformation, center)); }
	};
}