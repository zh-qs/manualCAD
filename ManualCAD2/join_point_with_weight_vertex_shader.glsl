#version 410 compatibility

layout(location = 0) in vec3 input_pos;
layout(location = 1) in float weight;

out float tcs_weight;

void main() {
	gl_Position = vec4(input_pos, 1.0f);
	tcs_weight = weight;
}