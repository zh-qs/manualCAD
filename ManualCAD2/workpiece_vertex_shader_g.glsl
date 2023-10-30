#version 410 compatibility

in vec2 input_uv;
//out vec3 normal;
//out vec3 world_pos;

uniform mat4 u_m;
uniform mat4 u_pv;
uniform vec3 u_size;
uniform vec2 u_uv_offset;
uniform sampler2D u_height_map;

void main() {
	bool in_map_x = input_uv.x >= 0 && input_uv.x <= 1,
		in_map_y = input_uv.y >= 0 && input_uv.y <= 1;

	vec2 clamped_uv = clamp(input_uv, vec2(0, 0), vec2(1, 1));
	vec4 pos = vec4((input_uv.x - 0.5f) * u_size.x, 0.0f, (input_uv.y - 0.5f) * u_size.z, 1.0f);

	if (in_map_x && in_map_y)
		pos.y = texture(u_height_map, input_uv).x * u_size.y;

	gl_Position = u_m * pos;
}