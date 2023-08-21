#version 410 compatibility

layout(quads, equal_spacing) in;

uniform mat4 u_pvm;

out vec2 tess_coords;

vec4 de_casteljau(vec4 b0, vec4 b1, vec4 b2, vec4 b3, float t)
{
	b0 = (1 - t) * b0 + t * b1;
	b1 = (1 - t) * b1 + t * b2;
	b2 = (1 - t) * b2 + t * b3;

	b0 = (1 - t) * b0 + t * b1;
	b1 = (1 - t) * b1 + t * b2;

	b0 = (1 - t) * b0 + t * b1;

	return b0;
}

void main()
{
	float u = gl_TessCoord.x, v = gl_TessCoord.y;
	tess_coords = vec2(u, v);

	// 0-3: 1st row (4) 00--10
	// 4-9: 2nd row (6: internal doubled) 00--10
	// 10-15: 3rd row (6: internal doubled) 10--11
	// 16-19: 4rd row (4) 10--11
	vec4 v00ab = (u == 0 && v == 0) ? vec4(0.0f, 0.0f, 0.0f, 1.0f) : ((u * gl_in[6].gl_Position + v * gl_in[5].gl_Position) / (u + v));
	vec4 v01ab = (u == 0 && v == 1) ? vec4(0.0f, 0.0f, 0.0f, 1.0f) : ((u * gl_in[12].gl_Position + (1 - v) * gl_in[11].gl_Position) / (1 + u - v));
	vec4 v10ab = (u == 1 && v == 0) ? vec4(0.0f, 0.0f, 0.0f, 1.0f) : (((1 - u) * gl_in[7].gl_Position + v * gl_in[8].gl_Position) / (1 - u + v));
	vec4 v11ab = (u == 1 && v == 1) ? vec4(0.0f, 0.0f, 0.0f, 1.0f) : (((1 - u) * gl_in[13].gl_Position + (1 - v) * gl_in[14].gl_Position) / (2 - u - v));

	vec4 p0 = de_casteljau(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position, u);
	vec4 p1 = de_casteljau(gl_in[4].gl_Position, v00ab, v10ab, gl_in[9].gl_Position, u);
	vec4 p2 = de_casteljau(gl_in[10].gl_Position, v01ab, v11ab, gl_in[15].gl_Position, u);
	vec4 p3 = de_casteljau(gl_in[16].gl_Position, gl_in[17].gl_Position, gl_in[18].gl_Position, gl_in[19].gl_Position, u);

	gl_Position = u_pvm * de_casteljau(p0, p1, p2, p3, v);
}