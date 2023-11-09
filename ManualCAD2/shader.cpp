#include "shader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace ManualCAD
{
	GLuint Shader::load_shader(const char* filename, GLenum shader_type)
	{
		GLuint id = glCreateShader(shader_type);

		std::string shader_code;
		std::ifstream ifstr(filename);

		// read code
		if (!ifstr.good()) {
			throw std::runtime_error(std::string("Cannot open shader file: ") + filename);
		}

		std::stringstream sstr;
		sstr << ifstr.rdbuf();
		shader_code = sstr.str();
		ifstr.close();

		// compile code
		const GLchar* const shader_ptr = shader_code.c_str();
		glShaderSource(id, 1, &shader_ptr, NULL);
		glCompileShader(id);

		// check shader
		GLint result = GL_FALSE;
		int info_log_length;
		glGetShaderiv(id, GL_COMPILE_STATUS, &result);
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_length);
		if (info_log_length > 0) {
			std::vector<char> message(info_log_length + 1);
			glGetShaderInfoLog(id, info_log_length, NULL, message.data());
			printf("%s\n", message.data());
			throw std::runtime_error(message.data());
		}

		return id;
	}

	void Shader::init(const char* vertex_shader_file, const char* fragment_shader_file)
	{
		auto vertex_shader_id = load_shader(vertex_shader_file, GL_VERTEX_SHADER);
		auto fragment_shader_id = load_shader(fragment_shader_file, GL_FRAGMENT_SHADER);

		// link shaders
		id = glCreateProgram();
		glAttachShader(id, vertex_shader_id);
		glAttachShader(id, fragment_shader_id);
		glLinkProgram(id);

		// check program
		GLint result = GL_FALSE;
		int info_log_length;
		glGetProgramiv(id, GL_LINK_STATUS, &result);
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_log_length);
		if (info_log_length > 0) {
			std::vector<char> message(info_log_length + 1);
			glGetProgramInfoLog(id, info_log_length, NULL, message.data());
			printf("%s\n", message.data());
			throw std::runtime_error(message.data());
		}

		glDetachShader(id, vertex_shader_id);
		glDetachShader(id, fragment_shader_id);

		glDeleteShader(vertex_shader_id);
		glDeleteShader(fragment_shader_id);
	}

	void Shader::init(const char* vertex_shader_file, const char* geometry_shader_file, const char* fragment_shader_file)
	{
		auto vertex_shader_id = load_shader(vertex_shader_file, GL_VERTEX_SHADER);
		auto geometry_shader_id = load_shader(geometry_shader_file, GL_GEOMETRY_SHADER);
		auto fragment_shader_id = load_shader(fragment_shader_file, GL_FRAGMENT_SHADER);

		// link shaders
		id = glCreateProgram();
		glAttachShader(id, vertex_shader_id);
		glAttachShader(id, geometry_shader_id);
		glAttachShader(id, fragment_shader_id);
		glLinkProgram(id);

		// check program
		GLint result = GL_FALSE;
		int info_log_length;
		glGetProgramiv(id, GL_LINK_STATUS, &result);
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_log_length);
		if (info_log_length > 0) {
			std::vector<char> message(info_log_length + 1);
			glGetProgramInfoLog(id, info_log_length, NULL, message.data());
			printf("%s\n", message.data());
			throw std::runtime_error(message.data());
		}

		glDetachShader(id, vertex_shader_id);
		glDetachShader(id, geometry_shader_id);
		glDetachShader(id, fragment_shader_id);

		glDeleteShader(vertex_shader_id);
		glDeleteShader(geometry_shader_id);
		glDeleteShader(fragment_shader_id);
	}

	void Shader::init(const char* vertex_shader_file, const char* tess_control_shader_file, const char* tess_eval_shader_file, const char* fragment_shader_file)
	{
		auto vertex_shader_id = load_shader(vertex_shader_file, GL_VERTEX_SHADER);
		auto tess_control_shader_id = load_shader(tess_control_shader_file, GL_TESS_CONTROL_SHADER);
		auto tess_eval_shader_id = load_shader(tess_eval_shader_file, GL_TESS_EVALUATION_SHADER);
		auto fragment_shader_id = load_shader(fragment_shader_file, GL_FRAGMENT_SHADER);

		// link shaders
		id = glCreateProgram();
		glAttachShader(id, vertex_shader_id);
		glAttachShader(id, tess_control_shader_id);
		glAttachShader(id, tess_eval_shader_id);
		glAttachShader(id, fragment_shader_id);
		glLinkProgram(id);

		// check program
		GLint result = GL_FALSE;
		int info_log_length;
		glGetProgramiv(id, GL_LINK_STATUS, &result);
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_log_length);
		if (info_log_length > 0) {
			std::vector<char> message(info_log_length + 1);
			glGetProgramInfoLog(id, info_log_length, NULL, message.data());
			printf("%s\n", message.data());
			throw std::runtime_error(message.data());
		}

		glDetachShader(id, vertex_shader_id);
		glDetachShader(id, tess_control_shader_id);
		glDetachShader(id, tess_eval_shader_id);
		glDetachShader(id, fragment_shader_id);

		glDeleteShader(vertex_shader_id);
		glDeleteShader(tess_control_shader_id);
		glDeleteShader(tess_eval_shader_id);
		glDeleteShader(fragment_shader_id);
	}

	void Shader::init(const char* vertex_shader_file, const char* tess_control_shader_file, const char* tess_eval_shader_file, const char* geometry_shader_file ,const char* fragment_shader_file)
	{
		auto vertex_shader_id = load_shader(vertex_shader_file, GL_VERTEX_SHADER);
		auto tess_control_shader_id = load_shader(tess_control_shader_file, GL_TESS_CONTROL_SHADER);
		auto tess_eval_shader_id = load_shader(tess_eval_shader_file, GL_TESS_EVALUATION_SHADER);
		auto geometry_shader_id = load_shader(geometry_shader_file, GL_GEOMETRY_SHADER);
		auto fragment_shader_id = load_shader(fragment_shader_file, GL_FRAGMENT_SHADER);

		// link shaders
		id = glCreateProgram();
		glAttachShader(id, vertex_shader_id);
		glAttachShader(id, tess_control_shader_id);
		glAttachShader(id, tess_eval_shader_id);
		glAttachShader(id, geometry_shader_id);
		glAttachShader(id, fragment_shader_id);
		glLinkProgram(id);

		// check program
		GLint result = GL_FALSE;
		int info_log_length;
		glGetProgramiv(id, GL_LINK_STATUS, &result);
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_log_length);
		if (info_log_length > 0) {
			std::vector<char> message(info_log_length + 1);
			glGetProgramInfoLog(id, info_log_length, NULL, message.data());
			printf("%s\n", message.data());
			throw std::runtime_error(message.data());
		}

		glDetachShader(id, vertex_shader_id);
		glDetachShader(id, tess_control_shader_id);
		glDetachShader(id, tess_eval_shader_id);
		glDetachShader(id, geometry_shader_id);
		glDetachShader(id, fragment_shader_id);

		glDeleteShader(vertex_shader_id);
		glDeleteShader(tess_control_shader_id);
		glDeleteShader(tess_eval_shader_id);
		glDeleteShader(geometry_shader_id);
		glDeleteShader(fragment_shader_id);
	}

	void Shader::use() const
	{
		glUseProgram(id);
	}

	GLuint Shader::get_uniform_location(const GLchar* name) const
	{
		return glGetUniformLocation(id, name);
	}
}