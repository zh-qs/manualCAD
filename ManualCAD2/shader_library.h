#pragma once

#include "shader.h"
#include <vector>
#include <stdexcept>

namespace ManualCAD
{
	struct ShaderSet {
		Shader line_shader;
		Shader trim_line_shader;
		Shader point_shader;
		Shader cursor_shader;
		Shader bezier_shader;
		Shader bezier_patch_shader;
		Shader de_boor_patch_shader;
		Shader rational_20_param_patch_shader;
		Shader simple_shader;
		Shader two_dim_shader;
		Shader workpiece_shader;
		Shader triangle_shader;

		enum class Type {
			Default, HeightMap
		};
	};

	class ShaderLibrary {
		std::vector<ShaderSet> shader_sets;

		bool initialized = false;

		void init_shaders();
		void init_locations();
		ShaderLibrary();

		static ShaderLibrary library;
	public:
		// locations common for all shaders
		GLint ul_pvm_location, utl_pvm_location, up_pvm_location, uc_pvm_location, ub_pvm_location, upa_pvm_location, upb_pvm_location, upr_pvm_location, ut_pvm_location;
		GLint uw_pv_location, uw_m_location, ut_m_location;
		GLint ul_color_location, utl_color_location, up_color_location, ub_color_location, upa_color_location, upb_color_location, upr_color_location, us_color_location, u2d_color_location, uw_color_location, ut_color_location;
		GLint ub_width_location;
		GLint ub_height_location;
		GLint upa_divisions_x_location, upb_divisions_x_location, upr_divisions_x_location;
		GLint upa_divisions_y_location, upb_divisions_y_location, upr_divisions_y_location;
		GLint us_scale_location;
		GLint us_pos_location;
		GLint u2d_urange_location;
		GLint u2d_vrange_location;
		GLint utl_trim_texture_location, upa_trim_texture_location, upb_trim_texture_location;
		GLint upa_patches_x_location, upb_patches_x_location;
		GLint upa_patches_y_location, upb_patches_y_location;
		GLint uw_size_location;
		GLint uw_height_map_location;
		GLint uw_uv_offset_location;

		const ShaderSet& get_shaders(ShaderSet::Type type) const { return shader_sets[static_cast<int>(type)]; }

		static void init() {
			if (!library.initialized)
			{
				// this order is mandatory!
				library.init_shaders();
				library.init_locations();

				library.initialized = true;
			}
		}
		static const ShaderLibrary& get() {
			if (!library.initialized)
				throw std::runtime_error("You must call init() on ShaderLibrary before first use!");

			return library;
		}
	};
}