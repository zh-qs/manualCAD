#version 410 compatibility

layout (location = 0) in vec3 input_pos;
layout (location = 1) in vec2 input_tex_coord;

out vec2 tex_coord;

uniform mat4 u_pvm;

void main() {
	gl_Position = u_pvm * vec4(input_pos, 1.0f);
	tex_coord = input_tex_coord;
}