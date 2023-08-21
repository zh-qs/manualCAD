#include "settings_window.h"
#include "serializer.h"
#include "logger.h"
#include <tinyfiledialogs.h>
#include <filesystem>
#include <string>
#include <Scene/SerializerException.h>

static const char* PATTERN = "*.json";

namespace ManualCAD
{

	void ObjectControllerSettingsWindow::load_model_from_file()
	{
		if (!check_and_confirm_unsaved())
			return;
		char* filename;
		try
		{
			filename = tinyfd_openFileDialog("Open", "", 1, &PATTERN, "", 0);
		}
		catch (const std::exception& e)
		{
			Logger::log("[ERROR] Opening JSON file: ", e.what());
		}
		if (filename == nullptr)
			return;
		std::string s(filename);
		Serializer serializer(controller, camera);
		controller.clear_all();
		controller.set_saved();
		try
		{
			serializer.deserialize(s);
		}
		catch (MG1::SerializerException& e)
		{
			tinyfd_messageBox("Error", e.what(), "ok", "error", 1);
		}
	}

	void ObjectControllerSettingsWindow::save_model_to_file()
	{
		char* filename;
		try
		{
			filename = tinyfd_saveFileDialog("Save", "", 1, &PATTERN, "");
		}
		catch (const std::exception& e)
		{
			Logger::log("[ERROR] Saving JSON file: ", e.what());
		}
		if (filename == nullptr)
			return;
		std::string s(filename);
		Serializer serializer(controller, camera);
		serializer.serialize(s);
		controller.set_saved();
	}

	bool ObjectControllerSettingsWindow::check_and_confirm_unsaved()
	{
		if (controller.is_unsaved())
		{
			int result = tinyfd_messageBox("Question", "Model is not saved. Continue?", "yesno", "question", 0);
			return result == 1;
		}
		return true;
	}

	std::pair<ParametricSurface*, ParametricSurface*> ObjectControllerSettingsWindow::try_get_surfaces_from_selection(bool& invalid_selection_count, bool& not_surface)
	{
		auto& selected = controller.get_selected_objects();
		if (selected.count() != 1 && selected.count() != 2)
		{
			invalid_selection_count = true;
			return { nullptr, nullptr };
		}

		ParametricSurface* surfs[2] = { nullptr,nullptr };
		int i = 0;
		for (auto* obj : selected)
		{
			ParametricSurface* surf = dynamic_cast<ParametricSurface*>(obj);
			if (surf == nullptr) {
				not_surface = true;
				return { nullptr, nullptr };
			}
			surfs[i++] = surf;
		}

		return { surfs[0], surfs[1] };
	}

	void ObjectControllerSettingsWindow::try_add_intersection_curve_with_cursor_hint(ParametricSurface& surf1, ParametricSurface& surf2, bool& not_intersect, bool& timeout)
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

	void ObjectControllerSettingsWindow::try_add_intersection_curve_without_hint(ParametricSurface& surf1, ParametricSurface& surf2, bool& not_intersect, bool& timeout)
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

	void ObjectControllerSettingsWindow::try_add_self_intersection_curve_with_cursor_hint(ParametricSurface& surf, bool& not_intersect, bool& timeout)
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

	void ObjectControllerSettingsWindow::try_add_self_intersection_curve_without_hint(ParametricSurface& surf, bool& not_intersect, bool& timeout)
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


