#version 410 compatibility

layout(lines) in;
layout(line_strip, max_vertices = 4) out;

const float wrap_threshold = 1.0f;

void main()
{
	vec4 in1 = gl_in[1].gl_Position, in0 = gl_in[0].gl_Position;
	vec2 diff = in1.xy - in0.xy;

	gl_Position = in0;
	EmitVertex();
	if (abs(diff.x) > wrap_threshold || abs(diff.y) > wrap_threshold)
	{
		gl_Position = vec4(
			diff.x > wrap_threshold ? (in1.x - 2.0f) : (diff.x < -wrap_threshold ? (in1.x + 2.0f) : in1.x),
			diff.y > wrap_threshold ? (in1.y - 2.0f) : (diff.y < -wrap_threshold ? (in1.y + 2.0f) : in1.y),
			in1.zw
		);
		EmitVertex();
		EndPrimitive();
	}
	gl_Position = in1;
	EmitVertex();
	if (abs(diff.x) > wrap_threshold || abs(diff.y) > wrap_threshold)
	{
		gl_Position = vec4(
			diff.x > wrap_threshold ? (in0.x + 2.0f) : (diff.x < -wrap_threshold ? (in0.x - 2.0f) : in0.x),
			diff.y > wrap_threshold ? (in0.y + 2.0f) : (diff.y < -wrap_threshold ? (in0.y - 2.0f) : in0.y),
			in0.zw
		);
		EmitVertex();
	}
	EndPrimitive();
}