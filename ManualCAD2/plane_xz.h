#pragma once

#include "parametric_surface.h"

namespace ManualCAD
{
	class PlaneXZ : public ParametricSurfaceWithSecondDerivative {
		static constexpr int PLANE_PATCH_DIVISIONS = 5;

		Vector2 min, max;
		float height;
	public:
		PlaneXZ(const Vector2& min, const Vector2& max, float height) : min(min), max(max), height(height) {}

		Range<float> get_u_range() const override {
			return { min.x, max.x };
		}
		Range<float> get_v_range() const override {
			return { min.y, max.y };
		}

		Vector3 evaluate(float u, float v) const override {
			return { u, height, v };
		}
		Vector3 normal(float u, float v) const override {
			return { 0.0f,1.0f,0.0f };
		}
		Vector3 du(float u, float v) const override {
			return { 1.0f,0.0f,0.0f };
		}
		Vector3 dv(float u, float v) const override {
			return { 0.0f,0.0f,1.0f };
		}
		Vector3 duu(float u, float v) const override {
			return { 0.0f,0.0f,0.0f };
		}
		Vector3 duv(float u, float v) const override {
			return { 0.0f,0.0f,0.0f };
		}
		Vector3 dvv(float u, float v) const override {
			return { 0.0f,0.0f,0.0f };
		}

		Box get_bounding_box() const override {
			Box box;
			box.x_min = min.x;
			box.x_max = max.x;
			box.y_min = height;
			box.y_max = height;
			box.z_min = min.y;
			box.z_max = max.y;
			return box;
		}

		std::vector<RangedBox<float>> get_patch_bounds() const override {
			std::vector<RangedBox<float>> result(PLANE_PATCH_DIVISIONS * PLANE_PATCH_DIVISIONS);
			for (int i = 0; i < PLANE_PATCH_DIVISIONS; ++i)
			{
				for (int j = 0; j < PLANE_PATCH_DIVISIONS; ++j)
				{
					Box box;
					box.x_min = min.x + i * (max.x - min.x) / PLANE_PATCH_DIVISIONS;
					box.x_max = min.x + (i + 1) * (max.x - min.x) / PLANE_PATCH_DIVISIONS;
					box.y_min = height;
					box.y_max = height;
					box.z_min = min.y + j * (max.y - min.y) / PLANE_PATCH_DIVISIONS;
					box.z_max = max.y + (j + 1) * (max.y - min.y) / PLANE_PATCH_DIVISIONS;
					Range<float> us{ box.x_min, box.x_max },
						vs{ box.z_min,box.z_max };
					result.push_back({ box, us, vs });
				}
			}
			return result;
		}
	};
}