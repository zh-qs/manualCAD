#version 410 compatibility

in vec3 input_pos;
out vec4 input_color;

uniform mat4 u_pvm;

void main() {
	input_color = vec4(input_pos, 1.0f);
	vec4 pos = vec4((gl_VertexID & 1) * input_pos, 1.0f);
	gl_Position = u_pvm * pos;
}