#pragma once

#include <glad/glad.h>
#include "exception.h"

namespace ManualCAD
{
	template <GLenum FORMAT, GLenum INTERNALFORMAT = FORMAT>
	class GlTexture {
		GLuint id;
	public:
		GLuint get_id() const { return id; }

		void init() {
			glGenTextures(1, &id);
		}

		void bind() const {
			glBindTexture(GL_TEXTURE_2D, id);
		}

		void unbind() const {
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		void set_size(int width, int height) {
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, INTERNALFORMAT, width, height, 0, FORMAT, GL_FLOAT, nullptr);
		}

		void set_sub_image(int xoffset, int yoffset, int width, int height, const void* pixels) {
			glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, FORMAT, GL_FLOAT, pixels);
		}

		void set_image(int width, int height, const void* pixels) {
			glTexImage2D(GL_TEXTURE_2D, 0, INTERNALFORMAT, width, height, 0, FORMAT, GL_FLOAT, pixels);
		}

		void get_image(int width, int height, void* pixels) const {
			glGetTexImage(GL_TEXTURE_2D, 0, FORMAT, GL_FLOAT, pixels);
		}

		void configure(GLint mag_filter = GL_NEAREST, GLint min_filter = GL_NEAREST, GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
			// a framebuffer must be bind before!
			GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
			glFramebufferTexture2D(GL_FRAMEBUFFER, drawBuffer, GL_TEXTURE_2D, id, 0);
			glDrawBuffers(1, &drawBuffer);

			// Always check that our framebuffer is ok
			//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			//	THROW_EXCEPTION;
		}

		void set_as_read() {
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
		}

		void dispose() {
			glDeleteTextures(1, &id);
		}
	};

	using Texture = GlTexture<GL_RGBA>;
	using TexMap = GlTexture<GL_RED, GL_R32F>;
}