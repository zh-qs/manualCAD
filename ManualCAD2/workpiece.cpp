#include "workpiece.h"
#include "object_settings.h"

namespace ManualCAD {
	int Workpiece::counter = 0;

	void Workpiece::generate_renderable()
	{
		renderable.set_data_from_map(height_map);
		renderable.color = color;
	}

	void Workpiece::build_specific_settings(ObjectSettingsWindow& parent)
	{
		ObjectSettings::build_workpiece_settings(*this, parent);
	}

	void Workpiece::set_milling_program(MillingProgram&& milling_program)
	{
		program = std::move(milling_program);
		auto positions = program->get_cutter_positions();
		path.set_data(positions);
		program->get_cutter().generate_cutter_mesh(cylinder);
		set_cutter_mesh_position(positions[0]);
		cylinder.visible = true;
		path.visible = true;
		active_task = nullptr;
		active_task_ended = true;
	}

	void Workpiece::delete_milling_program()
	{
		program = std::nullopt;
		path.set_data({});
		cylinder.set_data({}, {}, {});
		cylinder.visible = false;
		path.visible = false;
	}

	bool Workpiece::has_milling_program() const
	{
		return program.has_value();
	}

	void Workpiece::execute_milling_program_immediately()
	{
		if (active_task != nullptr && !active_task_ended)
		{
			active_task->execute_immediately();
			return;
		}
		if (has_milling_program())
			program->execute_on(*this);
		active_task_ended = true;
	}

	void Workpiece::animate_milling_program()
	{
		if (has_milling_program() && (active_task == nullptr || active_task_ended))
		{
			active_task = &task_manager.add_task(program->get_task(*this, active_task_ended));
		}
	}

	std::vector<ObjectHandle> Workpiece::clone() const
	{
		return std::vector<ObjectHandle>(); // IntersectionCurve is a special object and thus can't be cloned
	}

	void Workpiece::on_delete()
	{
		if (active_task != nullptr && !active_task_ended)
			active_task->terminate();
	}
}