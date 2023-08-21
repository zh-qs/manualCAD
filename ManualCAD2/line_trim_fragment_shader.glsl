#version 410 compatibility

in vec2 tex_coord;

out vec4 output_color;

uniform sampler2D u_trim_texture;
uniform vec4 u_color;

void main() {
	if (texture(u_trim_texture, tex_coord).w < 1.0f)
		discard;
	else
		output_color = u_color;
}