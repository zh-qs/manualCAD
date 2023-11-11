#pragma once

#include "parametric_surface.h"

namespace ManualCAD
{
	class OffsetSurface : public ParametricSurface {
		const ParametricSurfaceWithSecondDerivative& surf;
	public:
		float offset = 0.0f;

		OffsetSurface(const ParametricSurfaceWithSecondDerivative& surf, float offset) : surf(surf), offset(offset) {}

		Range<float> get_u_range() const override { return surf.get_u_range(); }
		Range<float> get_v_range() const override { return surf.get_v_range(); }

		Vector3 evaluate(float u, float v) const override {
			return surf.evaluate(u, v) + offset * surf.normal(u, v);
		}
		Vector3 normal(float u, float v) const override {
			return normalize(cross(du(u, v), dv(u, v)));
		}
		Vector3 du(float u, float v) const override {
			const auto fu = surf.du(u, v),
				fv = surf.dv(u, v),
				fuu = surf.duu(u, v),
				fuv = surf.duv(u, v);
			const auto cr = cross(fu, fv);
			const auto dcr = cross(fuu, fv) + cross(fu, fuv);
			const float invl = 1.0f/cr.length();
			return surf.du(u,v) + (offset * invl) * (dcr - (invl * invl * dot(dcr, cr)) * cr);
		}
		Vector3 dv(float u, float v) const override {
			const auto fu = surf.du(u, v),
				fv = surf.dv(u, v),
				fuv = surf.duv(u, v),
				fvv = surf.dvv(u, v);
			const auto cr = cross(fu, fv);
			const auto dcr = cross(fuv, fv) + cross(fu, fvv);
			const float invl = 1.0f / cr.length();
			return surf.dv(u, v) + (offset * invl) * (dcr - (invl * invl * dot(dcr, cr)) * cr);
		}

		Box get_bounding_box() const override {
			Box box = surf.get_bounding_box();
			box.offset_by(offset);
			return box;
		}

		std::vector<RangedBox<float>> get_patch_bounds() const override {
			auto bounds = surf.get_patch_bounds();
			for (auto& rb : bounds)
			{
				rb.box.offset_by(offset);
			}
			return bounds;
		}
	};
}