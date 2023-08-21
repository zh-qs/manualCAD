#pragma once
#include "algebra.h"
#include "property_handle.h"

namespace ManualCAD
{
	class Serializer;

	class Camera {
		friend class Serializer;

		Vector3 target;
		Vector3 direction;
		Vector3 up;

		float rotx = 0.25f * PI, roty = 0.25f * PI, rotz = 0.0f;
		float scale = 1;
		float distance_to_target = 5.0f;
	public:
		float near = 0.1f, far = 100.0f;
		float fov_rad = 0.25f * PI;

		float specular_exponent = 2;

		float eye_distance = 0, focus_plane = 10.0f;

		Matrix4x4 get_view_matrix() const;
		Matrix4x4 get_inverse_view_matrix() const;
		Matrix4x4 get_projection_matrix(int width, int height) const;
		Matrix4x4 get_inverse_projection_matrix(int width, int height) const;
		Matrix4x4 get_bilinear_form_transformation_matrix() const;
		void move_by(const Vector3& vec);
		void zoom(float factor);
		void rotate(float ang_x, float ang_y, float ang_z);
		float get_scale() const { return scale; }
		Vector3 get_rotation_deg() const;
		void reset_scale() { scale = 1.0f; }
		Vector3 screen_to_world(const Vector3& screen, int width, int height) const;
		Vector3 world_to_screen(const Vector3& world, int width, int height) const;
		Vector3 get_world_position() const;
		void look_at(const Vector3& vector) { target = vector; }
		Camera();

		PropertyHandle<float> get_specular_exponent_handle() { return &specular_exponent; }
		PropertyHandle<float> get_screen_to_target_distance_handle() { return &distance_to_target; }
	};
}