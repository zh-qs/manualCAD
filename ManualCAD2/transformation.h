#pragma once
#include "algebra.h"

namespace ManualCAD
{
	class Transformation {
	public:
		Vector3 position = { 0.0f,0.0f,0.0f };
		Vector3 scale = { 1.0f,1.0f,1.0f };

		Vector3 rotation_vector = { 0.0f,0.0f,0.0f }; // axis-angle
		float rotation_angle_deg = 0.0f;

		Matrix4x4 get_matrix() const {
			return Matrix4x4::translation(position) * Matrix4x4::rotation_axis_angle(rotation_vector, (PI / 180) * rotation_angle_deg) * Matrix4x4::scale(scale.x, scale.y, scale.z);
		}

		Matrix4x4 get_inversed_transposed_matrix() const {
			return Matrix4x4::translation_transposed(-position) * Matrix4x4::rotation_axis_angle(rotation_vector, (PI / 180) * rotation_angle_deg) * Matrix4x4::scale(1 / scale.x, 1 / scale.y, 1 / scale.z);
		}

		Matrix4x4 get_matrix_combined_with(const Transformation& t, const Vector3& rotation_center) const {
			return Matrix4x4::translation(rotation_center + t.position)
				* Matrix4x4::rotation_axis_angle(t.rotation_vector, (PI / 180) * t.rotation_angle_deg)
				* Matrix4x4::translation(-rotation_center + (Vector3{ 1.0f,1.0f,1.0f } - t.scale) * (rotation_center - position))
				* get_matrix()
				* Matrix4x4::scale(t.scale.x, t.scale.y, t.scale.z);
		}

		void combine_with(const Transformation& t, const Vector3& rotation_center) {
			// combine scale
			scale *= t.scale;

			float rotation_angle_rad = (PI / 180) * rotation_angle_deg;
			float t_rotation_angle_rad = (PI / 180) * t.rotation_angle_deg;

			// combine positions (scale first, then rotate and translate)
			position = (Vector3{ 1.0f,1.0f,1.0f } - t.scale) * rotation_center + t.scale * position; // scale
			position = Matrix3x3::rotation_angle_axis(t.rotation_vector, t_rotation_angle_rad) * (position - rotation_center) + rotation_center; // rotate
			position += t.position; // translate

			// combine rotations: https://i.imgur.com/Mdc2AV3.jpg

			if (t.rotation_vector.length() == 0) {
				return;
			}
			if (rotation_vector.length() == 0) {
				rotation_angle_deg = t.rotation_angle_deg;
				rotation_vector = t.rotation_vector;
				return;
			}

			auto rot_n = normalize(rotation_vector), t_rot_n = normalize(t.rotation_vector);

			if (rot_n == t_rot_n)
			{
				rotation_angle_deg += t.rotation_angle_deg;
				return;
			}
			if (rot_n == -1.0f * t_rot_n)
			{
				rotation_angle_deg -= t.rotation_angle_deg;
				return;
			}

			float sin_half_a = sinf(t_rotation_angle_rad * 0.5f), cos_half_a = cosf(t_rotation_angle_rad * 0.5f),
				sin_half_b = sinf(rotation_angle_rad * 0.5f), cos_half_b = cosf(rotation_angle_rad * 0.5f);

			float angle_rad = 2.0f * acosf(cos_half_a * cos_half_b - sin_half_a * sin_half_b * dot(rot_n, t_rot_n));
			auto axis = sin_half_a * cos_half_b * t_rot_n + cos_half_a * sin_half_b * rot_n + sin_half_a * sin_half_b * normalize(cross(t_rot_n, rot_n));

			// TODO mo¿e da siê dok³adniej numerycznie
			// TODO sin == 0 (cos == 1) case

			rotation_angle_deg = angle_rad * 180 / PI;
			rotation_vector = normalize(axis);
		}

		Vector3 get_euler_rotation_deg() const {
			Matrix3x3 rot_mat = Matrix3x3::rotation_angle_axis(rotation_vector, (PI / 180) * rotation_angle_deg);
			Vector3 rot;
			if (rot_mat.elem[2][0] == 1.0f)
			{
				rot.y = -HALF_PI;
				rot.x = 0.0f;
				rot.z = atan2f(-rot_mat.elem[0][1], -rot_mat.elem[0][2]);
			}
			else if (rot_mat.elem[2][0] == -1.0f)
			{
				rot.y = HALF_PI;
				rot.x = 0.0f;
				rot.z = atan2f(rot_mat.elem[0][1], rot_mat.elem[0][2]);
			}
			else
			{
				rot.y = -asinf(rot_mat.elem[2][0]);
				rot.x = atan2f(rot_mat.elem[2][1], rot_mat.elem[2][2]);
				rot.z = atan2f(rot_mat.elem[1][0], rot_mat.elem[0][0]);
			}
			return (180.0f / PI) * rot;
		}

		void set_rotation_from_euler_deg(const Vector3& rot) {
			Matrix3x3 rot_mat = Matrix3x3::get_rotation(PI / 180.0f * rot.x, PI / 180.0f * rot.y, PI / 180.0f * rot.z);
			rotation_angle_deg = (180.0f / PI) * acosf(0.5f * (rot_mat.trace() - 1));
			Vector3 axis{ rot_mat.elem[2][1] - rot_mat.elem[1][2], rot_mat.elem[0][2] - rot_mat.elem[2][0],rot_mat.elem[1][0] - rot_mat.elem[0][1] };
			if (axis.length() == 0.0f) // sin == 0
			{
				if (rot_mat.trace() - 1 > 0.0f) // cos == 1 and angle = 0
					rotation_vector = { 0.0f,0.0f,0.0f };
				else
					rotation_vector = { sqrtf(0.5f * (rot_mat.elem[0][0] + 1)), sqrtf(0.5f * (rot_mat.elem[1][1] + 1)), sqrtf(0.5f * (rot_mat.elem[2][2] + 1)) };
			}
			else
			{
				rotation_vector = normalize(axis);
			}
		}

		void reset() {
			position = { 0.0f,0.0f,0.0f };
			scale = { 1.0f,1.0f,1.0f };
			rotation_vector = { 0.0f,0.0f,0.0f };
			rotation_angle_deg = 0.0f;
		}
	};
}