#version 410 compatibility

layout(location = 0) in vec3 input_pos;

uniform vec2 u_scale;
uniform vec2 u_pos;

void main() {
	gl_Position = vec4(input_pos.xy * u_scale + u_pos, 0.0f, 1.0f);
}