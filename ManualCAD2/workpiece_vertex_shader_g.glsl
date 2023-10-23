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
	vec4 pos = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	bool in_map_x = false,
		in_map_y = false;

	//pos = vec4(input_uv.x, 0.0f, input_uv.y, 1.0f);

	if (input_uv.x < 0)
		pos.x = -0.5f * u_size.x;
	else if (input_uv.x > 1)
		pos.x = 0.5f * u_size.x;
	else
	{
		pos.x = (input_uv.x - 0.5f) * u_size.x;
		in_map_x = true;
	}

	if (input_uv.y < 0)
		pos.z = -0.5f * u_size.z;
	else if (input_uv.y > 1)
		pos.z = 0.5f * u_size.z;
	else
	{
		pos.z = (input_uv.y - 0.5f) * u_size.z;
		in_map_y = true;
	}

	if (in_map_x && in_map_y)
		pos.y = texture(u_height_map, input_uv).x * u_size.y;

	/*vec3 grad_x = vec3(2.0f * u_uv_offset.x,
		u_size.y * (texture(u_height_map, vec2(input_uv.x + u_uv_offset.x, input_uv.y)).x - texture(u_height_map, vec2(input_uv.x - u_uv_offset.x, input_uv.y)).x),
		0.0f);
	vec3 grad_z = vec3(0.0f,
		u_size.y * (texture(u_height_map, vec2(input_uv.x, input_uv.y + u_uv_offset.y)).x - texture(u_height_map, vec2(input_uv.x, input_uv.y - u_uv_offset.y)).x),
		2.0f * u_uv_offset.y);

	normal = normalize(cross(grad_x, grad_z));
	vec4 wp = u_m * pos;
	world_pos = wp.xyz;
	gl_Position = u_pv * wp;*/
	gl_Position = u_m * pos;
}