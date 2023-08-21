#version 410 compatibility

layout(vertices = 20) out;

uniform int u_divisions_x, u_divisions_y;

void main()
{
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

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