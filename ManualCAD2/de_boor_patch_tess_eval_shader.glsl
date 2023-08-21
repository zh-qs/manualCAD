#version 410 compatibility

layout(quads, equal_spacing) in;

uniform mat4 u_pvm;
uniform float u_patches_x, u_patches_y;

out vec2 tess_coords;

vec4 de_boor(vec4 b0, vec4 b1, vec4 b2, vec4 b3, float t)
{
	// equidistant knots t_i = i
	float n01 = 1;
	float n10 = n01 * (1 - t), n11 = n01 * t;
	float n2m1 = n10 * (1 - t) / 2.0f, n20 = n10 * (t + 1) / 2.0f + n11 * (2 - t) / 2.0f, n21 = n11 * t / 2.0f;
	float n3m2 = n2m1 * (1 - t) / 3.0f, n3m1 = n2m1 * (t + 2) / 3.0f + n20 * (2 - t) / 3.0f, n30 = n20 * (t + 1) / 3.0f + n21 * (3 - t) / 3.0f, n31 = n21 * t / 3.0f;

	return b0 * n3m2 + b1 * n3m1 + b2 * n30 + b3 * n31;
}

void main()
{
	float u = gl_TessCoord.x, v = gl_TessCoord.y;
	tess_coords = vec2((u + mod(gl_PrimitiveID, u_patches_y)) / u_patches_y, (v + floor(gl_PrimitiveID / u_patches_y)) / u_patches_x);
	tess_coords = tess_coords.yx;// min(tess_coords.yx, vec2(0.99999f, 0.99999f));

	vec4 p0 = de_boor(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position, u);
	vec4 p1 = de_boor(gl_in[4].gl_Position, gl_in[5].gl_Position, gl_in[6].gl_Position, gl_in[7].gl_Position, u);
	vec4 p2 = de_boor(gl_in[8].gl_Position, gl_in[9].gl_Position, gl_in[10].gl_Position, gl_in[11].gl_Position, u);
	vec4 p3 = de_boor(gl_in[12].gl_Position, gl_in[13].gl_Position, gl_in[14].gl_Position, gl_in[15].gl_Position, u);

	gl_Position = u_pvm * de_boor(p0, p1, p2, p3, v);
}