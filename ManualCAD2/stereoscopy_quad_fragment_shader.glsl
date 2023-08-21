#version 410 compatibility

in vec2 fragment_uv;
out vec4 output_color;

uniform sampler2D u_tex0;
uniform sampler2D u_tex1;
uniform vec4 u_lcolor;
uniform float u_saturation;

vec4 saturate(vec4 color) {
	float mono = (0.2125f * color.x) + (0.7154f * color.y) + (0.0721f * color.z);
	return u_saturation * color + (1.0f - u_saturation) * vec4(mono, mono, mono, color.w);
}

void main() {
	vec4 color0 = texture(u_tex0, fragment_uv);
	vec4 color1 = texture(u_tex1, fragment_uv);
	if (color0.w == 0 && color1.w == 0) {
		discard;
	}
	else {
		output_color = saturate(color0) * u_lcolor + saturate(color1) * (vec4(1.0f, 1.0f, 1.0f, 2.0f) - u_lcolor);
	}
}
