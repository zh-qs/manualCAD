#pragma once

#include "object.h"
#include "object_controller.h"
#include <vector>
#include <filesystem>
#include "serializer.h"

namespace ManualCAD
{
	class JSONSerializer : public Serializer
	{
		ObjectController& controller;
		Camera& camera;
		size_t object_count;

	public:
		JSONSerializer(ObjectController& controller, Camera& camera);

		void serialize(const std::filesystem::path& filepath) override;
		void deserialize(const std::filesystem::path& filepath) override;

		void add_point(const Point& point, int idx) override;
		void add_torus(const Torus& torus, int idx) override;
		void add_bezier_c0_curve(const BezierC0Curve& curve, int idx) override;
		void add_bezier_c2_curve(const BezierC2Curve& curve, int idx) override;
		void add_interpolation_spline(const InterpolationSpline& spline, int idx) override;
		void add_bicubic_bezier_c0_surface(const BicubicC0BezierSurface& surf, int idx) override;
		void add_bicubic_bezier_c2_surface(const BicubicC2BezierSurface& surf, int idx) override;
		void add_bicubic_nurbs_c2_surface(const BicubicC2NURBSSurface& surf, int idx) override;
	};
}

