#include "settings_window.h"
#include "json_serializer.h"
#include "logger.h"
#include "system_dialog.h"
#include <filesystem>
#include <string>

namespace ManualCAD
{

	void ObjectControllerSettingsWindow::load_model_from_file(bool clear_workspace)
	{
		if (clear_workspace && !check_and_confirm_unsaved())
			return;
		std::string filename;
		try
		{
			filename = SystemDialog::open_file_dialog("Open", { {"*.json", "JSON file"} });
		}
		catch (const std::exception& e)
		{
			Logger::log_error("[ERROR] Opening JSON file: %s\n", e.what());
		}
		if (filename.empty())
			return;
		std::string s(filename);
		JSONSerializer serializer(controller, camera);
		if (clear_workspace)
			controller.clear_all();
		controller.set_saved();
		try
		{
			serializer.deserialize(s);
		}
		catch (SerializerException& e)
		{
			Logger::log_error("[ERROR] Error deserializing JSON:\n%s\n", e.what());
			SystemDialog::message_box("Error", "Error deserializing JSON", SystemDialog::ButtonType::Ok, SystemDialog::MessageBoxType::Error);
		}
	}

	void ObjectControllerSettingsWindow::save_model_to_file()
	{
		std::string filename;
		try
		{
			filename = SystemDialog::save_file_dialog("Save", { { "*.json", "JSON file" } });
		}
		catch (const std::exception& e)
		{
			Logger::log_error("[ERROR] Saving JSON file: %s\n", e.what());
		}
		if (filename.empty())
			return;
		if (filename.size() < 4 || filename.substr(filename.size() - 4, 4) != ".json")
			filename += ".json";
		std::string s(filename);
		JSONSerializer serializer(controller, camera);
		serializer.serialize(s);
		controller.set_saved();
	}

	bool ObjectControllerSettingsWindow::check_and_confirm_unsaved()
	{
		if (controller.is_unsaved())
		{
			auto result = SystemDialog::message_box("Question", "Model is not saved. Continue?", SystemDialog::ButtonType::YesNo, SystemDialog::MessageBoxType::Question);
			return result == SystemDialog::Button::YesOk;
		}
		return true;
	}

	std::pair<ParametricSurfaceObject*, ParametricSurfaceObject*> ObjectControllerSettingsWindow::try_get_surfaces_from_selection(bool& invalid_selection_count, bool& not_surface)
	{
		auto& selected = controller.get_selected_objects();
		if (selected.count() != 1 && selected.count() != 2)
		{
			invalid_selection_count = true;
			return { nullptr, nullptr };
		}

		ParametricSurfaceObject* surfs[2] = { nullptr,nullptr };
		int i = 0;
		for (auto* obj : selected)
		{
			ParametricSurfaceObject* surf = dynamic_cast<ParametricSurfaceObject*>(obj);
			if (surf == nullptr) {
				not_surface = true;
				return { nullptr, nullptr };
			}
			surfs[i++] = surf;
		}

		return { surfs[0], surfs[1] };
	}

	void ObjectControllerSettingsWindow::try_add_all_intersection_curves(ParametricSurfaceObject& surf1, ParametricSurfaceObject& surf2, bool& not_intersect, bool& timeout)
	{
		try 
		{
			auto curves = IntersectionCurve::find_many_intersections(surf1, surf2, intersection_step_length, intersection_max_steps, intersection_sample_count, intersection_sample_count);
			if (curves.empty())
			{
				not_intersect = true;
				return;
			}
			for (auto& c : curves)
			{
				controller.add_object(std::move(c));
			}
		}
		catch (const CommonIntersectionPointNotFoundException&)
		{
			not_intersect = true;
		}
		catch (const TimeoutException&)
		{
			timeout = true;
		}
	}

	void ObjectControllerSettingsWindow::try_add_intersection_curve_with_cursor_hint(ParametricSurfaceObject& surf1, ParametricSurfaceObject& surf2, bool& not_intersect, bool& timeout)
	{
		try
		{
			auto curve = IntersectionCurve::intersect_surfaces_with_hint(surf1, surf2, intersection_step_length, intersection_max_steps, cursor.get_world_position());
			controller.add_object(std::move(curve));
		}
		catch (const CommonIntersectionPointNotFoundException&)
		{
			not_intersect = true;
		}
		catch (const TimeoutException&)
		{
			timeout = true;
		}

		//auto curve = Object::create<Point>();
		//auto c2 = Object::create<Point>();

		//curve->name = "First";
		//c2->name = "Second";
		//
		//auto p = IntersectionCurve::find_nearest_point(*surfs[0], cursor.get_world_position()),
		//	q = IntersectionCurve::find_nearest_point(*surfs[1], cursor.get_world_position());
		//curve->transformation.position = surfs[0]->evaluate(p.x, p.y);
		//c2->transformation.position = surfs[1]->evaluate(q.x, q.y);

		///*auto p = IntersectionCurve::find_first_common_point(*surfs[0], *surfs[1], cursor.get_world_position());
		//curve->transformation.position = surfs[0]->evaluate(p.first.x, p.first.y);
		//c2->transformation.position = surfs[1]->evaluate(p.second.x, p.second.y);*/
		//
		//controller.add_object(std::move(curve));
		//controller.add_object(std::move(c2));
	}

	void ObjectControllerSettingsWindow::try_add_intersection_curve_without_hint(ParametricSurfaceObject& surf1, ParametricSurfaceObject& surf2, bool& not_intersect, bool& timeout)
	{
		try
		{
			auto curve = IntersectionCurve::intersect_surfaces_without_hint(surf1, surf2, intersection_step_length, intersection_max_steps, intersection_sample_count, intersection_sample_count);
			controller.add_object(std::move(curve));
		}
		catch (const CommonIntersectionPointNotFoundException&)
		{
			not_intersect = true;
		}
		catch (const TimeoutException&)
		{
			timeout = true;
		}
	}

	void ObjectControllerSettingsWindow::try_add_self_intersection_curve_with_cursor_hint(ParametricSurfaceObject& surf, bool& not_intersect, bool& timeout)
	{
		try
		{
			auto curve = IntersectionCurve::self_intersect_surface_with_hint(surf, intersection_step_length, intersection_max_steps, cursor.get_world_position());
			controller.add_object(std::move(curve));
		}
		catch (const CommonIntersectionPointNotFoundException&)
		{
			not_intersect = true;
		}
		catch (const TimeoutException&)
		{
			timeout = true;
		}
	}

	void ObjectControllerSettingsWindow::try_add_self_intersection_curve_without_hint(ParametricSurfaceObject& surf, bool& not_intersect, bool& timeout)
	{
		try
		{
			auto curve = IntersectionCurve::self_intersect_surface_without_hint(surf, intersection_step_length, intersection_max_steps, intersection_sample_count, intersection_sample_count);
			controller.add_object(std::move(curve));
		}
		catch (const CommonIntersectionPointNotFoundException&)
		{
			not_intersect = true;
		}
		catch (const TimeoutException&)
		{
			timeout = true;
		}
	}
}


