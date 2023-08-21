#pragma once

#include <glad/glad.h>

namespace ManualCAD
{
	class FrameBuffer {
		GLuint id;
		GLuint texture_id;
		GLuint renderbuffer_id;
	public:
		void init() {
			glGenFramebuffers(1, &id);
		}

		void bind() const {
			glBindFramebuffer(GL_FRAMEBUFFER, id);
		}

		void unbind() const {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void dispose() {
			glDeleteFramebuffers(1, &id);
		}

		void bind_to_read() {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
		}
	};
}