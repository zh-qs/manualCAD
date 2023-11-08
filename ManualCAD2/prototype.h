#pragma once

#include "object.h"
#include "milling_program.h"
#include "cutter.h"
#include <memory>
#include <optional>

namespace ManualCAD
{
	class Prototype : public Object {
		enum class ProgramType {
			Rough, FlatPlane, Envelope, Detailed, Signature
		};

		friend class ObjectSettings;

		static int counter;

		Line view;
		std::vector<Vector3> view_boundary_points;
		Vector3 center;
		float offset = 0.3f;
		float mill_height = 1.3f;
		Vector3 size = { 15, 5, 15 };
		std::list<const ParametricSurfaceObject*> surfaces;

		std::optional<MillingProgram> generated_program = std::nullopt;

		Box bounding_box = Box::degenerate();
		bool box_valid = false;

		float scale = 0.0f;
		float epsilon_factor = 0.1f;

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;

		Vector2 to_workpiece_coords(const Vector2& model_coords) { return (1.0f / scale) * (model_coords - Vector2{center.x, center.z}); }
		Vector3 to_workpiece_coords(const Vector3& model_coords) { return (1.0f / scale) * (model_coords - center); }
		float safe_height() const { return scale * (size.y + 1.0f) + center.y; }

		std::vector<Vector3> leap(const Vector2& from, const Vector2& to);
		std::vector<Vector3> link_flat_paths(std::vector<std::vector<Vector2>> paths);
		std::vector<Vector3> link_single_flat_loop(std::vector<Vector2> loop);
		std::vector<Vector3> compact_path(const std::vector<Vector3>& path);
		void to_workpiece_coords(std::vector<Vector3>& model_coords) { for (auto& c : model_coords) c = to_workpiece_coords(c); }

		void show_envelope_experimental();
		void generate_rough_program(const Cutter& cutter);
		void generate_flat_plane_program(const Cutter& cutter);
		void generate_envelope_program(const Cutter& cutter);
		void generate_detailed_program(const Cutter& cutter);
		void generate_signature_program(const Cutter& cutter);
	public:
		Prototype(const std::list<const ParametricSurfaceObject*> surfaces) : Object(view), view(), surfaces(surfaces) {
			name = "Milling prototype " + std::to_string(counter++);
			view.looped = true;
		}

		void bind_with(Object& object) override {
			const ParametricSurfaceObject* surf = dynamic_cast<ParametricSurfaceObject*>(&object);
			if (surf != nullptr)
				surfaces.push_back(surf);
			box_valid = false;
			invalidate();
		}
		void remove_binding_with(Object& object) override {
			const ParametricSurfaceObject* surf = dynamic_cast<ParametricSurfaceObject*>(&object);
			if (surf != nullptr)
				surfaces.remove(surf);
			box_valid = false;
			invalidate();
		}

		void update_view();

		bool invalidate_object_and_box_if(bool cond) {
			if (cond)
			{
				box_valid = false;
				invalidate();
			}
			return cond;
		}

		float intersect_with_ray(const Vector3& origin, const Vector3& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }

		void generate_program(ProgramType type, std::unique_ptr<Cutter>&& cutter);
		void remove_program() { generated_program = std::nullopt; }

		void replace_child_by(Object& child, Object& other) override {}

		std::vector<ObjectHandle> clone() const override;

		void dispose() override { Object::dispose(); }
	};
}