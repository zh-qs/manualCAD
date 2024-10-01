#include <filesystem>
#include <stdexcept>
#include <string>

namespace ManualCAD
{
	class Serializer {
	public:
		virtual void serialize(const std::filesystem::path& filepath) = 0;
		virtual void deserialize(const std::filesystem::path& filepath) = 0;

		virtual void add_point(const Point& point, int idx) = 0;
		virtual void add_torus(const Torus& torus, int idx) = 0;
		virtual void add_bezier_c0_curve(const BezierC0Curve& curve, int idx) = 0;
		virtual void add_bezier_c2_curve(const BezierC2Curve& curve, int idx) = 0;
		virtual void add_interpolation_spline(const InterpolationSpline& spline, int idx) = 0;
		virtual void add_bicubic_bezier_c0_surface(const BicubicC0BezierSurface& surf, int idx) = 0;
		virtual void add_bicubic_bezier_c2_surface(const BicubicC2BezierSurface& surf, int idx) = 0;
		virtual void add_bicubic_nurbs_c2_surface(const BicubicC2NURBSSurface& surf, int idx) = 0;
	};

	class SerializerException : public std::runtime_error {
	public:
		SerializerException(const std::string& message) : std::runtime_error(message) {}
	};
}