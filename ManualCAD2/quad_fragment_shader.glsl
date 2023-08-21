#version 410 compatibility

in vec2 fragment_uv;
out vec4 output_color;

uniform sampler2D u_rendered_texture;

void main() {
	vec4 color = texture(u_rendered_texture, fragment_uv);
	if (color.w == 0) {
		discard;
	}
	else {
		output_color = color;
	}
}
