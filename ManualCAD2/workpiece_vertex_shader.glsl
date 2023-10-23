#version 410 compatibility

in vec2 input_uv;

uniform mat4 u_m;
uniform vec3 u_size;
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

	gl_Position = u_m * pos;
}