#version 410 compatibility

layout(location = 0) in vec3 input_pos;
layout(location = 1) in vec3 input_normal;
out vec3 normal;
out vec3 world_pos;

uniform mat4 u_pvm;
uniform mat4 u_m;

void main() {
	normal = input_normal;
	world_pos = (u_m * vec4(input_pos, 1.0f)).xyz;
	gl_Position = u_pvm * vec4(input_pos, 1.0f);
}