#version 410 compatibility

in vec3 input_pos;

uniform mat4 u_pvm;

void main() {
	gl_Position = u_pvm * vec4(input_pos, 1.0f);
}