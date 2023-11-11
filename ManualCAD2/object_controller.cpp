#include "object_controller.h"
#include "settings_window.h"
#include "application_settings.h"
#include "message_box.h"

namespace ManualCAD
{
	std::vector<ObjectInfo> ObjectController::get_objects()
	{
		auto it = selected_indices.cbegin();
		std::vector<ObjectInfo> res(objects.size() + (preview_present ? 1 : 0));
		for (int i = 0; i < objects.size(); ++i) {
			if (it != selected_indices.cend() && *it == i) {
				res[i] = { objects[i].get(), true };
				it++;
			}
			else {
				res[i] = { objects[i].get(), false };
			}
		}
		if (preview_present) res[objects.size()] = { preview.get(), true };
		return res;
	}

	int ObjectController::get_nearest_object_from_pixels(float x, float y, const Camera& camera, int width, int height)
	{
		// raycasting
		// TODO proper torus raycasting
		/*Vector3 cam_pos = camera.get_world_position();
		Vector3 world_vec = normalize(camera.screen_to_world({ 2.0f * x / width - 1.0f, 1.0f - 2.0f * y / height, 0.0f }, width, height) - cam_pos);*/
		Ray ray = camera.get_ray_to(camera.screen_to_world({ 2.0f * x / width - 1.0f, 1.0f - 2.0f * y / height, 0.0f }, width, height));

		int min_i = -1;
		double min_dist = INFINITY;
		for (int i = 0; i < objects.size(); ++i)
		{
			if (!objects[i]->get_const_renderable().visible) continue;
			const double dist = objects[i]->intersect_with_ray(ray);
			if (isnan(dist)) continue;
			if (dist < min_dist)
			{
				min_dist = dist;
				min_i = i;
			}
		}

		return min_i;
		/* float min_sin = ApplicationSettings::SELECTION_THRESHOLD;

		 int min_i = -1;
		 for (int i = 0; i < objects.size();++i) {
			 if (!objects[i]->is_transformable()) continue;
			 Vector3 to_object = normalize(objects[i]->transformation.position - cam_pos);
			 float sin = cross(to_object, world_vec).length();
			 if (sin < min_sin)
			 {
				 min_sin = sin;
				 min_i = i;
			 }
		 }
		 return min_i;*/
	}

	void ObjectController::toggle_selection_objects_inside(const Rectangle& rect, const Camera& camera, int width, int height)
	{
		if (!selected_indices_snapshot_valid)
		{
			selected_indices_snapshot = selected_indices;
			selected_indices_snapshot_valid = true;
		}
		//Rectangle r = rect;

		//r.x_min /= width;
		//r.x_max /= width;
		//r.y_min /= height;
		//r.y_max /= height;

		const auto transformation = camera.get_projection_matrix(width, height) * camera.get_view_matrix();
		auto s_it = selected_indices_snapshot.begin();
		auto it = objects.begin();
		int i = 0;
		while (it != objects.end()) {
			const auto& obj = *it->get();
			if (!obj.is_illusory()) {
				if (obj.is_inside_screen_rectangle(rect, transformation)) {
					if (s_it != selected_indices_snapshot.end() && i == *s_it)
					{
						selected_indices.erase(i);
						++s_it;
					}
					else
					{
						selected_indices.insert(i);
					}
				}
				else {
					if (s_it != selected_indices_snapshot.end() && i == *s_it)
					{
						selected_indices.insert(i);
						++s_it;
					}
					else
					{
						selected_indices.erase(i);
					}
				}
			}

			++it;
			++i;
		}
		on_selection_changed();
	}

	WindowHandle ObjectController::create_settings_window(Camera& camera, const Cursor& cursor, WindowHandle& object_settings_window)
	{
		object_settings_window->visible = false;
		return make_window<ObjectControllerSettingsWindow>(*this, camera, cursor, object_settings_window, task_manager);
	}

	WindowHandle ObjectController::create_current_object_settings_window(ParameterSpaceViewWindow& parameter_view)
	{
		return make_window<ObjectSettingsWindow>(*this, parameter_view);
	}

	void ObjectController::add_objects_to_serializer(Serializer& serializer) const
	{
		for (int i = 0; i < objects.size(); ++i)
			objects[i]->add_to_serializer(serializer, i);
	}

	void ObjectController::merge_points(Point& p1, Point& p2)
	{
		clear_selection();
		p1.transformation.position = 0.5f * (p1.transformation.position + p2.transformation.position);
		p1.update_renderable_matrix();
		for (auto it = objects.begin(); it != objects.end();)
		{
			if ((*it).get() == &p2)
			{
				p2.replace_in_observers_by(p1);
				p1.inherit_observers_from(p2);
				p1.add_persistence(p2);
				p1.invalidate_observers();

				it = objects.erase(it);
				break;
			}
			else
				++it;
		}
	}
}