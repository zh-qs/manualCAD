#version 410 compatibility

layout(location = 0) in vec3 input_pos;
layout(location = 1) in vec2 input_uv;

out vec2 fragment_uv;

void main() {
	gl_Position = vec4(input_pos, 1.0f);
	fragment_uv = input_uv;
}