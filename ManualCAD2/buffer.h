#pragma once

#include <glad/glad.h>

namespace ManualCAD
{
	template <typename T, GLenum TARGET>
	class GlBuffer {
		GLuint id;
	public:
		void init() {
			glGenBuffers(1, &id);
		}

		void bind() const {
			glBindBuffer(TARGET, id);
		}

		void unbind() const {
			glBindBuffer(TARGET, 0);
		}

		void set_static_data(const T* data, GLsizeiptr data_size) {
			glBufferData(TARGET, data_size, data, GL_STATIC_DRAW);
		}

		void set_dynamic_data(const T* data, GLsizeiptr data_size) {
			glBufferData(TARGET, data_size, data, GL_DYNAMIC_DRAW);
		}

		void dispose() {
			glDeleteBuffers(1, &id);
		}
	};

	using VertexBuffer = GlBuffer<GLfloat, GL_ARRAY_BUFFER>;
	using ElementBuffer = GlBuffer<GLuint, GL_ELEMENT_ARRAY_BUFFER>;
}