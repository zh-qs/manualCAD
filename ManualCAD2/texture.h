#pragma once

#include <glad/glad.h>
#include "exception.h"

namespace ManualCAD
{
	template <GLenum FORMAT, GLenum INTERNALFORMAT = FORMAT, bool WITH_RENDERBUFFER = false>
	class GlTexture {
		GLuint id;
		GLuint rbid;
	public:
		GLuint get_id() const { return id; }

		void init() {
			glGenTextures(1, &id);
			if constexpr (WITH_RENDERBUFFER)
				glGenRenderbuffers(1, &rbid);
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

			if constexpr (WITH_RENDERBUFFER)
			{
				glBindRenderbuffer(GL_RENDERBUFFER, rbid);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
				glBindRenderbuffer(GL_RENDERBUFFER, 0);
			}
		}

		void set_sub_image(int xoffset, int yoffset, int width, int height, const void* pixels) {
			glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, FORMAT, GL_FLOAT, pixels);
		}

		void set_image(int width, int height, const void* pixels) {
			glTexImage2D(GL_TEXTURE_2D, 0, INTERNALFORMAT, width, height, 0, FORMAT, GL_FLOAT, pixels);

			if constexpr (WITH_RENDERBUFFER)
			{
				glBindRenderbuffer(GL_RENDERBUFFER, rbid);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
				glBindRenderbuffer(GL_RENDERBUFFER, 0);
			}
		}

		void get_image(void* pixels) const {
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

			if constexpr (WITH_RENDERBUFFER)
			{
				glBindRenderbuffer(GL_RENDERBUFFER, rbid);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 100, 100); // xd
				glBindRenderbuffer(GL_RENDERBUFFER, 0);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbid);
			}

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
	using RenderTexture = GlTexture<GL_RGBA, GL_RGBA, true>;
	using TexMap = GlTexture<GL_RED, GL_R32F>;
	using RenderTexMap = GlTexture<GL_RED, GL_R32F, true>;
}