#include "shader_library.h"

namespace ManualCAD
{
	ShaderLibrary ShaderLibrary::library{};

	ShaderSet get_default_set() 
	{
		ShaderSet set{};
		set.line_shader.init("line_vertex_shader.glsl", "line_fragment_shader.glsl");
		set.trim_line_shader.init("line_tex_vertex_shader.glsl", "line_trim_fragment_shader.glsl");
		set.point_shader.init("line_vertex_shader.glsl", "point_fragment_shader.glsl");
		set.cursor_shader.init("cursor_vertex_shader.glsl", "cursor_fragment_shader.glsl");
		set.bezier_shader.init("line_vertex_shader.glsl", "bezier_curve_geometry_shader.glsl", "line_fragment_shader.glsl");
		set.bezier_patch_shader.init("dummy_vertex_shader.glsl", "patch_tess_control_shader.glsl", "bezier_patch_tess_eval_shader.glsl", "patch_remove_diagonal_geometry_shader.glsl", "line_trim_fragment_shader.glsl");
		set.de_boor_patch_shader.init("dummy_vertex_shader.glsl", "patch_tess_control_shader.glsl", "de_boor_patch_tess_eval_shader.glsl", "patch_remove_diagonal_geometry_shader.glsl", "line_trim_fragment_shader.glsl");
		set.rational_20_param_patch_shader.init("dummy_vertex_shader.glsl", "rational_20_param_patch_tess_control_shader.glsl", "rational_20_param_patch_tess_eval_shader.glsl", "patch_remove_diagonal_geometry_shader.glsl", "line_fragment_shader.glsl");
		set.simple_shader.init("simple_rect_vertex_shader.glsl", "line_fragment_shader.glsl");
		set.two_dim_shader.init("2d_vertex_shader.glsl", "wrap_around_parameters_geometry_shader.glsl", "line_fragment_shader.glsl");
		//set.workpiece_shader.init("workpiece_vertex_shader.glsl", "phong_fragment_shader.glsl");
		set.workpiece_shader.init("workpiece_vertex_shader_g.glsl", "workpiece_geometry_shader.glsl", "phong_fragment_shader.glsl");
		set.triangle_shader.init("triangle_vertex_shader.glsl", "phong_fragment_shader.glsl");

		return set;
	}

	ShaderSet get_height_map_set()
	{
		ShaderSet set{};
		set.line_shader.init("line_vertex_shader.glsl", "line_fragment_shader.glsl");
		set.trim_line_shader.init("line_tex_vertex_shader.glsl", "line_trim_fragment_shader.glsl");
		set.point_shader.init("line_vertex_shader.glsl", "point_fragment_shader.glsl");
		set.cursor_shader.init("cursor_vertex_shader.glsl", "cursor_fragment_shader.glsl");
		set.bezier_shader.init("line_vertex_shader.glsl", "bezier_curve_geometry_shader.glsl", "line_fragment_shader.glsl");
		set.bezier_patch_shader.init("dummy_vertex_shader.glsl", "patch_tess_control_shader.glsl", "bezier_patch_tess_eval_shader.glsl", "height_map_fragment_shader.glsl");
		set.de_boor_patch_shader.init("dummy_vertex_shader.glsl", "patch_tess_control_shader.glsl", "de_boor_patch_tess_eval_shader.glsl", "height_map_fragment_shader.glsl");
		set.rational_20_param_patch_shader.init("dummy_vertex_shader.glsl", "rational_20_param_patch_tess_control_shader.glsl", "rational_20_param_patch_tess_eval_shader.glsl", "height_map_fragment_shader.glsl");
		set.simple_shader.init("simple_rect_vertex_shader.glsl", "line_fragment_shader.glsl");
		set.two_dim_shader.init("2d_vertex_shader.glsl", "wrap_around_parameters_geometry_shader.glsl", "line_fragment_shader.glsl");
		//set.workpiece_shader.init("workpiece_vertex_shader.glsl", "phong_fragment_shader.glsl");
		set.workpiece_shader.init("workpiece_vertex_shader_g.glsl", "workpiece_geometry_shader.glsl", "phong_fragment_shader.glsl");
		set.triangle_shader.init("triangle_vertex_shader.glsl", "phong_fragment_shader.glsl");

		return set;
	}

	ShaderLibrary::ShaderLibrary()
	{
	}

	void ShaderLibrary::init_shaders()
	{
		shader_sets.push_back(get_default_set());
		shader_sets.push_back(get_height_map_set()); // TODO optimize: OpenGL compiles many GLSL files more than once
	}

	void ShaderLibrary::init_locations()
	{
		auto default_set = get_shaders(ShaderSet::Type::Default);

		ul_pvm_location = default_set.line_shader.get_uniform_location("u_pvm");
		ul_color_location = default_set.line_shader.get_uniform_location("u_color");

		utl_pvm_location = default_set.trim_line_shader.get_uniform_location("u_pvm");
		utl_color_location = default_set.trim_line_shader.get_uniform_location("u_color");
		utl_trim_texture_location = default_set.trim_line_shader.get_uniform_location("u_trim_texture");

		up_pvm_location = default_set.point_shader.get_uniform_location("u_pvm");
		up_color_location = default_set.point_shader.get_uniform_location("u_color");

		uc_pvm_location = default_set.cursor_shader.get_uniform_location("u_pvm");

		ub_pvm_location = default_set.bezier_shader.get_uniform_location("u_pvm");
		ub_color_location = default_set.bezier_shader.get_uniform_location("u_color");
		ub_width_location = default_set.bezier_shader.get_uniform_location("u_width");
		ub_height_location = default_set.bezier_shader.get_uniform_location("u_height");

		upa_pvm_location = default_set.bezier_patch_shader.get_uniform_location("u_pvm");
		upa_color_location = default_set.bezier_patch_shader.get_uniform_location("u_color");
		upa_divisions_x_location = default_set.bezier_patch_shader.get_uniform_location("u_divisions_x");
		upa_divisions_y_location = default_set.bezier_patch_shader.get_uniform_location("u_divisions_y");
		upa_patches_x_location = default_set.bezier_patch_shader.get_uniform_location("u_patches_x");
		upa_patches_y_location = default_set.bezier_patch_shader.get_uniform_location("u_patches_y");
		upa_trim_texture_location = default_set.bezier_patch_shader.get_uniform_location("u_trim_texture");

		upb_pvm_location = default_set.de_boor_patch_shader.get_uniform_location("u_pvm");
		upb_color_location = default_set.de_boor_patch_shader.get_uniform_location("u_color");
		upb_divisions_x_location = default_set.de_boor_patch_shader.get_uniform_location("u_divisions_x");
		upb_divisions_y_location = default_set.de_boor_patch_shader.get_uniform_location("u_divisions_y");
		upb_patches_x_location = default_set.de_boor_patch_shader.get_uniform_location("u_patches_x");
		upb_patches_y_location = default_set.de_boor_patch_shader.get_uniform_location("u_patches_y");
		upb_trim_texture_location = default_set.de_boor_patch_shader.get_uniform_location("u_trim_texture");

		upr_pvm_location = default_set.rational_20_param_patch_shader.get_uniform_location("u_pvm");
		upr_color_location = default_set.rational_20_param_patch_shader.get_uniform_location("u_color");
		upr_divisions_x_location = default_set.rational_20_param_patch_shader.get_uniform_location("u_divisions_x");
		upr_divisions_y_location = default_set.rational_20_param_patch_shader.get_uniform_location("u_divisions_y");

		us_color_location = default_set.simple_shader.get_uniform_location("u_color");
		us_scale_location = default_set.simple_shader.get_uniform_location("u_scale");
		us_pos_location = default_set.simple_shader.get_uniform_location("u_pos");

		u2d_color_location = default_set.two_dim_shader.get_uniform_location("u_color");
		u2d_urange_location = default_set.two_dim_shader.get_uniform_location("u_urange");
		u2d_vrange_location = default_set.two_dim_shader.get_uniform_location("u_vrange");

		uw_pv_location = default_set.workpiece_shader.get_uniform_location("u_pv");
		uw_m_location = default_set.workpiece_shader.get_uniform_location("u_m");
		uw_size_location = default_set.workpiece_shader.get_uniform_location("u_size");
		uw_height_map_location = default_set.workpiece_shader.get_uniform_location("u_height_map");
		uw_color_location = default_set.workpiece_shader.get_uniform_location("u_color");
		uw_uv_offset_location = default_set.workpiece_shader.get_uniform_location("u_uv_offset");

		ut_pvm_location = default_set.triangle_shader.get_uniform_location("u_pvm");
		ut_m_location = default_set.triangle_shader.get_uniform_location("u_m");
		ut_color_location = default_set.triangle_shader.get_uniform_location("u_color");
	}
}
