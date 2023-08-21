#version 410 compatibility

layout(triangles) in;
layout(line_strip, max_vertices = 3) out;
out vec2 tex_coord;

in vec2 tess_coords[];

void create_lines(int idx1, int idx2, int idx3)
{
	gl_Position = gl_in[idx1].gl_Position;
	tex_coord = tess_coords[idx1];
	EmitVertex();
	gl_Position = gl_in[idx2].gl_Position;
	tex_coord = tess_coords[idx2];
	EmitVertex();
	gl_Position = gl_in[idx3].gl_Position;
	tex_coord = tess_coords[idx3];
	EmitVertex();
	EndPrimitive();
}

bool is_diagonal(vec2 v1, vec2 v2)
{
	return v1.x != v2.x && v1.y != v2.y;
}

void main() {
	if (is_diagonal(tess_coords[0], tess_coords[1]))
	{
		create_lines(0, 2, 1);
	}
	else if (is_diagonal(tess_coords[1], tess_coords[2]))
	{
		create_lines(1, 0, 2);
	}
	else
	{
		create_lines(0, 1, 2);
	}
}