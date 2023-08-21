#version 410 compatibility

layout(location = 0) in vec2 input_pos;
uniform vec2 u_urange, u_vrange;

float scale(float v, vec2 range) {
	return 2.0f * (v / (range.y - range.x) - range.x) - 1.0f;
}

void main() {
	gl_Position = vec4(scale(input_pos.x, u_urange), scale(input_pos.y, u_vrange), 0.0f, 1.0f);
}