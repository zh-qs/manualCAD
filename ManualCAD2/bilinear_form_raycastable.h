#pragma once

#include "algebra.h"

namespace ManualCAD
{
	class BilinearFormRaycastable {
	public:
		Vector4 color;
		BilinearFormRaycastable(Vector4 color) : color(color) {}

		GLColumnOrderMatrix4x4 get_transformed_form(const Matrix4x4& mat_reverse)
		{
			return { mul_with_first_transposed(mat_reverse, form) * mat_reverse };
		}
	protected:
		Matrix4x4 form;
	};
}