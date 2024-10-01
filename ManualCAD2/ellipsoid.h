#pragma once

#include "bilinear_form_object.h"
#include <cmath>

namespace ManualCAD
{
	class Ellipsoid : public BilinearFormObject {
		friend class ObjectSettings;

		static int counter;
	protected:
		void build_specific_settings(ObjectSettingsWindow& parent) override;
	public:
		Ellipsoid() : BilinearFormObject() {
			name = "Ellipsoid " + std::to_string(counter++);

			float a = 1, b = 1, c = 1;
			raycastable.form.elem[0][0] = 1 / (a * a);
			raycastable.form.elem[1][1] = 1 / (b * b);
			raycastable.form.elem[2][2] = 1 / (c * c);
			raycastable.form.elem[3][3] = -1;
		}

		float get_semiaxis(int index) {
			return 1 / sqrt(raycastable.form.elem[index][index]);
		}

		void set_semiaxes(float a, float b, float c) {
			raycastable.form.elem[0][0] = 1 / (a * a);
			raycastable.form.elem[1][1] = 1 / (b * b);
			raycastable.form.elem[2][2] = 1 / (c * c);
		}

		std::vector<Handle<Object>> clone() const override;
	};
}