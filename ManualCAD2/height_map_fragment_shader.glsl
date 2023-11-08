#version 410 compatibility

out vec4 output_color;
in vec4 view_pos;

uniform vec4 u_color;

void main() {
	output_color = vec4(view_pos.z,0.0f,0.0f,1.0f);
}