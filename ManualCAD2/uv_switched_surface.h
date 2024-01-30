#pragma once

#include "parametric_surface.h"

namespace ManualCAD
{
	class UVSwitchedSurface : public ParametricSurface {
		const ParametricSurface& surf;
	public:
		UVSwitchedSurface(const ParametricSurface& surf) : surf(surf) {}

		Range<float> get_u_range() const override { return surf.get_v_range(); }
		Range<float> get_v_range() const override { return surf.get_u_range(); }

		Vector3 evaluate(float u, float v) const override {
			return surf.evaluate(v, u);
		}
		Vector3 normal(float u, float v) const override {
			return normalize(cross(du(u, v), dv(u, v)));
		}
		Vector3 du(float u, float v) const override {
			return dv(v, u);
		}
		Vector3 dv(float u, float v) const override {
			return du(v, u);
		}

		Box get_bounding_box() const override {
			return surf.get_bounding_box();
		}

		std::vector<RangedBox<float>> get_patch_bounds() const override {
			auto bounds = surf.get_patch_bounds();
			for (auto& rb : bounds)
			{
				std::swap(rb.us, rb.vs);
			}
			return bounds;
		}
	};
}