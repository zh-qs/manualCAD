#pragma once
#include <glad/glad.h>

namespace ManualCAD
{
	class Shader {
	protected:
		GLuint load_shader(const char* filename, GLenum shader_type);
	public:
		GLuint id;
		void init(const char* vertex_shader_file, const char* fragment_shader_file);
		void init(const char* vertex_shader_file, const char* geometry_shader_file, const char* fragment_shader_file);
		void init(const char* vertex_shader_file, const char* tess_control_shader_file, const char* tess_eval_shader_file, const char* fragment_shader_file);
		void init(const char* vertex_shader_file, const char* tess_control_shader_file, const char* tess_eval_shader_file, const char* geometry_shader_file, const char* fragment_shader_file);
		void use() const;
		GLuint get_uniform_location(const GLchar* name) const;
	};
}