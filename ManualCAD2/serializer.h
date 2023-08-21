#pragma once

#include <Serializer.h>
#include "object.h"
#include "object_controller.h"
#include <vector>
#include <filesystem>

namespace ManualCAD
{
	class Serializer
	{
		MG1::SceneSerializer serializer;
		ObjectController& controller;
		Camera& camera;
		size_t object_count;

		MG1::PointRef get_point_ref_to(const Point& point);

	public:
		Serializer(ObjectController& controller, Camera& camera);

		void serialize(const std::filesystem::path& filepath);
		void deserialize(const std::filesystem::path& filepath);
		
		void add_point(const Point& point, int idx);
		void add_torus(const Torus& torus, int idx);
		void add_bezier_c0_curve(const BezierC0Curve& curve, int idx);
		void add_bezier_c2_curve(const BezierC2Curve& curve, int idx);
		void add_interpolation_spline(const InterpolationSpline& spline, int idx);
		void add_bicubic_bezier_c0_surface(const BicubicC0BezierSurface& surf, int idx);
		void add_bicubic_bezier_c2_surface(const BicubicC2BezierSurface& surf, int idx);
	};
}

