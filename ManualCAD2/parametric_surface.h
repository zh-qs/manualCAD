#pragma once

#include <vector>
#include "algebra.h"
#include "box.h"

namespace ManualCAD 
{
	class ParametricSurface {
		static constexpr float EPS = 1e-6f;
	public:
		virtual Range<float> get_u_range() const = 0;
		virtual Range<float> get_v_range() const = 0;

		virtual Vector3 evaluate(float u, float v) const = 0;
		virtual Vector3 normal(float u, float v) const = 0;
		virtual Vector3 du(float u, float v) const = 0;
		virtual Vector3 dv(float u, float v) const = 0;

		virtual Box get_bounding_box() const = 0;
		virtual std::vector<RangedBox<float>> get_patch_bounds() const = 0;

		bool v_wraps_at_u(float u) const {
			auto vrange = get_v_range();
			return (evaluate(u, vrange.from) - evaluate(u, vrange.to)).length() < EPS;
		}
		bool u_wraps_at_v(float v) const {
			auto urange = get_u_range();
			//auto v1 = evaluate(urange.from, v), v2 = evaluate(urange.to, v), v3 = v1 - v2;
			return (evaluate(urange.from, v) - evaluate(urange.to, v)).length() < EPS;
		}
	};

	class ParametricSurfaceWithSecondDerivative : public ParametricSurface {
	public:
		virtual Vector3 duu(float u, float v) const = 0;
		virtual Vector3 duv(float u, float v) const = 0;
		Vector3 dvu(float u, float v) const { return duv(u, v); }
		virtual Vector3 dvv(float u, float v) const = 0;
	};
}