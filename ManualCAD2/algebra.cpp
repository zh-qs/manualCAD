#include "algebra.h"
#include <cmath>

namespace ManualCAD
{
	Vector4 operator*(const Matrix4x4& mat, const Vector4& vec) {
		return {
			vec.x * mat.elem[0][0] + vec.y * mat.elem[0][1] + vec.z * mat.elem[0][2] + vec.w * mat.elem[0][3],
			vec.x * mat.elem[1][0] + vec.y * mat.elem[1][1] + vec.z * mat.elem[1][2] + vec.w * mat.elem[1][3],
			vec.x * mat.elem[2][0] + vec.y * mat.elem[2][1] + vec.z * mat.elem[2][2] + vec.w * mat.elem[2][3],
			vec.x * mat.elem[3][0] + vec.y * mat.elem[3][1] + vec.z * mat.elem[3][2] + vec.w * mat.elem[3][3]
		};
	}

	Vector4 operator*(const float& x, const Vector4& vec)
	{
		return { x * vec.x, x * vec.y, x * vec.z, x * vec.w };
	}

	Vector4 operator+(const Vector4& vec1, const Vector4& vec2)
	{
		return { vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z, vec1.w + vec2.w };
	}

	Vector4 operator-(const Vector4& vec1, const Vector4& vec2)
	{
		return { vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z, vec1.w - vec2.w };
	}

	Vector4 operator-(const Vector4& vec)
	{
		return { -vec.x,-vec.y,-vec.z,-vec.w };
	}

	bool operator==(const Vector4& vec1, const Vector4& vec2)
	{
		return vec1.x == vec2.x && vec1.y == vec2.y && vec1.z == vec2.z && vec1.w == vec2.w;
	}

	bool operator!=(const Vector4& vec1, const Vector4& vec2)
	{
		return !(vec1 == vec2);
	}

	Vector4 make_vec4(const Vector3& vec, float w)
	{
		return { vec.x,vec.y,vec.z,w };
	}

	Matrix3x3 operator*(const Matrix3x3& mat1, const Matrix3x3& mat2)
	{
		Matrix3x3 result;
		for (int i = 0; i < Matrix3x3::DIMENSION; ++i) {
			for (int j = 0; j < Matrix3x3::DIMENSION; ++j) {
				result.elem[i][j] = mat1.elem[i][0] * mat2.elem[0][j] + mat1.elem[i][1] * mat2.elem[1][j] + mat1.elem[i][2] * mat2.elem[2][j];
			}
		}
		return result;
	}

	Matrix4x4 operator*(const Matrix4x4& mat1, const Matrix4x4& mat2) {
		Matrix4x4 result;
		for (int i = 0; i < Matrix4x4::DIMENSION; ++i) {
			for (int j = 0; j < Matrix4x4::DIMENSION; ++j) {
				result.elem[i][j] = mat1.elem[i][0] * mat2.elem[0][j] + mat1.elem[i][1] * mat2.elem[1][j] + mat1.elem[i][2] * mat2.elem[2][j] + mat1.elem[i][3] * mat2.elem[3][j];
			}
		}
		return result;
	}

	Matrix4x4 mul_with_first_transposed(const Matrix4x4& mat1, const Matrix4x4& mat2) {
		Matrix4x4 result;
		for (int i = 0; i < Matrix4x4::DIMENSION; ++i) {
			for (int j = 0; j < Matrix4x4::DIMENSION; ++j) {
				result.elem[i][j] = mat1.elem[0][i] * mat2.elem[0][j] + mat1.elem[1][i] * mat2.elem[1][j] + mat1.elem[2][i] * mat2.elem[2][j] + mat1.elem[3][i] * mat2.elem[3][j];
			}
		}
		return result;
	}

	float dot(const Vector4& vec1, const Vector4& vec2) {
		return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z + vec1.w * vec2.w;
	}

	float dot(const Vector3& vec1, const Vector3& vec2)
	{
		return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
	}

	float Vector4::length() const
	{
		return sqrt(x * x + y * y + z * z + w * w);;
	}

	Vector4& Vector4::operator/=(const float& a)
	{
		x /= a;
		y /= a;
		z /= a;
		w /= a;
		return *this;
	}

	Vector4& Vector4::operator+=(const Vector4& vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		w += vec.w;
		return *this;
	}

	float Vector3::length() const
	{
		return sqrt(x * x + y * y + z * z);
	}

	bool Vector3::is_valid() const
	{
		return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
	}

	Vector3& Vector3::operator*=(const float& a)
	{
		x *= a;
		y *= a;
		z *= a;
		return *this;
	}

	Vector3& Vector3::operator/=(const float& a)
	{
		x /= a;
		y /= a;
		z /= a;
		return *this;
	}

	Vector3& Vector3::operator+=(const Vector3& vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	Vector3& Vector3::operator-=(const Vector3& vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}

	Vector3& Vector3::operator*=(const Vector3& vec)
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
		return *this;
	}

	Vector3 cross(const Vector3& vec1, const Vector3& vec2) {
		return { vec1.y * vec2.z - vec1.z * vec2.y, vec1.z * vec2.x - vec1.x * vec2.z, vec1.x * vec2.y - vec1.y * vec2.x };
	}

	Vector3 normalize(const Vector3& vec) {
		auto l = vec.length();
		return { vec.x / l, vec.y / l, vec.z / l };
	}

	Vector3 operator+(const Vector3& vec1, const Vector3& vec2)
	{
		return { vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z };
	}

	Vector3 operator-(const Vector3& vec1, const Vector3& vec2)
	{
		return { vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z };
	}

	Vector3 operator-(const Vector3& vec)
	{
		return { -vec.x,-vec.y,-vec.z };
	}

	Vector3 operator*(const float& x, const Vector3& vec)
	{
		return { vec.x * x,vec.y * x,vec.z * x };
	}

	Vector3 operator/(const Vector3& vec, const float& x)
	{
		return { vec.x / x,vec.y / x,vec.z / x };
	}

	Vector3 operator*(const Vector3& vec1, const Vector3& vec2)
	{
		return { vec1.x * vec2.x, vec1.y * vec2.y, vec1.z * vec2.z };
	}

	bool operator==(const Vector3& vec1, const Vector3& vec2)
	{
		return vec1.x == vec2.x && vec1.y == vec2.y && vec1.z == vec2.z;
	}

	bool operator!=(const Vector3& vec1, const Vector3& vec2)
	{
		return !(vec1 == vec2);
	}

	Matrix3x3 transpose(const Matrix3x3& mat)
	{
		Matrix3x3 res;
		res.elem[0][0] = mat.elem[0][0];
		res.elem[0][1] = mat.elem[1][0];
		res.elem[0][2] = mat.elem[2][0];
		res.elem[1][0] = mat.elem[0][1];
		res.elem[1][1] = mat.elem[1][1];
		res.elem[1][2] = mat.elem[2][1];
		res.elem[2][0] = mat.elem[0][2];
		res.elem[2][1] = mat.elem[1][2];
		res.elem[2][2] = mat.elem[2][2];
		return res;
	}

	Matrix4x4 transpose(const Matrix4x4& mat)
	{
		Matrix4x4 res;
		res.elem[0][0] = mat.elem[0][0];
		res.elem[0][1] = mat.elem[1][0];
		res.elem[0][2] = mat.elem[2][0];
		res.elem[0][3] = mat.elem[3][0];
		res.elem[1][0] = mat.elem[0][1];
		res.elem[1][1] = mat.elem[1][1];
		res.elem[1][2] = mat.elem[2][1];
		res.elem[1][3] = mat.elem[3][1];
		res.elem[2][0] = mat.elem[0][2];
		res.elem[2][1] = mat.elem[1][2];
		res.elem[2][2] = mat.elem[2][2];
		res.elem[2][3] = mat.elem[3][2];
		res.elem[3][0] = mat.elem[0][3];
		res.elem[3][1] = mat.elem[1][3];
		res.elem[3][2] = mat.elem[2][3];
		res.elem[3][3] = mat.elem[3][3];
		return res;
	}

	Vector3 operator*(const Matrix3x3& mat, const Vector3& vec) {
		return {
			vec.x * mat.elem[0][0] + vec.y * mat.elem[0][1] + vec.z * mat.elem[0][2],
			vec.x * mat.elem[1][0] + vec.y * mat.elem[1][1] + vec.z * mat.elem[1][2],
			vec.x * mat.elem[2][0] + vec.y * mat.elem[2][1] + vec.z * mat.elem[2][2],
		};
	}

	Vector2 operator+(const Vector2& vec1, const Vector2& vec2)
	{
		return { vec1.x + vec2.x, vec1.y + vec2.y };
	}

	Vector2 operator-(const Vector2& vec1, const Vector2& vec2)
	{
		return { vec1.x - vec2.x, vec1.y - vec2.y };
	}

	Vector2 operator-(const Vector2& vec)
	{
		return { -vec.x, -vec.y };
	}

	Vector2 operator*(const float& x, const Vector2& vec)
	{
		return { x * vec.x, x * vec.y };
	}

	bool operator==(const Vector2& vec1, const Vector2& vec2)
	{
		return vec1.x == vec2.x && vec1.y == vec2.y;
	}

	bool operator!=(const Vector2& vec1, const Vector2& vec2)
	{
		return !(vec1 == vec2);
	}

	Matrix3x3& Matrix3x3::operator*=(const float& a)
	{
		elem[0][0] *= a;	elem[0][1] *= a;	elem[0][2] *= a;
		elem[1][0] *= a;	elem[1][1] *= a;	elem[1][2] *= a;
		elem[2][0] *= a;	elem[2][1] *= a;	elem[2][2] *= a;
		return *this;
	}

	Matrix3x3& Matrix3x3::operator+=(const Matrix3x3& mat)
	{
		elem[0][0] += mat.elem[0][0];	elem[0][1] += mat.elem[0][1];	elem[0][2] += mat.elem[0][2];
		elem[1][0] += mat.elem[1][0];	elem[1][1] += mat.elem[1][1];	elem[1][2] += mat.elem[1][2];
		elem[2][0] += mat.elem[2][0];	elem[2][1] += mat.elem[2][1];	elem[2][2] += mat.elem[2][2];
		return *this;
	}

	Matrix3x3 Matrix3x3::identity()
	{
		Matrix3x3 res;
		res.elem[0][0] = 1;
		res.elem[1][1] = 1;
		res.elem[2][2] = 1;
		return res;
	}

	// euler XYZ
	Matrix3x3 Matrix3x3::get_rotation(float ang_x, float ang_y, float ang_z)
	{
		float cosax = cos(ang_x), sinax = sin(ang_x);
		float cosay = cos(ang_y), sinay = sin(ang_y);
		float cosaz = cos(ang_z), sinaz = sin(ang_z);

		Matrix3x3 rot;
		rot.elem[0][0] = cosay * cosaz;
		rot.elem[0][1] = sinax * sinay * cosaz - cosax * sinaz;
		rot.elem[0][2] = cosax * sinay * cosaz + sinax * sinaz;
		rot.elem[1][0] = cosay * sinaz;
		rot.elem[1][1] = sinax * sinay * sinaz + cosax * cosaz;
		rot.elem[1][2] = cosax * sinay * sinaz - sinax * cosaz;
		rot.elem[2][0] = -sinay;
		rot.elem[2][1] = sinax * cosay;
		rot.elem[2][2] = cosax * cosay;

		return rot;
	}

	template <class T>
	T _rotation_axis_angle(const Vector3& axis, float angle_rad)
	{
		if (angle_rad == 0 || axis.length() == 0)
			return T::identity();

		auto ax_norm = normalize(axis);
		// http://motion.pratt.duke.edu/RoboticSystems/3DRotations.html
		T cross_matrix;
		cross_matrix.elem[0][1] = -ax_norm.z;
		cross_matrix.elem[0][2] = ax_norm.y;
		cross_matrix.elem[1][0] = ax_norm.z;
		cross_matrix.elem[1][2] = -ax_norm.x;
		cross_matrix.elem[2][0] = -ax_norm.y;
		cross_matrix.elem[2][1] = ax_norm.x;

		T sq_cross_matrix = cross_matrix * cross_matrix;

		cross_matrix *= sinf(angle_rad);
		sq_cross_matrix *= (1 - cosf(angle_rad));

		cross_matrix += sq_cross_matrix;
		cross_matrix += T::identity();

		return cross_matrix;
	}

	Matrix3x3 Matrix3x3::rotation_angle_axis(const Vector3& axis, float angle_rad)
	{
		return _rotation_axis_angle<Matrix3x3>(axis, angle_rad);
	}

	Matrix4x4& Matrix4x4::operator*=(const float& a)
	{
		elem[0][0] *= a;	elem[0][1] *= a;	elem[0][2] *= a;	elem[0][3] *= a;
		elem[1][0] *= a;	elem[1][1] *= a;	elem[1][2] *= a;	elem[1][3] *= a;
		elem[2][0] *= a;	elem[2][1] *= a;	elem[2][2] *= a;	elem[2][3] *= a;
		elem[3][0] *= a;	elem[3][1] *= a;	elem[3][2] *= a;	elem[3][3] *= a;
		return *this;
	}

	Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& mat)
	{
		elem[0][0] += mat.elem[0][0];	elem[0][1] += mat.elem[0][1];	elem[0][2] += mat.elem[0][2];	elem[0][3] += mat.elem[0][3];
		elem[1][0] += mat.elem[1][0];	elem[1][1] += mat.elem[1][1];	elem[1][2] += mat.elem[1][2];	elem[1][3] += mat.elem[1][3];
		elem[2][0] += mat.elem[2][0];	elem[2][1] += mat.elem[2][1];	elem[2][2] += mat.elem[2][2];	elem[2][3] += mat.elem[2][3];
		elem[3][0] += mat.elem[3][0];	elem[3][1] += mat.elem[3][1];	elem[3][2] += mat.elem[3][2];	elem[3][3] += mat.elem[3][3];
		return *this;
	}

	Matrix4x4 Matrix4x4::identity() {
		Matrix4x4 res;
		res.elem[0][0] = 1;
		res.elem[1][1] = 1;
		res.elem[2][2] = 1;
		res.elem[3][3] = 1;
		return res;
	}

	Matrix4x4 Matrix4x4::rotation_x(float radians)
	{
		float sin = sinf(radians), cos = cosf(radians);
		Matrix4x4 res;
		res.elem[0][0] = 1;
		res.elem[3][3] = 1;
		res.elem[1][1] = cos;
		res.elem[2][2] = cos;
		res.elem[1][2] = -sin;
		res.elem[2][1] = sin;
		return res;
	}

	Matrix4x4 Matrix4x4::rotation_y(float radians)
	{
		float sin = sinf(radians), cos = cosf(radians);
		Matrix4x4 res;
		res.elem[1][1] = 1;
		res.elem[3][3] = 1;
		res.elem[0][0] = cos;
		res.elem[2][2] = cos;
		res.elem[2][0] = -sin;
		res.elem[0][2] = sin;
		return res;
	}

	Matrix4x4 Matrix4x4::rotation_z(float radians)
	{
		float sin = sinf(radians), cos = cosf(radians);
		Matrix4x4 res;
		res.elem[2][2] = 1;
		res.elem[3][3] = 1;
		res.elem[0][0] = cos;
		res.elem[1][1] = cos;
		res.elem[0][1] = -sin;
		res.elem[1][0] = sin;
		return res;
	}

	Matrix4x4 Matrix4x4::rotation_axis_angle(const Vector3& axis, float angle_rad)
	{
		return _rotation_axis_angle<Matrix4x4>(axis, angle_rad);
	}

	Matrix4x4 Matrix4x4::scale(float x, float y, float z)
	{
		Matrix4x4 res;
		res.elem[0][0] = x;
		res.elem[1][1] = y;
		res.elem[2][2] = z;
		res.elem[3][3] = 1;
		return res;
	}

	Matrix4x4 Matrix4x4::translation(const Vector3& vec)
	{
		Matrix4x4 res = Matrix4x4::identity();
		res.elem[0][3] = vec.x;
		res.elem[1][3] = vec.y;
		res.elem[2][3] = vec.z;
		return res;
	}

	Matrix4x4 Matrix4x4::translation_transposed(const Vector3& vec)
	{
		Matrix4x4 res = Matrix4x4::identity();
		res.elem[3][0] = vec.x;
		res.elem[3][1] = vec.y;
		res.elem[3][2] = vec.z;
		return res;
	}

	float Vector2::length() const
	{
		return sqrtf(x * x + y * y);
	}

	Vector2& Vector2::operator+=(const Vector2& vec)
	{
		x += vec.x;
		y += vec.y;
		return *this;
	}

	Vector2& Vector2::operator-=(const Vector2& vec)
	{
		x -= vec.x;
		y -= vec.y;
		return *this;
	}
}