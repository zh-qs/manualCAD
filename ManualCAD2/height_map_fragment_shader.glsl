#version 410 compatibility

out vec4 output_color;
in vec3 view_pos;

uniform vec4 u_color;
uniform float is_idx_map;
uniform float au_surf_idx;
uniform float au_surf_count;

void main() {
	if (is_idx_map > 0.0f)
		output_color = vec4(au_surf_idx / au_surf_count, 0.0f, 0.0f, 1.0f);
	else
		output_color = vec4(1.0f - view_pos.z,0.0f,0.0f,1.0f);
}