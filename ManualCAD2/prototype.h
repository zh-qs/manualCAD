#pragma once

#include "object.h"

namespace ManualCAD
{
	class Prototype : public Object {
		friend class ObjectSettings;

		static int counter;

		Line view;
		float offset = 0.3f;
		float mill_height = 0.5f;
		Vector3 size = { 15, 5, 15 };
		std::list<const ParametricSurface*> surfaces;

		Box bounding_box = Box::degenerate();
		bool box_valid = false;

		float scale = 0.0f;

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;
	public:
		Prototype(const std::list<const ParametricSurface*> surfaces) : Object(view), view(), surfaces(surfaces) {
			name = "Milling prototype " + std::to_string(counter++);
		}

		void bind_with(Object& object) override {
			const ParametricSurface* surf = dynamic_cast<ParametricSurface*>(&object);
			if (surf != nullptr)
				surfaces.push_back(surf);
			box_valid = false;
			invalidate();
		}
		void remove_binding_with(Object& object) override {
			const ParametricSurface* surf = dynamic_cast<ParametricSurface*>(&object);
			if (surf != nullptr)
				surfaces.remove(surf);
			box_valid = false;
			invalidate();
		}

		void update_view();

		float intersect_with_ray(const Vector3& origin, const Vector3& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }

		void replace_child_by(Object& child, Object& other) override {}

		std::vector<ObjectHandle> clone() const override;

		void dispose() override { Object::dispose(); }
	};
}