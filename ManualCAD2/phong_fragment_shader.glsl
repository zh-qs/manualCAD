#version 410 compatibility

out vec4 output_color;
in vec3 normal;
in vec3 world_pos;

uniform vec3 light_pos = vec3(0.0f, 10.0f, 0.0f);
const float ambient = 0.3f;
const float diffuse = 0.7f;

uniform vec4 u_color;

void main() {
	vec3 l = normalize(light_pos - world_pos);
	output_color = vec4(u_color.xyz * (ambient + diffuse * max(dot(l, normal), 0)), 1.0f);
}