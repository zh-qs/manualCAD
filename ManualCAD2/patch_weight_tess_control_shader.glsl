#version 410 compatibility

layout(vertices = 16) out;

uniform int u_divisions_x, u_divisions_y;
in float tcs_weight[];
out float weight[];

void main()
{
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	weight[gl_InvocationID] = tcs_weight[gl_InvocationID];

	if (gl_InvocationID == 0)
	{
		gl_TessLevelOuter[0] = u_divisions_y;
		gl_TessLevelOuter[1] = u_divisions_x;
		gl_TessLevelOuter[2] = u_divisions_y;
		gl_TessLevelOuter[3] = u_divisions_x;

		gl_TessLevelInner[0] = u_divisions_x;
		gl_TessLevelInner[1] = u_divisions_y;
	}
}