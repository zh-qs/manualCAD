#include "object_settings.h"
#include <imgui.h>
#include <imgui_internal.h>
#include "settings_window.h"
#include "milling_program.h"
#include <cstring>
#include "logger.h"
#include "system_dialog.h"
#include "height_map_renderer.h"
#include "plane_xz.h"

namespace ManualCAD
{
	template <class O, template <class T> class Container>
	void build_objects_list(const char* name, Prototype& prototype, Container<const O*>& points) {
		if (ImGui::BeginListBox(name)) {
			int idx = 0;
			for (auto it = points.begin(); it != points.end();) {
				ImGui::Text("%s", (*it)->name.c_str());
				std::string tag = "##" + std::to_string(idx++);				

				ImGui::SameLine();
				std::string label_with_tag = "X" + tag;
				if (prototype.invalidate_object_and_box_if(ImGui::Button(label_with_tag.c_str()))) {
					it = points.erase(it);
				}
				else it++;
			}

			ImGui::EndListBox();
		}
	}

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

	void build_nurbs_point_list(Object& object, std::vector<Point*>& points, std::vector<float>& weights) {
		if (ImGui::BeginListBox("Point weights")) {
			for (int i = 0; i < points.size(); ++i) {
				//ImGui::Text("%s", points[i].name.c_str());
				std::string tag = "##" + std::to_string(i);
				std::string label = points[i]->name + tag;

				object.invalidate_if(ImGui::SliderFloat(label.c_str(), &weights[i], 0.01, 10, NULL, ImGuiSliderFlags_NoInput));
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

		//ImGui::Text("Persistence: %d", object.persistence);
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

	void ObjectSettings::build_bicubic_c2_nurbs_surface_settings(BicubicC2NURBSSurface& surf, ObjectSettingsWindow& parent)
	{
		int s_div_x = surf.surf.divisions_x, s_div_y = surf.surf.divisions_y;
		ImGui::SeparatorText("NURBS C2 surface");
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

		build_nurbs_point_list(surf, surf.points, surf.weights);
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

	void ObjectSettings::build_bicubic_c2_nurbs_surface_preview_settings(BicubicC2NURBSSurfacePreview& preview, ObjectSettingsWindow& parent)
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
			BicubicC2NURBSSurface* patch;
			if (preview.cylinder)
				patch = &BicubicC2NURBSSurface::create_cylinder_and_add(parent.controller, preview.position, preview.patches_x, preview.patches_y, preview.radius, preview.width_y / patches_y);
			else
				patch = &BicubicC2NURBSSurface::create_and_add(parent.controller, preview.position, preview.patches_x, preview.patches_y, preview.width_x / patches_x, preview.width_y / patches_y);
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

	void ObjectSettings::build_parametric_surface_settings(ParametricSurfaceObject& surf, ObjectSettingsWindow& parent)
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

	void ObjectSettings::build_prototype_settings(Prototype& prototype, ObjectSettingsWindow& parent)
	{
		ImGui::SeparatorText("Prototype");
		prototype.invalidate_if(ImGui::SliderFloat3("Size", prototype.size.data(), 1.0f, 25.0f, NULL, ImGuiSliderFlags_NoInput));
		prototype.invalidate_if(ImGui::SliderFloat("Model height", &prototype.mill_height, 0.0f, prototype.size.y, NULL, ImGuiSliderFlags_NoInput));
		prototype.invalidate_if(ImGui::SliderFloat("Offset", &prototype.offset, 0.0f, 0.4f * std::min(prototype.size.x, prototype.size.z), NULL, ImGuiSliderFlags_NoInput));
		build_objects_list<ParametricSurfaceObject, std::list>("Surfaces", prototype, prototype.surfaces);

		ImGui::SeparatorText("Generate program");
		const char* items[] = {"Rough", "Flat plane", "Envelope", "Detailed", "Signature"};
		static int item_current;
		ImGui::Combo("Program type", &item_current, items, IM_ARRAYSIZE(items));
		const char* cutters[] = { "K16", "K08", "K01", "F12", "F10" };
		static int cutter_current;
		ImGui::Combo("Cutter type", &cutter_current, cutters, IM_ARRAYSIZE(cutters));

		if (static_cast<Prototype::ProgramType>(item_current) == Prototype::ProgramType::Signature)
		{
			build_objects_list<ParametricCurveObject, std::list>("Signature curves", prototype, prototype.signature_curves);
		}

		if (ImGui::Button("Generate"))
		{
			std::unique_ptr<Cutter> cutter;
			if (cutter_current > 2)
				cutter = std::make_unique<FlatCutter>(cutter_current == 3 ? 1.2f : 1.0f);
			else
				cutter = std::make_unique<BallCutter>(cutter_current == 0 ? 1.6f : (cutter_current == 1 ? 0.8f : 0.1f));

			prototype.generate_program(static_cast<Prototype::ProgramType>(item_current), std::move(cutter));
		}

		if (prototype.generated_program.has_value())
		{
			ImGui::SeparatorText("Milling program");
			auto& program = prototype.generated_program.value();
			ImGui::Text("Name: %s", program.get_name().c_str());
			ImGui::SeparatorText("Cutter");
			ImGui::Text("Diameter: %.1f mm", program.cutter->get_diameter() * 10.0f);
			ImGui::Text("Type: %s", program.cutter->get_type());

			if (ImGui::Button("Save"))
			{
				std::string filename;
				try
				{
					filename = SystemDialog::save_file_dialog("Save", { {"*.k??,*.f??", nullptr} });
				}
				catch (const std::exception& e)
				{
					Logger::log_error("[ERROR] Saving program file: %s\n", e.what());
				}
				if (!filename.empty())
				{
					int diameter_i = static_cast<int>(10.0f * program.cutter->get_diameter());
					std::string diameter_str = (diameter_i < 10 ? "0" : "") + std::to_string(diameter_i);
					std::string extension = '.' + (program.cutter->get_type_char() + diameter_str);
					if (filename.substr(filename.size() - 4, 4) != extension)
						filename += extension;
					try {
						program.save_to_file(filename.c_str());
					}
					catch (std::runtime_error& e)
					{
						Logger::log_error("[ERROR] %s\n", e.what());
					}
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear"))
				prototype.remove_program();
		}

		ImGui::SeparatorText("Experimental");
		if (ImGui::Button("Build and show envelope"))
		{
			prototype.show_envelope_experimental();
		}
		static int tex_idx = -1;
		if (ImGui::Button("Render index map (memory unsafe!)"))
		{
			const Vector2 min = { prototype.view_boundary_points[0].x, prototype.view_boundary_points[0].z },
				max = { prototype.view_boundary_points[2].x, prototype.view_boundary_points[2].z };
			const float height = prototype.view_boundary_points[0].y;
			PlaneXZ plane{ min, max, height };
			Box box = plane.get_bounding_box();
			box.y_max += prototype.mill_height * prototype.scale;
			Vector3 map_size = { box.x_max - box.x_min, box.y_max - box.y_min,box.z_max - box.z_min };
			auto r = HeightMapRenderer{ prototype.surfaces, box, prototype.scale * 0.5f * 0.8f }; // 0.8f : for K08 mill
			r.render_index_map(map_size);
			tex_idx = r.get_texture().get_id();
		}
		if (tex_idx > 0)
		{
			ImGui::Image((void*)(intptr_t)tex_idx, { 2000, 2000 });
		}
	}
	void ObjectSettings::build_ellipsoid_settings(Ellipsoid& ellipsoid, ObjectSettingsWindow& parent)
	{
		ImGui::SeparatorText("Ellipsoid");

		float a = ellipsoid.get_semiaxis(0),
			b = ellipsoid.get_semiaxis(1),
			c = ellipsoid.get_semiaxis(2);

		ImGui::Text("Semi-axes lengths:");
		if (ImGui::SliderFloat("a", &a, 0.0f, 10.0f, NULL, ImGuiSliderFlags_NoInput)) {
			ellipsoid.set_semiaxes(a, b, c);
			ellipsoid.raycastable.reset_downsampling();
		}
		if (ImGui::SliderFloat("b", &b, 0.0f, 10.0f, NULL, ImGuiSliderFlags_NoInput)) {
			ellipsoid.set_semiaxes(a, b, c);
			ellipsoid.raycastable.reset_downsampling();
		}
		if (ImGui::SliderFloat("c", &c, 0.0f, 10.0f, NULL, ImGuiSliderFlags_NoInput)) {
			ellipsoid.set_semiaxes(a, b, c);
			ellipsoid.raycastable.reset_downsampling();
		}

		if (ImGui::SliderFloat("Light specular exponent", &ellipsoid.raycastable.specular_exponent, 1.0f, 100.0f, NULL, ImGuiSliderFlags_NoInput)) {
			ellipsoid.raycastable.reset_downsampling();
		}
		if (ImGui::SliderInt("Adaptive downsampling scale", &ellipsoid.raycastable.downsampling_scale, 1, 64, NULL, ImGuiSliderFlags_NoInput)) {
			ellipsoid.raycastable.reset_downsampling();
		}
	}
}