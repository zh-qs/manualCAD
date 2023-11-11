#pragma once

#include "object.h"
#include "height_map.h"
#include "workpiece_renderable.h"
#include "task.h"
#include "milling_program.h"
#include <optional>
#include "triangle_mesh.h"

namespace ManualCAD
{
	class Workpiece : public Object {
		friend class ObjectSettings;

		static int counter;
		WorkpieceRenderable renderable;
		Line path;
		TriangleMesh cylinder;
		TaskManager& task_manager;
		Task* active_task = nullptr;
		bool active_task_ended = true;
		float max_cutter_depth = 10.0f;

		std::optional<MillingProgram> program;

		int divisions_x = 1500, divisions_y = 1500;
		Vector3 size = { 15, 5, 15 };

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;
	public:
		HeightMap height_map;

		Workpiece(TaskManager& task_manager) : Object(renderable), path(), cylinder(), renderable(size, path, cylinder), task_manager(task_manager), program(std::nullopt) {
			height_map = { divisions_x, divisions_y, size };
			name = "Milling workpiece " + std::to_string(counter++);
			path.color = { 0.0f,1.0f,0.0f,1.0f };
			cylinder.color = { 0.7f, 0.7f, 0.7f, 1.0f };
			transformable = false;
		}

		void bind_with(Object& object) override {}
		void remove_binding_with(Object& object) override {}
		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }

		void replace_child_by(Object& child, Object& other) override {}

		void recreate_height_map() {
			height_map.resize(divisions_x, divisions_y, size);
		}

		void set_milling_program(MillingProgram&& milling_program);
		void delete_milling_program();
		bool has_milling_program() const;
		bool can_execute_milling_program() const { return active_task_ended == true; }
		void animate_milling_program();
		void execute_milling_program_immediately();
		MillingProgram& get_milling_program() { return program.value(); }
		const MillingProgram& get_milling_program() const { return program.value(); }

		void set_cutter_mesh_position(const Vector3& pos) { cylinder.set_model_matrix(Matrix4x4::translation(pos)); }

		float get_max_cutter_depth() const { return max_cutter_depth; }

		std::vector<ObjectHandle> clone() const override;

		void dispose() override { Object::dispose(); path.dispose(); cylinder.dispose(); }
		void on_delete() override;
	};
}