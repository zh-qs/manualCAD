#version 410 compatibility

layout(quads, equal_spacing) in;
in float weight[];

uniform mat4 u_pvm;
uniform float u_patches_x, u_patches_y;
uniform float au_offset;

out vec2 tess_coords;
out vec3 view_pos;

vec4 de_boor3(vec4 b0, vec4 b1, vec4 b2, vec4 b3, float t)
{
	// equidistant knots t_i = i
	float n01 = 1;
	float n10 = n01 * (1 - t), n11 = n01 * t;
	float n2m1 = n10 * (1 - t) / 2.0f, n20 = n10 * (t + 1) / 2.0f + n11 * (2 - t) / 2.0f, n21 = n11 * t / 2.0f;
	float n3m2 = n2m1 * (1 - t) / 3.0f, n3m1 = n2m1 * (t + 2) / 3.0f + n20 * (2 - t) / 3.0f, n30 = n20 * (t + 1) / 3.0f + n21 * (3 - t) / 3.0f, n31 = n21 * t / 3.0f;

	return b0 * n3m2 + b1 * n3m1 + b2 * n30 + b3 * n31;
}

vec4 de_boor2(vec4 b0, vec4 b1, vec4 b2, float t)
{
	// equidistant knots t_i = i
	float n01 = 1;
	float n10 = n01 * (1 - t), n11 = n01 * t;
	float n2m1 = n10 * (1 - t) / 2.0f, n20 = n10 * (t + 1) / 2.0f + n11 * (2 - t) / 2.0f, n21 = n11 * t / 2.0f;

	return b0 * n2m1 + b1 * n20 + b2 * n21;
}

vec4 nurbs3(vec4 b0, vec4 b1, vec4 b2, vec4 b3, float w0, float w1, float w2, float w3, float t)
{
	// equidistant knots t_i = i
	float n01 = 1;
	float n10 = n01 * (1 - t), n11 = n01 * t;
	float n2m1 = n10 * (1 - t) / 2.0f, n20 = n10 * (t + 1) / 2.0f + n11 * (2 - t) / 2.0f, n21 = n11 * t / 2.0f;
	float n3m2 = n2m1 * (1 - t) / 3.0f, n3m1 = n2m1 * (t + 2) / 3.0f + n20 * (2 - t) / 3.0f, n30 = n20 * (t + 1) / 3.0f + n21 * (3 - t) / 3.0f, n31 = n21 * t / 3.0f;

	return b0 * w0 * n3m2 + b1 * w1 * n3m1 + b2 * w2 * n30 + b3 * w3 * n31;
}

vec4 dnurbs3(vec4 b0, vec4 b1, vec4 b2, vec4 b3, float w0, float w1, float w2, float w3, float t)
{
	// equidistant knots t_i = i
	float n01 = 1;
	float n10 = n01 * (1 - t), n11 = n01 * t;
	float n2m1 = n10 * (1 - t) / 2.0f, n20 = n10 * (t + 1) / 2.0f + n11 * (2 - t) / 2.0f, n21 = n11 * t / 2.0f;

	return (b1 * w1 - b0 * w0) * n2m1 + (b2 * w2 - b1 * w1) * n20 + (b3 * w3 - b2 * w2) * n21;
}

void main()
{
	float u = gl_TessCoord.x, v = gl_TessCoord.y;
	tess_coords = vec2((u + mod(gl_PrimitiveID, u_patches_y)) / u_patches_y, (v + floor(gl_PrimitiveID / u_patches_y)) / u_patches_x);
	tess_coords = tess_coords.yx;// min(tess_coords.yx, vec2(0.99999f, 0.99999f));

	vec4 p0 = nurbs3(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position, 
		weight[0], weight[1], weight[2], weight[3], u);
	vec4 p1 = nurbs3(gl_in[4].gl_Position, gl_in[5].gl_Position, gl_in[6].gl_Position, gl_in[7].gl_Position,
		weight[4], weight[5], weight[6], weight[7], u);
	vec4 p2 = nurbs3(gl_in[8].gl_Position, gl_in[9].gl_Position, gl_in[10].gl_Position, gl_in[11].gl_Position,
		weight[8], weight[9], weight[10], weight[11], u);
	vec4 p3 = nurbs3(gl_in[12].gl_Position, gl_in[13].gl_Position, gl_in[14].gl_Position, gl_in[15].gl_Position,
		weight[12], weight[13], weight[14], weight[15], u);

	vec4 dp0 = dnurbs3(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position,
		weight[0], weight[1], weight[2], weight[3], u);
	vec4 dp1 = dnurbs3(gl_in[4].gl_Position, gl_in[5].gl_Position, gl_in[6].gl_Position, gl_in[7].gl_Position,
		weight[4], weight[5], weight[6], weight[7], u);
	vec4 dp2 = dnurbs3(gl_in[8].gl_Position, gl_in[9].gl_Position, gl_in[10].gl_Position, gl_in[11].gl_Position,
		weight[8], weight[9], weight[10], weight[11], u);
	vec4 dp3 = dnurbs3(gl_in[12].gl_Position, gl_in[13].gl_Position, gl_in[14].gl_Position, gl_in[15].gl_Position,
		weight[12], weight[13], weight[14], weight[15], u);

	vec4 result = de_boor3(p0, p1, p2, p3, v);

	vec4 du = de_boor3(dp0, dp1, dp2, dp3, v);
	vec4 dv = de_boor2(p1 - p0, p2 - p1, p3 - p2, v);

	vec3 actual_du = du.xyz / result.w - result.xyz * du.w / (result.w * result.w);
	vec3 actual_dv = dv.xyz / result.w - result.xyz * dv.w / (result.w * result.w);
	vec3 normal = normalize(cross(actual_du.xyz, actual_dv.xyz));

	gl_Position = u_pvm * (vec4(result.xyz / result.w, 1.0f) - au_offset * vec4(normal, 0.0f));
	view_pos = gl_Position.xyz;
}