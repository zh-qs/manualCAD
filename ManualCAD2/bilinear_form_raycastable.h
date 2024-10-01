#pragma once

#include "renderable.h"

namespace ManualCAD {
	class BilinearFormRaycastable : public Renderable {
		Matrix4x4 inverse_model;
		mutable int current_downsampling_scale = 1;
	public:
		BilinearFormRaycastable(const Matrix4x4& model) : Renderable(model) { vao.unbind(); }
		BilinearFormRaycastable() : Renderable() { vao.unbind(); }

		Matrix4x4 form;
		int downsampling_scale = 1;
		float specular_exponent = 10.0f;

		virtual void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const;

		void apply_transformation(const Transformation& trans) override {
			Renderable::apply_transformation(trans);
			inverse_model = trans.get_inversed_matrix();
		}

		void apply_transformations(const Transformation& trans, const Transformation& combine, const Vector3& rotation_center) override {
			Renderable::apply_transformations(trans, combine, rotation_center);
			inverse_model = trans.get_inversed_matrix_combined_with(combine, rotation_center);
		}

		Matrix4x4 get_transformed_form() const {
			return mul_with_first_transposed(inverse_model, form) * inverse_model;
		}

		int get_current_downsampling_scale() const { return current_downsampling_scale; }
		void update_current_downsampling() const { if (current_downsampling_scale > 1) current_downsampling_scale /= 2; }
		void reset_downsampling() { current_downsampling_scale = downsampling_scale; }
	};
}