#version 410 compatibility

out vec4 output_color;

uniform vec4 u_color;

void main() {
	vec2 coord = gl_PointCoord - vec2(0.5f, 0.5f);
	if (length(coord) > 0.5f)
		discard;
	output_color = u_color;
}