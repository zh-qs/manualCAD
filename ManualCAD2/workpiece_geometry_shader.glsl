#version 410 compatibility

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
out vec3 normal;
out vec3 world_pos;

uniform mat4 u_pv;

void main() {
	normal = normalize(cross(gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz));

	world_pos = gl_in[0].gl_Position.xyz;
	gl_Position = u_pv * gl_in[0].gl_Position;
	EmitVertex();
	world_pos = gl_in[1].gl_Position.xyz;
	gl_Position = u_pv * gl_in[1].gl_Position;
	EmitVertex();
	world_pos = gl_in[2].gl_Position.xyz;
	gl_Position = u_pv * gl_in[2].gl_Position;
	EmitVertex();

	EndPrimitive();
}