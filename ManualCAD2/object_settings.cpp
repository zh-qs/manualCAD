#include "object_settings.h"
#include <imgui.h>
#include <imgui_internal.h>
#include "settings_window.h"
#include "milling_program.h"
#include <cstring>
#include "logger.h"
#include "system_dialog.h"

namespace ManualCAD
{
	template <class Curve, template <class T> class Container>
	void build_points_list(Curve& curve, Container<Point*>& points) {
		if (ImGui::BeginListBox("Points")) {
			int idx = 0;
			for (auto it = points.begin(); it != points.end();) {
				ImGui::Text("%s", (*it)->name.c_str());
				std::string tag = "##" + std::to_string(idx++);

				ImGui::SameLine();
				std::string label_with_tag = "/\\" + tag;
				if (it == points.begin())
				{
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}
				if (curve.invalidate_if(ImGui::Button(label_with_tag.c_str()))) {
					auto** pp = &*it;
					auto* p = *pp;
					it--;
					*pp = *it;
					*it = p;
					it++;
				}
				if (it == points.begin())
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}

				ImGui::SameLine();
				label_with_tag = "\\/" + tag;
				it++;
				if (it == points.end())
				{
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}
				if (curve.invalidate_if(ImGui::Button(label_with_tag.c_str()))) {
					auto** pp = &*it;
					auto* p = *pp;
					it--;
					*pp = *it;
					*it = p;
					it++;
				}
				if (it == points.end())
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}
				it--;

				ImGui::SameLine();
				label_with_tag = "X" + tag;
				if (curve.invalidate_if(ImGui::Button(label_with_tag.c_str()))) {
					it = points.erase(it);
				}
				else it++;
			}

			ImGui::EndListBox();
		}
	}

	void ObjectSettings::build_general_settings(Object& object) {
		constexpr int INPUT_BUF_SIZE = 100;
		char input_buf[INPUT_BUF_SIZE];
		strncpy_s(input_buf, object.name.c_str(), INPUT_BUF_SIZE);
		if (object.is_transformable())
		{
			ImGui::InputFloat3("Scale", object.transformation.scale.data());
			ImGui::InputFloat("Rotation angle", &object.transformation.rotation_angle_deg);
			ImGui::InputFloat3("Rotation axis", object.transformation.rotation_vector.data());
		}
		if (ImGui::ColorEdit4("Color", object.color.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreview))
		{
			object.renderable.color = object.color;
		}
		if (!object.is_illusory() && ImGui::InputText("Name", input_buf, INPUT_BUF_SIZE, ImGuiInputTextFlags_AutoSelectAll)) {
			object.name.assign(input_buf);
		}

		//ImGui::SeparatorText("Observers");
		//for (const auto* o : object.observers)
		//	ImGui::Text("%s", o->name.c_str());
	}

	void ObjectSettings::build_collection_settings(ObjectCollection& collection, ObjectSettingsWindow& parent)
	{
		if (collection.objects.empty()) return;

		bool transformable = collection.is_transformable();

		Vector3 translation = collection.position + collection.get_transformation().position;
		ImGui::SeparatorText("General");
		if (transformable && ImGui::InputFloat3("Position", translation.data())) {
			collection.set_world_position(translation); // we can't update collection.position directly, because set_world_position translates all objects in collection
		}

		if (collection.objects.size() == 1) {
			collection.objects[0]->build_settings(parent);
			return;
		}

		Vector3 scale = collection.get_transformation().scale;
		Vector3 rotation_axis = collection.get_transformation().rotation_vector;
		float rotation_angle = collection.get_transformation().rotation_angle_deg;

		if (!transformable) return;

		if (ImGui::InputFloat3("Scale", scale.data())) {
			collection.set_scale(scale);
		}
		if (ImGui::InputFloat("Rotation angle", &rotation_angle)) {
			collection.set_rotation(rotation_axis, rotation_angle);
		}
		if (ImGui::InputFloat3("Rotation axis", rotation_axis.data())) {
			collection.set_rotation(rotation_axis, rotation_angle);
		}

		if (collection.objects.size() == 2 && collection.objects[0]->is_point() && collection.objects[1]->is_point())
		{
			if (ImGui::Button("Merge points")) {
				parent.controller.merge_points(*dynamic_cast<Point*>(collection.objects[0]), *dynamic_cast<Point*>(collection.objects[1]));
				parent.visible = false;
			}
		}
	}

	void ObjectSettings::build_torus_settings(Torus& torus, ObjectSettingsWindow& parent)
	{
		int s_div_x = torus.divisions_x, s_div_y = torus.divisions_y;
		float r = torus.small_radius, R = torus.large_radius;
		ImGui::SeparatorText("Torus");
		bool r_changed = torus.invalidate_if(ImGui::SliderFloat("Small radius", &r, 0.0f, 10.0f, NULL, ImGuiSliderFlags_NoInput));
		bool R_changed = torus.invalidate_if(ImGui::SliderFloat("Large radius", &R, 0.0f, 10.0f, NULL, ImGuiSliderFlags_NoInput));
		if (r_changed || R_changed) {
			if (r < R) {
				torus.small_radius = r;
				torus.large_radius = R;
			}
		}
		torus.invalidate_if(ImGui::InputInt("Small circle divisions", &s_div_x));
		torus.invalidate_if(ImGui::InputInt("Large circle divisions", &s_div_y));

		if (((unsigned long long)s_div_x) * s_div_y <= 1000000ULL) // prevent app hanging on too many vertices
		{
			if (s_div_x > 2)
				torus.divisions_x = s_div_x;
			if (s_div_y > 2)
				torus.divisions_y = s_div_y;
		}
	}

	void ObjectSettings::build_bezier_c0_curve_settings(BezierC0Curve& curve, ObjectSettingsWindow& parent)
	{
		ImGui::SeparatorText("Bezier C0 curve");
		if (ImGui::ColorEdit4("Polyline color", curve.polyline_color.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreview))
		{
			curve.curve.polyline_color = curve.polyline_color;
		}
		curve.invalidate_if(ImGui::Checkbox("Polyline visible", &curve.polyline_visible));
		curve.invalidate_if(ImGui::Checkbox("Curve visible", &curve.curve_visible));

		if (ImGui::Button("Select all points"))
		{
			//parent.controller.select_all_from<Point, std::vector>(surf.points);
			parent.controller.select_observing(curve);
		}

		build_points_list<BezierC0Curve, std::list>(curve, curve.points);
	}

	void ObjectSettings::build_bezier_c2_curve_settings(BezierC2Curve& curve, ObjectSettingsWindow& parent)
	{
		ImGui::SeparatorText("Bezier C2 curve");
		if (ImGui::ColorEdit4("Polyline color", curve.polyline_color.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreview))
		{
			curve.curve.polyline_color = curve.polyline_color;
		}
		//curve.invalidate_if(ImGui::Checkbox("De Boor polyline visible", &curve.boor_polyline_visible));
		curve.invalidate_if(ImGui::Checkbox("Bezier polyline visible", &curve.polyline_visible));
		curve.invalidate_if(ImGui::Checkbox("Curve visible", &curve.curve_visible));
		ImGui::Checkbox("Edit Bernstein points", &curve.bernstein_points->renderable.visible);

		if (curve.bernstein_points->renderable.visible)
			ImGui::Combo("De Boor points behaviour", (int*)&curve.behaviour, "RotateAroundCenter\0MoveAdjacent\0\0");
		if (curve.points.size() < 4) {
			const size_t remaining = 4 - curve.points.size();
			ImGui::Text("NOTE: Insert %u more point%sto see the curve", remaining, remaining == 1 ? " " : "s ");
		}

		if (ImGui::Button("Select all points"))
		{
			//parent.controller.select_all_from<Point, std::vector>(surf.points);
			parent.controller.select_observing(curve);
		}

		build_points_list<BezierC2Curve, std::vector>(curve, curve.points);
	}

	void ObjectSettings::build_interpolation_spline_curve_settings(InterpolationSpline& curve, ObjectSettingsWindow& parent)
	{
		ImGui::SeparatorText("Spline");
		if (ImGui::ColorEdit4("Polyline color", curve.polyline_color.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreview))
		{
			curve.curve.polyline_color = curve.polyline_color;
		}
		curve.invalidate_if(ImGui::Checkbox("Polyline visible", &curve.polyline_visible));
		curve.invalidate_if(ImGui::Checkbox("Curve visible", &curve.curve_visible));

		if (ImGui::Button("Select all points"))
		{
			//parent.controller.select_all_from<Point, std::vector>(surf.points);
			parent.controller.select_observing(curve);
		}

		build_points_list<InterpolationSpline, std::vector>(curve, curve.points);
	}

	void ObjectSettings::build_bicubic_c0_bezier_surface_settings(BicubicC0BezierSurface& surf, ObjectSettingsWindow& parent)
	{
		int s_div_x = surf.surf.divisions_x, s_div_y = surf.surf.divisions_y;
		ImGui::SeparatorText("Bezier C0 surface");
		if (ImGui::ColorEdit4("Grid color", surf.contour_color.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreview))
		{
			surf.surf.contour_color = surf.contour_color;
		}
		surf.invalidate_if(ImGui::Checkbox("Grid visible", &surf.contour_visible));
		surf.invalidate_if(ImGui::Checkbox("Patch visible", &surf.patch_visible));

		bool changed_x = ImGui::InputInt("X divisions", &s_div_x);
		bool changed_y = ImGui::InputInt("Z divisions", &s_div_y);

		if (changed_x || changed_y)
		{
			if (s_div_x > 2 && s_div_x <= 64)
			{
				surf.surf.divisions_x = s_div_x;
				surf.invalidate();
			}
			if (s_div_y > 2 && s_div_y <= 64)
			{
				surf.surf.divisions_y = s_div_y;
				surf.invalidate();
			}
		}
		if (ImGui::Button("Select all points"))
		{
			parent.controller.select_observing(surf);
			//parent.controller.select_all_from<Point, std::vector>(surf.points);
		}
	}

	void ObjectSettings::build_bicubic_c2_bezier_surface_settings(BicubicC2BezierSurface& surf, ObjectSettingsWindow& parent)
	{
		int s_div_x = surf.surf.divisions_x, s_div_y = surf.surf.divisions_y;
		ImGui::SeparatorText("Bezier C2 surface");
		if (ImGui::ColorEdit4("Grid color", surf.contour_color.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreview))
		{
			surf.surf.contour_color = surf.contour_color;
		}
		surf.invalidate_if(ImGui::Checkbox("Grid visible", &surf.contour_visible));
		surf.invalidate_if(ImGui::Checkbox("Patch visible", &surf.patch_visible));

		bool changed_x = ImGui::InputInt("X divisions", &s_div_x);
		bool changed_y = ImGui::InputInt("Z divisions", &s_div_y);

		if (changed_x || changed_y)
		{
			if (s_div_x > 2 && s_div_x <= 64)
			{
				surf.surf.divisions_x = s_div_x;
				surf.invalidate();
			}
			if (s_div_y > 2 && s_div_y <= 64)
			{
				surf.surf.divisions_y = s_div_y;
				surf.invalidate();
			}
		}
		if (ImGui::Button("Select all points"))
		{
			//parent.controller.select_all_from<Point, std::vector>(surf.points);
			parent.controller.select_observing(surf);
		}
	}

	void ObjectSettings::build_bicubic_c0_bezier_surface_preview_settings(BicubicC0BezierSurfacePreview& preview, ObjectSettingsWindow& parent)
	{
		int patches_x = preview.patches_x, patches_y = preview.patches_y;
		ImGui::SeparatorText("Bezier C0 patch");
		bool changed_x = ImGui::InputInt("X patch count", &patches_x);
		bool changed_y = ImGui::InputInt("Z patch count", &patches_y);

		const int min_patches_x = preview.cylinder ? 2 : 1;

		if ((changed_x || changed_y) && patches_x >= min_patches_x && patches_y > 0)
		{
			preview.patches_x = patches_x;
			preview.patches_y = patches_y;
			preview.invalidate();
		}

		if (preview.invalidate_if(ImGui::Checkbox("Cylinder (join edges)", &preview.cylinder))) {
			if (preview.patches_x < 2)
				preview.patches_x = 2;
		}

		if (preview.cylinder)
		{
			preview.invalidate_if(ImGui::SliderFloat("Radius", &preview.radius, 0.001f, 10.0f));
			preview.invalidate_if(ImGui::SliderFloat("Height", &preview.width_y, 0.001f, 10.0f));
		}
		else
		{
			preview.invalidate_if(ImGui::SliderFloat("Width", &preview.width_x, 0.001f, 10.0f));
			preview.invalidate_if(ImGui::SliderFloat("Depth", &preview.width_y, 0.001f, 10.0f));
		}

		if (ImGui::Button("Build"))
		{
			parent.controller.unset_preview();
			parent.visible = false;
			BicubicC0BezierSurface* patch;
			if (preview.cylinder)
				patch = &BicubicC0BezierSurface::create_cylinder_and_add(parent.controller, preview.position, preview.patches_x, preview.patches_y, preview.radius, preview.width_y / (3 * patches_y));
			else
				patch = &BicubicC0BezierSurface::create_and_add(parent.controller, preview.position, preview.patches_x, preview.patches_y, preview.width_x / (3 * patches_x), preview.width_y / (3 * patches_y));
			patch->color = preview.color;
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			parent.controller.unset_preview();
			parent.visible = false;
		}
	}

	void ObjectSettings::build_bicubic_c2_bezier_surface_preview_settings(BicubicC2BezierSurfacePreview& preview, ObjectSettingsWindow& parent)
	{
		int patches_x = preview.patches_x, patches_y = preview.patches_y;
		ImGui::SeparatorText("Bezier C2 patch");
		bool changed_x = ImGui::InputInt("X patch count", &patches_x);
		bool changed_y = ImGui::InputInt("Z patch count", &patches_y);

		const int min_patches_x = preview.cylinder ? 3 : 1;

		if ((changed_x || changed_y) && patches_x >= min_patches_x && patches_y > 0)
		{
			preview.patches_x = patches_x;
			preview.patches_y = patches_y;
			preview.invalidate();
		}

		if (preview.invalidate_if(ImGui::Checkbox("Cylinder (join edges)", &preview.cylinder))) {
			if (preview.patches_x < 3)
				preview.patches_x = 3;
		}

		if (preview.cylinder)
		{
			preview.invalidate_if(ImGui::SliderFloat("Radius", &preview.radius, 0.001f, 10.0f));
			preview.invalidate_if(ImGui::SliderFloat("Height", &preview.width_y, 0.001f, 10.0f));
		}
		else
		{
			preview.invalidate_if(ImGui::SliderFloat("Width", &preview.width_x, 0.001f, 10.0f));
			preview.invalidate_if(ImGui::SliderFloat("Depth", &preview.width_y, 0.001f, 10.0f));
		}

		if (ImGui::Button("Build"))
		{
			parent.controller.unset_preview();
			parent.visible = false;
			BicubicC2BezierSurface* patch;
			if (preview.cylinder)
				patch = &BicubicC2BezierSurface::create_cylinder_and_add(parent.controller, preview.position, preview.patches_x, preview.patches_y, preview.radius, preview.width_y / patches_y);
			else
				patch = &BicubicC2BezierSurface::create_and_add(parent.controller, preview.position, preview.patches_x, preview.patches_y, preview.width_x / patches_x, preview.width_y / patches_y);
			patch->color = preview.color;
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			parent.controller.unset_preview();
			parent.visible = false;
		}
	}
	void ObjectSettings::build_gregory_20_param_surface_settings(Gregory20ParamSurface& surf, ObjectSettingsWindow& parent)
	{
		int s_div_x = surf.surf.divisions_x, s_div_y = surf.surf.divisions_y;
		ImGui::SeparatorText("Gregory fill-in");
		if (ImGui::ColorEdit4("Grid color", surf.contour_color.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaPreview))
		{
			surf.surf.contour_color = surf.contour_color;
		}
		surf.invalidate_if(ImGui::Checkbox("Grid visible", &surf.contour_visible));
		surf.invalidate_if(ImGui::Checkbox("Patch visible", &surf.patch_visible));

		bool changed_x = ImGui::InputInt("X divisions", &s_div_x);
		bool changed_y = ImGui::InputInt("Z divisions", &s_div_y);

		if (changed_x || changed_y)
		{
			if (s_div_x > 2 && s_div_x <= 64)
			{
				surf.surf.divisions_x = s_div_x;
				surf.invalidate();
			}
			if (s_div_y > 2 && s_div_y <= 64)
			{
				surf.surf.divisions_y = s_div_y;
				surf.invalidate();
			}
		}
	}

	void ObjectSettings::build_intersection_curve_settings(IntersectionCurve& curve, ObjectSettingsWindow& parent)
	{
		ImGui::SeparatorText("Intersection curve");
		ImGui::Text("Point count: %u%s", curve.line.get_point_count(), curve.line.looped ? ", looped" : "");
		if (curve.singular_crossed)
			ImGui::Text("NOTE: The curve passes through a singular point. There is probably another intersection nearby.");
		if (curve.too_short)
			ImGui::Text("NOTE: The curve has probably been truncated by max steps parameter");
		if (ImGui::Button("Convert to spline"))
		{
			auto spline_data = curve.to_spline();
			parent.controller.add_objects(spline_data.first);
			parent.controller.add_object(std::move(spline_data.second));
			parent.controller.clear_selection();
			parent.visible = false;
		}
	}

	void ObjectSettings::build_parametric_surface_settings(ParametricSurface& surf, ObjectSettingsWindow& parent)
	{
		if (ImGui::Button("Show parameter space"))
		{
			parent.parameter_view.show_surface(surf);
		}
	}

	void ObjectSettings::build_workpiece_settings(Workpiece& workpiece, ObjectSettingsWindow& parent)
	{
		ImGui::SeparatorText("Workpiece");
		ImGui::BeginDisabled(!workpiece.can_execute_milling_program());
		ImGui::Text("Divisions");
		ImGui::SameLine();
		if (ImGui::SliderInt("X", &workpiece.divisions_x, 100, 2000, NULL, ImGuiSliderFlags_NoInput))
		{
			workpiece.recreate_height_map();
			workpiece.invalidate();
		}
		ImGui::SameLine();
		if (ImGui::SliderInt("Y", &workpiece.divisions_y, 100, 2000, NULL, ImGuiSliderFlags_NoInput))
		{
			workpiece.recreate_height_map();
			workpiece.invalidate();
		}
		if (ImGui::SliderFloat3("Size", workpiece.size.data(), 1.0f, 25.0f, NULL, ImGuiSliderFlags_NoInput))
		{
			workpiece.height_map.fill(workpiece.size);
			workpiece.invalidate();
		}
		ImGui::SliderFloat("Max cutter depth", &workpiece.max_cutter_depth, 1.0f, 15.0f, NULL, ImGuiSliderFlags_NoInput);
		ImGui::EndDisabled();
		if (!workpiece.can_execute_milling_program())
			ImGui::Text("Parameters can't be edited during a simulation!");
		ImGui::SeparatorText("Milling program");
		if (ImGui::Button("Load program"))
		{
			std::string filename;
			try
			{
				filename = SystemDialog::open_file_dialog("Open", { {"*.k??,*.f??", nullptr} });
			}
			catch (const std::exception& e)
			{
				Logger::log_error("[ERROR] Opening program file: %s\n", e.what());
			}
			if (!filename.empty())
			{
				try {
					workpiece.set_milling_program(MillingProgram::read_from_file(filename.c_str()));
				}
				catch (std::runtime_error& e)
				{
					Logger::log_error("[ERROR] %s\n", e.what());
				}
			}
		}
		if (workpiece.has_milling_program())
		{
			auto& program = workpiece.get_milling_program();
			ImGui::Text("%s loaded", program.get_name().c_str());
			ImGui::Checkbox("Path visible", &workpiece.path.visible);
			ImGui::Checkbox("Cutter visible", &workpiece.cylinder.visible);
			ImGui::SeparatorText("Cutter");
			ImGui::SliderFloat("Speed", &program.cutter_speed, 1.0f, 100.0f, NULL, ImGuiSliderFlags_NoInput);
			ImGui::SliderFloat("Cutting part height", &program.cutter->cutting_part_height, 1.0f, 10.0f, NULL, ImGuiSliderFlags_NoInput);
			ImGui::Text("Diameter: %.1f mm", program.cutter->get_diameter() * 10.0f);
			ImGui::Text("Type: %s", program.cutter->get_type());

			ImGui::BeginDisabled(!workpiece.can_execute_milling_program());
			if (ImGui::Button("Animate"))
				workpiece.animate_milling_program();
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Immediate"))
				workpiece.execute_milling_program_immediately();
		}
	}
}