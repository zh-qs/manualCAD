#pragma once

#include "algebra.h"
#include "object.h"
#include "bilinear_form_raycastable.h"

namespace ManualCAD
{
	class BilinearFormObject : public Object {
	public:
		BilinearFormObject() : Object(raycastable) {}
	protected:
		BilinearFormRaycastable raycastable;

		void generate_renderable() override;

	public:
		void bind_with(Object& object) override {};
		void remove_binding_with(Object& object) override {};

		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }

		void replace_child_by(Object& child, Object& other) override {}

		void on_camera_move() override { raycastable.reset_downsampling(); }
	};
}