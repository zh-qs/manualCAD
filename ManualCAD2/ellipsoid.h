#pragma once

#include "bilinear_form_raycastable.h"
#include <cmath>

namespace ManualCAD
{
	class Ellipsoid : public BilinearFormRaycastable {
	public:
		Ellipsoid(float a, float b, float c, Vector4 color) : BilinearFormRaycastable(color) {
			form.elem[0][0] = 1 / (a * a);
			form.elem[1][1] = 1 / (b * b);
			form.elem[2][2] = 1 / (c * c);
			form.elem[3][3] = -1;
		}

		float get_semiaxis(int index) {
			return 1 / sqrt(form.elem[index][index]);
		}

		void set_semiaxes(float a, float b, float c) {
			form.elem[0][0] = 1 / (a * a);
			form.elem[1][1] = 1 / (b * b);
			form.elem[2][2] = 1 / (c * c);
		}
	};
}