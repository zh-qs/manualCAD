#version 410 compatibility

out vec4 output_color;

uniform vec4 u_color;

void main() {
	output_color = u_color;
}