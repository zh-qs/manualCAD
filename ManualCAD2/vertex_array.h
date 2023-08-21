#pragma once

#include <glad/glad.h>

namespace ManualCAD
{
	class VertexArray {
		GLuint id;
	public:
		void init() {
			glGenVertexArrays(1, &id);
		}

		void bind() const {
			glBindVertexArray(id);
		}

		void unbind() const {
			glBindVertexArray(0);
		}

		void dispose() {
			glDeleteVertexArrays(1, &id);
		}
	};
}