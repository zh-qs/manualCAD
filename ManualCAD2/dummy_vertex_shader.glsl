#version 410 compatibility

layout(location = 0) in vec3 input_pos;

void main() {
	gl_Position = vec4(input_pos, 1.0f);
}