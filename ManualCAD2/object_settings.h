#pragma once
#include "object.h"
#include "workpiece.h"

namespace ManualCAD
{
	class ObjectSettingsWindow;
	class ObjectSettings {

	public:
		static void build_general_settings(Object& object);
		static void build_collection_settings(ObjectCollection& collection, ObjectSettingsWindow& parent);
		static void build_torus_settings(Torus& torus, ObjectSettingsWindow& parent);
		static void build_bezier_c0_curve_settings(BezierC0Curve& curve, ObjectSettingsWindow& parent);
		static void build_bezier_c2_curve_settings(BezierC2Curve& curve, ObjectSettingsWindow& parent);
		static void build_interpolation_spline_curve_settings(InterpolationSpline& curve, ObjectSettingsWindow& parent);
		static void build_bicubic_c0_bezier_surface_settings(BicubicC0BezierSurface& surf, ObjectSettingsWindow& parent);
		static void build_bicubic_c2_bezier_surface_settings(BicubicC2BezierSurface& surf, ObjectSettingsWindow& parent);
		static void build_bicubic_c0_bezier_surface_preview_settings(BicubicC0BezierSurfacePreview& preview, ObjectSettingsWindow& parent);
		static void build_bicubic_c2_bezier_surface_preview_settings(BicubicC2BezierSurfacePreview& preview, ObjectSettingsWindow& parent);
		static void build_gregory_20_param_surface_settings(Gregory20ParamSurface& surf, ObjectSettingsWindow& parent);
		static void build_intersection_curve_settings(IntersectionCurve& curve, ObjectSettingsWindow& parent);
		static void build_parametric_surface_settings(ParametricSurface& surf, ObjectSettingsWindow& parent);
		static void build_workpiece_settings(Workpiece& workpiece, ObjectSettingsWindow& parent);
	};
}