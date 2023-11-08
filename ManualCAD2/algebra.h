#pragma once

#include <type_traits>
#include <utility>
#include <algorithm>

namespace ManualCAD
{
	constexpr float PI = 3.141592653589793238462643383279502884f;
	constexpr float TWO_PI = 2.0f * PI;
	constexpr float HALF_PI = 0.5f * PI;

	struct Vector2 {
		float x, y;
		float length() const;
		float lengthsq() const;
		Vector2 normalize() const;

		Vector2& operator+=(const Vector2& vec);
		Vector2& operator-=(const Vector2& vec);
		Vector2& operator*=(const float& a);

		inline float* data() { return reinterpret_cast<float*>(this); }
	};

	struct Vector3 {
		float x, y, z;
		float length() const;
		bool is_valid() const;

		Vector3& operator*=(const float& a);
		Vector3& operator/=(const float& a);
		Vector3& operator+=(const Vector3& vec);
		Vector3& operator-=(const Vector3& vec);
		Vector3& operator*=(const Vector3& vec);

		inline float* data() { return reinterpret_cast<float*>(this); }

		inline static const Vector3 nan() { return { NAN,NAN,NAN }; }
	};

	struct Vector4 {
		float x, y, z, w;
		float length() const;

		Vector4& operator/=(const float& a);
		Vector4& operator+=(const Vector4& vec);

		inline float* data() { return reinterpret_cast<float*>(this); }
		inline void swap_elements(int i, int j) {
			float* d = data();
			std::swap(d[i], d[j]);
		}

		inline Vector3 xyz() const {
			return { x,y,z };
		}

		inline static const Vector4 extend(const Vector3& vec, float w) { return { vec.x,vec.y,vec.z,w }; }
	};

	struct IndexPair {
		unsigned int i, j;
		inline unsigned int* data() { return reinterpret_cast<unsigned int*>(this); }
	};

	struct IndexTriple {
		unsigned int i, j, k;
		inline unsigned int* data() { return reinterpret_cast<unsigned int*>(this); }
	};

	static_assert(std::is_pod_v<Vector4>, "Vector4 needs to be POD, but it's not");
	static_assert(std::is_pod_v<Vector3>, "Vector3 needs to be POD, but it's not");
	static_assert(std::is_pod_v<Vector2>, "Vector2 needs to be POD, but it's not");
	static_assert(std::is_pod_v<IndexPair>, "IndexPair needs to be POD, but it's not");
	static_assert(std::is_pod_v<IndexTriple>, "IndexTriple needs to be POD, but it's not");

	struct Matrix3x3 {
		static const int DIMENSION = 3;
		float elem[DIMENSION][DIMENSION];
		Matrix3x3() : elem{ {0,0,0},{0,0,0},{0,0,0} } {}

		Matrix3x3& operator*=(const float& a);
		Matrix3x3& operator+=(const Matrix3x3& mat);

		float trace() const { return elem[0][0] + elem[1][1] + elem[2][2]; }

		static Matrix3x3 identity();
		static Matrix3x3 get_rotation(float ang_x, float ang_y, float ang_z);
		static Matrix3x3 rotation_angle_axis(const Vector3& axis, float angle_rad);
	};

	struct Matrix4x4 {
		static const int DIMENSION = 4;
		float elem[DIMENSION][DIMENSION];

		Matrix4x4() : elem{ {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0} } {}
		Matrix4x4(const Vector3& v1, const Vector3& v2, const Vector3& v3) : elem{ {v1.x,v1.y,v1.z,0},{v2.x,v2.y,v2.z,0},{v3.x,v3.y,v3.z,0},{0,0,0,0} } {}

		Matrix4x4& operator*=(const float& a);
		Matrix4x4& operator+=(const Matrix4x4& mat);

		inline void swap_rows(int i, int j) {
			std::swap(elem[i][0], elem[j][0]);
			std::swap(elem[i][1], elem[j][1]);
			std::swap(elem[i][2], elem[j][2]);
			std::swap(elem[i][3], elem[j][3]);
		}

		static Matrix4x4 identity();
		static Matrix4x4 rotation_x(float radians);
		static Matrix4x4 rotation_y(float radians);
		static Matrix4x4 rotation_z(float radians);
		static Matrix4x4 rotation_axis_angle(const Vector3& axis, float angle_rad);
		static Matrix4x4 scale(float x, float y, float z);
		static Matrix4x4 translation(const Vector3& vec);
		static Matrix4x4 translation_transposed(const Vector3& vec);
	};

	struct GLColumnOrderMatrix4x4 {
		float elem[Matrix4x4::DIMENSION * Matrix4x4::DIMENSION];

		GLColumnOrderMatrix4x4(const Matrix4x4& mat) : elem{
			mat.elem[0][0], mat.elem[1][0], mat.elem[2][0],mat.elem[3][0],
			mat.elem[0][1], mat.elem[1][1], mat.elem[2][1],mat.elem[3][1],
			mat.elem[0][2], mat.elem[1][2], mat.elem[2][2],mat.elem[3][2],
			mat.elem[0][3], mat.elem[1][3], mat.elem[2][3],mat.elem[3][3]
		} {}
	};

	template <class T>
	struct Range {
		T from, to;

		T middle() { return (from + to) / 2; }
		T clamp(T val) { return std::clamp(val, from, to); }
		T wrap(T val) {
			const auto res = fmodf(val - from, to - from);
			if (res < 0)
				return to - from + res;
			return res;
			/*if (val < from) return val + to - from;
			if (val > to) return val - to + from;
			return val;*/
		}
		bool contains(T val) { return val >= from && val <= to; }
	};

	Vector4 operator*(const Matrix4x4& mat, const Vector4& vec);
	Vector4 operator*(const float& x, const Vector4& vec);
	Vector4 operator+(const Vector4& vec1, const Vector4& vec2);
	Vector4 operator-(const Vector4& vec1, const Vector4& vec2);
	Vector4 operator-(const Vector4& vec);
	bool operator==(const Vector4& vec1, const Vector4& vec2);
	bool operator!=(const Vector4& vec1, const Vector4& vec2);

	Matrix3x3 operator*(const Matrix3x3& mat1, const Matrix3x3& mat2);
	Matrix4x4 operator*(const Matrix4x4& mat1, const Matrix4x4& mat2);

	Matrix4x4 mul_with_first_transposed(const Matrix4x4& mat1, const Matrix4x4& mat2);

	float dot(const Vector4& vec1, const Vector4& vec2);
	float dot(const Vector3& vec1, const Vector3& vec2);
	float dot(const Vector2& vec1, const Vector2& vec2);

	Vector3 cross(const Vector3& vec1, const Vector3& vec2);

	Vector3 normalize(const Vector3& vec);

	Vector3 operator+(const Vector3& vec1, const Vector3& vec2);
	Vector3 operator-(const Vector3& vec1, const Vector3& vec2);
	Vector3 operator-(const Vector3& vec);
	Vector3 operator*(const float& x, const Vector3& vec);
	Vector3 operator/(const Vector3& vec, const float& x);
	Vector3 operator*(const Vector3& vec1, const Vector3& vec2);
	bool operator==(const Vector3& vec1, const Vector3& vec2);
	bool operator!=(const Vector3& vec1, const Vector3& vec2);

	Vector3 operator*(const Matrix3x3& mat, const Vector3& vec);

	float cross_sign(const Vector2& vec1, const Vector2& vec2);
	Vector2 normalize(const Vector2& vec);
	Vector2 max(const Vector2& vec1, const Vector2& vec2);
	Vector2 min(const Vector2& vec1, const Vector2& vec2);

	Vector2 operator+(const Vector2& vec1, const Vector2& vec2);
	Vector2 operator-(const Vector2& vec1, const Vector2& vec2);
	Vector2 operator-(const Vector2& vec);
	Vector2 operator*(const float& x, const Vector2& vec);
	bool operator==(const Vector2& vec1, const Vector2& vec2);
	bool operator!=(const Vector2& vec1, const Vector2& vec2);

	Matrix3x3 transpose(const Matrix3x3& mat);
	Matrix4x4 transpose(const Matrix4x4& mat);

	template <class V, class Number>
	inline constexpr V lerp(const V& from, const V& to, const Number& percent) {
		Number p = percent > 1 ? 1 : (percent < 0 ? 0 : percent);
		return (1 - p) * from + p * to;
	}
}