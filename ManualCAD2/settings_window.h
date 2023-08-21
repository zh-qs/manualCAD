#pragma once

#include "window.h"
#include "parameter_space_view_window.h"
#include "raycaster.h"
#include "renderer.h"
#include "object.h"
#include "object_controller.h"
#include "cursor.h"
#include <string>
#include "message_box.h"
#include <optional>

namespace ManualCAD
{
	class ObjectSettingsWindow : public Window {
	public:
		ObjectController& controller;
		ParameterSpaceViewWindow& parameter_view;

		ObjectSettingsWindow(ObjectController& controller, ParameterSpaceViewWindow& parameter_view) : controller(controller), parameter_view(parameter_view) {}

		virtual void build() override {
			if (controller.is_preview_present())
				build_for_preview();
			else
				build_for_objects();
		}

		void build_for_objects()
		{
			ImGui::Begin("Selection", &visible, ImGuiWindowFlags_AlwaysAutoResize);
			const auto count = controller.selected_objects_count();
			ImGui::Text("%u object%s selected", count, count == 1 ? "" : "s");
			if (ImGui::Button("Deselect"))
				deselect();
			ImGui::SameLine();
			if (ImGui::Button("Remove"))
				remove_selected_objects();
			ImGui::SameLine();
			if (ImGui::Button("Clone"))
				clone_selected_objects_then_select_cloned();
			auto& current = controller.get_selected_objects();
			current.build_settings(*this);
			if (ImGui::IsKeyPressed(ImGuiKey_Delete))
				remove_selected_objects();
			ImGui::End();
			if (!visible)
				controller.clear_selection();
		}

		void build_for_preview()
		{
			ImGui::Begin("New object", &visible, ImGuiWindowFlags_AlwaysAutoResize);
			auto& preview = controller.get_preview();
			preview.build_settings(*this);
			ImGui::End();
			if (!visible)
				controller.unset_preview();
		}

		void remove_selected_objects() {
			controller.remove_selected_objects();
			visible = false;
		}

		void deselect() {
			controller.clear_selection();
			visible = false;
		}

		void clone_selected_objects_then_select_cloned() {
			std::vector<std::pair<Object*, int>> objects_to_select;
			for (auto* obj : controller.get_selected_objects())
			{
				auto cloned_handles = obj->clone();
				for (int i = 0; i < cloned_handles.size(); ++i)
				{
					objects_to_select.push_back({ cloned_handles[i].get(), controller.get_object_count() });
					controller.add_object(std::move(cloned_handles[i]));
				}
			}
			controller.clear_selection();
			for (auto& i : objects_to_select)
			{
				if (i.first->is_transformable())
					controller.add_to_selection(i.second);
			}
		}
	};

	template <class T, template <class P> class Container>
	std::optional<T*> try_create_object_from_points(ObjectController& controller, bool& selected_empty, bool& not_point)
	{
		auto& selected = controller.get_selected_objects();
		Container<Point*> points;
		if constexpr (std::is_same_v<Container<Point*>, std::vector<Point*>>)
		{
			points.reserve(selected.count());
		}

		if (selected.count() == 0) {
			selected_empty = true;
		}
		for (auto* obj : selected) {
			Point* ptr = dynamic_cast<Point*>(obj);
			if (ptr == nullptr) {
				not_point = true;
				break;
			}
			points.push_back(ptr);
		}
		if (!selected_empty && !not_point)
		{
			auto h = Object::create<T>(points);
			T* ptr = h.get();
			controller.add_object(std::move(h));
			return ptr;
		}
		return std::nullopt;
	}

	template <template <class P> class Container>
	bool try_fill_holes(ObjectController& controller, bool& selected_empty, bool& not_surface)
	{
		auto& selected = controller.get_selected_objects();
		Container<BicubicC0BezierSurface*> surfs;
		if constexpr (std::is_same_v<Container<BicubicC0BezierSurface*>, std::vector<BicubicC0BezierSurface*>>)
		{
			surfs.reserve(selected.count());
		}

		if (selected.count() == 0) {
			selected_empty = true;
		}
		for (auto* obj : selected) {
			BicubicC0BezierSurface* ptr = dynamic_cast<BicubicC0BezierSurface*>(obj);
			if (ptr == nullptr) {
				not_surface = true;
				break;
			}
			surfs.push_back(ptr);
		}
		if (!selected_empty && !not_surface)
		{
			auto l = Gregory20ParamSurface::fill_all_holes<Container>(surfs);
			if (l.empty())
				return false;
			for (auto&& h : l)
				controller.add_object(std::move(h));
			return true;
		}
		return false;
	}

	class ObjectControllerSettingsWindow : public Window {
		ObjectController& controller;
		Camera& camera;
		const Cursor& cursor;
		bool& object_settings_window_visible;

		bool show_points_on_list = true;

		float intersection_step_length = ApplicationSettings::INTERSECTION_STEP_DEFAULT_LENGTH;
		int intersection_max_steps = ApplicationSettings::INTERSECTION_DEFAULT_MAX_STEPS;
		bool intersection_cursor_hint = false;
		int intersection_sample_count = 10;

		void load_model_from_file();
		void save_model_to_file();

		std::pair<ParametricSurface*, ParametricSurface*> try_get_surfaces_from_selection(bool& invalid_selection_count, bool& not_surface);
		void try_add_intersection_curve_with_cursor_hint(ParametricSurface& surf1, ParametricSurface& surf2, bool& not_intersect, bool& timeout);
		void try_add_intersection_curve_without_hint(ParametricSurface& surf1, ParametricSurface& surf2, bool& not_intersect, bool& timeout);

		void try_add_self_intersection_curve_with_cursor_hint(ParametricSurface& surf, bool& not_intersect, bool& timeout);
		void try_add_self_intersection_curve_without_hint(ParametricSurface& surf, bool& not_intersect, bool& timeout);
	public:
		ObjectControllerSettingsWindow(ObjectController& controller, Camera& camera, const Cursor& cursor, WindowHandle& object_settings_window) : controller(controller), camera(camera), cursor(cursor), object_settings_window_visible(object_settings_window->visible) {}

		virtual void build() override {
			ImGui::Begin("Objects", NULL, ImGuiWindowFlags_MenuBar);
			ImGui::PushItemWidth(200);

			bool selected_empty = false, not_point = false, not_c0_surface = false, cycle_found = true,
				invalid_selection_count = false, not_param_surface = false, not_intersect = false,
				should_open_intersection_popup = false, timeout = false;

			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::MenuItem("New")) {
						if (check_and_confirm_unsaved()) {
							controller.clear_all();
							controller.set_saved();
						}
					}
					if (ImGui::MenuItem("Open")) {
						load_model_from_file();
					}
					if (ImGui::MenuItem("Save")) {
						save_model_to_file();
					}
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("New")) {
					if (ImGui::MenuItem("Torus")) {
						controller.add_object(Object::create_at_cursor<Torus>(cursor));
					}
					if (ImGui::MenuItem("Point")) {
						controller.add_object(Object::create_at_cursor<Point>(cursor));
					}
					if (ImGui::MenuItem("Bezier C0 curve")) {
						try_create_object_from_points<BezierC0Curve, std::list>(controller, selected_empty, not_point);
					}
					if (ImGui::MenuItem("Bezier C2 curve")) {
						auto result = try_create_object_from_points<BezierC2Curve, std::vector>(controller, selected_empty, not_point);
						if (result.has_value())
							controller.add_object(result.value()->make_linked_bernstein_points());
					}
					if (ImGui::MenuItem("Spline")) {
						try_create_object_from_points<InterpolationSpline, std::vector>(controller, selected_empty, not_point);
					}
					if (ImGui::MenuItem("Bezier C0 surface")) {
						controller.set_preview(Object::create<BicubicC0BezierSurfacePreview>(cursor.get_world_position()));
						object_settings_window_visible = true;
						//BicubicC0BezierPatch::create_and_add(controller, cursor.get_world_position(), 2, 1, 1.0f, 1.0f); // TODO
					}
					if (ImGui::MenuItem("Bezier C2 surface")) {
						controller.set_preview(Object::create<BicubicC2BezierSurfacePreview>(cursor.get_world_position()));
						object_settings_window_visible = true;
					}
					if (ImGui::MenuItem("Gregory fill-in")) {
						cycle_found = try_fill_holes<std::vector>(controller, selected_empty, not_c0_surface);
					}
					if (ImGui::MenuItem("Intersection curve")) {
						auto surfaces = try_get_surfaces_from_selection(invalid_selection_count, not_param_surface);
						if (surfaces.first != nullptr)
						{
							should_open_intersection_popup = true;
							//try_add_intersection_curve(controller, *surfaces.first, *surfaces.second, cursor, not_intersect);
						}
					}
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Options"))
				{
					if (ImGui::MenuItem("Show points", nullptr, show_points_on_list))
					{
						show_points_on_list = !show_points_on_list;
					}
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			if (should_open_intersection_popup)
				ImGui::OpenPopup("Intersection parameters");
			if (ImGui::BeginPopup("Intersection parameters"))
			{
				ImGui::SeparatorText("Intersection parameters");
				if (controller.selected_objects_count() == 1)
				{
					ImGui::Text("NOTE: Only one surface selected. This will find self-intersections.");
				}
				ImGui::SliderFloat("Step length", &intersection_step_length, 0.001f, 0.1f, nullptr); // TODO consider NoInput
				int max_steps = intersection_max_steps;
				if (ImGui::InputInt("Max steps", &max_steps, 1, 10))
				{
					if (max_steps >= 10 && max_steps <= 10000)
						intersection_max_steps = max_steps;
				}
				ImGui::Checkbox("Take cursor as a hint", &intersection_cursor_hint);
				if (!intersection_cursor_hint)
				{
					ImGui::SliderInt("Samples along constant parameter", &intersection_sample_count, 1, 15, nullptr, ImGuiSliderFlags_NoInput);
				}
				if (ImGui::Button("Create"))
				{
					// maybe redundant, same in line 205
					auto surfaces = try_get_surfaces_from_selection(invalid_selection_count, not_param_surface);
					if (surfaces.first != nullptr && surfaces.second != nullptr) // intersection of 2 surfaces
					{
						if (intersection_cursor_hint)
						{
							try_add_intersection_curve_with_cursor_hint(*surfaces.first, *surfaces.second, not_intersect, timeout);
						}
						else
						{
							try_add_intersection_curve_without_hint(*surfaces.first, *surfaces.second, not_intersect, timeout);
						}
					}
					else if (surfaces.first != nullptr) // self-intersection
					{
						if (intersection_cursor_hint)
						{
							try_add_self_intersection_curve_with_cursor_hint(*surfaces.first, not_intersect, timeout);
						}
						else
						{
							try_add_self_intersection_curve_without_hint(*surfaces.first, not_intersect, timeout);
						}
					}
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			if (selected_empty)
				MessageBox::show_popup("Error##1");
			else if (not_point)
				MessageBox::show_popup("Error##0");
			else if (not_c0_surface)
				MessageBox::show_popup("Error##2");
			else if (!cycle_found)
				MessageBox::show_popup("Error##3");
			else if (invalid_selection_count)
				MessageBox::show_popup("Error##4");
			else if (not_param_surface)
				MessageBox::show_popup("Error##5");
			else if (not_intersect)
				MessageBox::show_popup("Error##6");
			else if (timeout)
				MessageBox::show_popup("Error##7");
			MessageBox::register_popup_error("Error##0", "All selected objects must be points!");
			MessageBox::register_popup_error("Error##1", "No objects selected!");
			MessageBox::register_popup_error("Error##2", "All selected objects must be Bezier C0 surfaces!");
			MessageBox::register_popup_error("Error##3", "No cycle found!");
			MessageBox::register_popup_error("Error##4", "There should be 1 or 2 objects selected to intersect!");
			MessageBox::register_popup_error("Error##5", "All selected objects must be parametric surfaces!");
			MessageBox::register_popup_error("Error##6", "No intersection found! (maybe try moving the cursor)");
			MessageBox::register_popup_error("Error##7", "Computation timed out!");

			auto objects = controller.get_objects();

			int to_indent = 0;
			int to_skip = 0;

			for (int i = 0; i < objects.size(); ++i) {
				/*if (to_skip > 0) {
					--to_skip;
					continue;
				}*/
				if (objects[i].object->is_illusory()) continue;
				if (!show_points_on_list && objects[i].object->is_point()) continue;

				/*bool last_indent = false;
				if (to_indent > 0) {
					--to_indent;
					last_indent = to_indent == 0;
				}*/
				std::string name_with_tag = objects[i].object->name + "##" + std::to_string(i);

				/*const bool dropdown = objects[i].object->real_children_count() > 0;
				if (dropdown) {
					std::string tree_label = "##treenode" + std::to_string(i);
					bool selected = ImGui::TreeNode(tree_label.c_str());
					if (selected) {
						to_indent = objects[i].object->real_children_count();
					}
					else {
						to_skip = objects[i].object->real_children_count();
					}
					ImGui::SameLine();
				}*/

				if (ImGui::Selectable(name_with_tag.c_str(), objects[i].selected)) {
					if (ImGui::IsKeyDown(ApplicationSettings::ADD_TO_SELECTION_KEY)) {
						if (objects[i].selected)
							controller.remove_from_selection(i);
						else
							controller.add_to_selection(i);
					}
					else if (ImGui::IsKeyDown(ApplicationSettings::BIND_TO_SELECTED_KEY)) {
						if (!objects[i].selected)
							for (auto& info : objects) {
								if (info.selected)
									info.object->bind_with(*objects[i].object);
							}
					}
					else {
						controller.select_object(i);
					}
					object_settings_window_visible = controller.selected_objects_count() > 0;
				}

				/*if (last_indent) ImGui::TreePop();*/
			}

			ImGui::End();
		}

		bool check_and_confirm_unsaved();
	};

	class SceneSettingsWindow : public Window {
		Cursor& cursor;
		Renderer& renderer;
		ImVec4& clear_color;
		const int& width;
		const int& height;
	public:
		SceneSettingsWindow(Cursor& cursor, Renderer& renderer, ImVec4& clear_color, const int& width, const int& height) : cursor(cursor), renderer(renderer), clear_color(clear_color), width(width), height(height) {}

		virtual void build() override {
			Camera& camera = renderer.get_camera();
			Vector3 world_pos = cursor.get_world_position();
			Vector3 scr_pos = cursor.get_screen_position(camera, width, height);
			ImGui::Begin("Scene", NULL);
			if (ImGui::CollapsingHeader("Display")) {
				ImGui::Text("FPS: %f", roundf(1.0f / ImGui::GetIO().DeltaTime));
				ImGui::ColorEdit3("Background color", &clear_color.x, ImGuiColorEditFlags_Float);
			}
			if (ImGui::CollapsingHeader("Camera")) {
				float fov_deg = camera.fov_rad * 180.0f / PI;
				ImGui::SliderFloat("Near plane", &camera.near, 0.01f, 1.0f, NULL, ImGuiSliderFlags_NoInput);
				ImGui::SliderFloat("Far plane", &camera.far, 10.0f, 1000.0f, NULL, ImGuiSliderFlags_NoInput);
				if (ImGui::SliderFloat("FOV angle", &fov_deg, 10.0f, 170.0f, NULL, ImGuiSliderFlags_NoInput)) {
					camera.fov_rad = fov_deg * PI / 180.0f;
				}
				ImGui::SliderFloat("Distance from target", camera.get_screen_to_target_distance_handle(), 0.0f, 10.0f, NULL, ImGuiSliderFlags_NoInput);
				ImGui::Text("Current scale: %f", camera.get_scale());
				ImGui::SameLine();
				if (ImGui::Button("Reset scale"))
					camera.reset_scale();
			}
			if (ImGui::CollapsingHeader("Cursor")) {
				if (ImGui::InputFloat3("World position", world_pos.data())) {
					if (world_pos.is_valid())
						cursor.set_world_position(world_pos);
				}
				if (ImGui::InputFloat3("Screen position", scr_pos.data())) {
					//cursor.set_screen_position(scr_pos, camera, width, height);
					auto w = camera.screen_to_world(scr_pos, width, height);
					if (w.is_valid())
						cursor.set_world_position(w);
				}
				if (ImGui::Button("Look at cursor")) {
					camera.look_at(cursor.get_world_position());
				}
			}
			if (ImGui::CollapsingHeader("Stereoscopy")) {
				ImGui::Checkbox("Enable stereoscopic view", &renderer.stereoscopy_settings.enabled);
				ImGui::SeparatorText("Stereoscopic view settings");
				ImGui::SliderFloat("Interocular distance", &renderer.stereoscopy_settings.interocular_distance, 0.0f, 1.0f, NULL, ImGuiSliderFlags_NoInput);
				ImGui::SliderFloat("Focus plane", &renderer.stereoscopy_settings.focus_plane, 0.1f, 100.0f, NULL, ImGuiSliderFlags_NoInput);
				ImGui::SliderFloat("Saturation", &renderer.stereoscopy_settings.view_saturation, 0.0f, 1.0f, NULL, ImGuiSliderFlags_NoInput);
				if (ImGui::ColorEdit3("Left eye color", renderer.stereoscopy_settings.left_eye_color.data(), ImGuiColorEditFlags_Float)) {
					renderer.stereoscopy_settings.right_eye_color.x = 1.0f - renderer.stereoscopy_settings.left_eye_color.x;
					renderer.stereoscopy_settings.right_eye_color.y = 1.0f - renderer.stereoscopy_settings.left_eye_color.y;
					renderer.stereoscopy_settings.right_eye_color.z = 1.0f - renderer.stereoscopy_settings.left_eye_color.z;
				}
				if (ImGui::ColorEdit3("Right eye color", renderer.stereoscopy_settings.right_eye_color.data(), ImGuiColorEditFlags_Float)) {
					renderer.stereoscopy_settings.left_eye_color.x = 1.0f - renderer.stereoscopy_settings.right_eye_color.x;
					renderer.stereoscopy_settings.left_eye_color.y = 1.0f - renderer.stereoscopy_settings.right_eye_color.y;
					renderer.stereoscopy_settings.left_eye_color.z = 1.0f - renderer.stereoscopy_settings.right_eye_color.z;
				}
			}
			ImGui::End();
		}
	};

	class RendererSettingsWindow : public Window {
		Camera& camera;
	public:
		RendererSettingsWindow(Camera& camera) : camera(camera) {}

		virtual void build() override {
			ImGui::Begin("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize);
			if (ImGui::CollapsingHeader("Scene"))
			{
				ImGui::SliderFloat("Screen to target distance", camera.get_screen_to_target_distance_handle(), 0.01f, 20.0f, NULL, ImGuiSliderFlags_NoInput);
			}
			ImGui::End();
		}
	};

	class RaycasterSettingsWindow : public Window {
	public:
		RaycasterSettingsWindow(Raycaster& raycaster, Camera& camera, Ellipsoid& ellipsoid, int& downsampling_scale)
			: raycaster_ptr(&raycaster), camera_ptr(&camera), ellipsoid_ptr(&ellipsoid), downsampling_scale_ptr(&downsampling_scale) {
			a = ellipsoid.get_semiaxis(0);
			b = ellipsoid.get_semiaxis(1);
			c = ellipsoid.get_semiaxis(2);
		}

		virtual void build() override {
			ImGui::Begin("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize);
			if (ImGui::CollapsingHeader("Scene"))
			{
				resetting_slider_float("Light specular exponent", camera_ptr->get_specular_exponent_handle(), 1.0f, 100.0f);
				resetting_slider_float("Screen to target distance", camera_ptr->get_screen_to_target_distance_handle(), 0.01f, 20.0f);
			}
			if (ImGui::CollapsingHeader("Render"))
			{
				resetting_slider_int("Adaptive downsampling scale", downsampling_scale_ptr, 1, 64);
			}
			if (ImGui::CollapsingHeader("Ellipsoid"))
			{
				ImGui::Text("Semi-axes lengths:");
				if (resetting_slider_float("a", &a, 0.0f, 10.0f))
					ellipsoid_ptr->set_semiaxes(a, b, c);
				if (resetting_slider_float("b", &b, 0.0f, 10.0f))
					ellipsoid_ptr->set_semiaxes(a, b, c);
				if (resetting_slider_float("c", &c, 0.0f, 10.0f))
					ellipsoid_ptr->set_semiaxes(a, b, c);
			}
			ImGui::End();
		}
	private:
		Raycaster* raycaster_ptr;
		Camera* camera_ptr;
		Ellipsoid* ellipsoid_ptr;
		int* downsampling_scale_ptr;

		float a, b, c;

		bool resetting_slider_float(const char* label, float* v, float v_min, float v_max) {
			bool changed = ImGui::SliderFloat(label, v, v_min, v_max, NULL, ImGuiSliderFlags_NoInput);
			if (changed)
				raycaster_ptr->reset_downsampling();
			return changed;
		}

		bool resetting_slider_int(const char* label, int* v, int v_min, int v_max) {
			bool changed = ImGui::SliderInt(label, v, v_min, v_max, NULL, ImGuiSliderFlags_NoInput);
			if (changed)
				raycaster_ptr->reset_downsampling();
			return changed;
		}
	};
}