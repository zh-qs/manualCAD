#include "camera.h"
#include <stdio.h>
#include <cmath>
#include "application_settings.h"

namespace ManualCAD
{
	Matrix4x4 Camera::get_view_matrix() const {
		Matrix3x3 rot3 = Matrix3x3::get_rotation(rotx, roty, rotz);
		auto n_dir = scale * normalize(rot3 * direction);
		auto n_up = scale * normalize(rot3 * up);
		auto right = scale * normalize(cross(n_dir, n_up));
		Matrix4x4 rot4(right, n_up, n_dir);
		rot4.elem[3][3] = 1;

		Matrix4x4 trt, trd;
		trt.elem[0][0] = 1;
		trt.elem[1][1] = 1;
		trt.elem[2][2] = 1;
		trt.elem[3][3] = 1;

		trt.elem[0][3] = -target.x;
		trt.elem[1][3] = -target.y;
		trt.elem[2][3] = -target.z;

		trd.elem[0][0] = 1;
		trd.elem[1][1] = 1;
		trd.elem[2][2] = 1;
		trd.elem[3][3] = 1;

		/*trd.elem[0][3] = direction.x;
		trd.elem[1][3] = direction.y;
		trd.elem[2][3] = direction.z;*/
		trd.elem[0][3] = 0;
		trd.elem[1][3] = 0;
		trd.elem[2][3] = distance_to_target;

		return trd * rot4 * trt;
	}

	Matrix4x4 Camera::get_inverse_view_matrix() const
	{
		Matrix3x3 rot3 = Matrix3x3::get_rotation(rotx, roty, rotz);
		auto n_dir = (1 / scale) * normalize(rot3 * direction);
		auto n_up = (1 / scale) * normalize(rot3 * up);
		auto right = (1 / scale) * normalize(cross(n_dir, n_up));
		Matrix4x4 rot4(right, n_up, n_dir);
		rot4.elem[3][3] = 1;
		Matrix4x4 rot4_inv = transpose(rot4);

		Matrix4x4 trt, trd;
		trt.elem[0][0] = 1;
		trt.elem[1][1] = 1;
		trt.elem[2][2] = 1;
		trt.elem[3][3] = 1;

		trt.elem[0][3] = target.x;
		trt.elem[1][3] = target.y;
		trt.elem[2][3] = target.z;

		trd.elem[0][0] = 1;
		trd.elem[1][1] = 1;
		trd.elem[2][2] = 1;
		trd.elem[3][3] = 1;

		/*trd.elem[0][3] = -direction.x;
		trd.elem[1][3] = -direction.y;
		trd.elem[2][3] = -direction.z;*/
		trd.elem[0][3] = 0;
		trd.elem[1][3] = 0;
		trd.elem[2][3] = -distance_to_target;

		return trt * rot4_inv * trd;
	}

	Matrix4x4 Camera::get_bilinear_form_transformation_matrix() const
	{
		Matrix3x3 rot3 = transpose(Matrix3x3::get_rotation(rotx, -roty, rotz));
		auto n_dir = (1 / scale) * normalize(rot3 * direction);
		auto n_up = (1 / scale) * normalize(rot3 * up);
		auto right = (1 / scale) * normalize(cross(n_dir, n_up));
		Matrix4x4 rot4(right, n_up, n_dir);
		rot4.elem[3][3] = 1;

		Matrix4x4 trt, trd;
		trt.elem[0][0] = 1;
		trt.elem[1][1] = 1;
		trt.elem[2][2] = 1;
		trt.elem[3][3] = 1;

		trt.elem[0][3] = target.x;
		trt.elem[1][3] = target.y;
		trt.elem[2][3] = target.z;

		trd.elem[0][0] = 1;
		trd.elem[1][1] = 1;
		trd.elem[2][2] = 1;
		trd.elem[3][3] = 1;

		/*trd.elem[0][3] = -direction.x;
		trd.elem[1][3] = -direction.y;
		trd.elem[2][3] = -direction.z;*/
		trd.elem[0][3] = 0;
		trd.elem[1][3] = 0;
		trd.elem[2][3] = -distance_to_target;

		return trt * rot4 * trd;
	}

	Matrix4x4 Camera::get_projection_matrix(int width, int height) const
	{
		if (eye_distance == 0)
		{
			Matrix4x4 result;
			float ctgfov2 = 1 / tanf(0.5f * fov_rad);
			float inv_aspect = static_cast<float>(height) / width;
			result.elem[0][0] = ctgfov2 * inv_aspect;
			result.elem[1][1] = ctgfov2;
			result.elem[2][2] = (far + near) / (far - near);
			result.elem[2][3] = -2.0f * far * near / (far - near);
			result.elem[3][2] = 1.0f;
			return result;
		}

		Matrix4x4 result;
		float ctgfov2 = 1 / tanf(0.5f * fov_rad);
		float inv_aspect = static_cast<float>(height) / width;
		result.elem[0][0] = ctgfov2 * inv_aspect;
		result.elem[0][2] = eye_distance * ctgfov2 / focus_plane * 0.5f;
		result.elem[1][1] = ctgfov2;
		result.elem[2][2] = (far + near) / (far - near);
		result.elem[2][3] = -2.0f * far * near / (far - near);
		result.elem[3][2] = 1.0f;
		return result * Matrix4x4::translation({ -eye_distance * 0.5f,0.0f,0.0f });
	}

	Matrix4x4 Camera::get_inverse_projection_matrix(int width, int height) const
	{
		Matrix4x4 result;
		float tgfov2 = tanf(0.5f * fov_rad);
		float aspect = static_cast<float>(width) / height;
		result.elem[0][0] = tgfov2 * aspect;
		result.elem[1][1] = tgfov2;
		result.elem[2][3] = 1;
		result.elem[3][2] = (near - far) / (2 * far * near);
		result.elem[3][3] = (far + near) / (2 * far * near);
		return result;
	}

	void Camera::move_by(const Vector3& vec)
	{
		Matrix3x3 rot3 = Matrix3x3::get_rotation(rotx, roty, rotz);
		auto n_dir = (1 / scale) * normalize(rot3 * direction);
		auto n_up = (1 / scale) * normalize(rot3 * up);
		auto right = (1 / scale) * normalize(cross(n_dir, n_up));

		target += vec.x * right + vec.y * n_up + vec.z * n_dir;
	}

	void Camera::zoom(float factor)
	{
		if constexpr (ApplicationSettings::ZOOM_POLICY == ApplicationSettings::CameraZoomPolicy::MultiplyScale)
		{
			if (scale * factor <= ApplicationSettings::MAX_ZOOM_SCALE && scale * factor >= ApplicationSettings::MIN_ZOOM_SCALE)
				scale *= factor;
		}
		else
		{
			if (distance_to_target * factor <= ApplicationSettings::MAX_ZOOM_SCALE && distance_to_target * factor >= ApplicationSettings::MIN_ZOOM_SCALE)
				distance_to_target *= factor;
		}
	}

	void Camera::rotate(float ang_x, float ang_y, float ang_z)
	{
		rotx += ang_x;
		roty += ang_y;
		rotz += ang_z;
	}

	Vector3 Camera::get_rotation_deg() const
	{
		return (180.0f / PI) * Vector3{ rotx, roty, rotz };
	}

	Vector3 Camera::screen_to_world(const Vector3& screen, int width, int height) const
	{
		float w = -2.0f * far * near / (screen.z * (far - near) - far - near);
		Vector4 scr4{ screen.x * w, screen.y * w,screen.z * w, w };
		Vector4 res4 = get_inverse_view_matrix() * (get_inverse_projection_matrix(width, height) * scr4);
		return { res4.x, res4.y, res4.z };
	}

	Vector3 Camera::world_to_screen(const Vector3& world, int width, int height) const
	{
		Vector4 src4 = Vector4::extend(world, 1.0f);
		Vector4 res4 = get_projection_matrix(width, height) * (get_view_matrix() * src4);
		return { res4.x / res4.w,res4.y / res4.w,res4.z / res4.w };
	}

	Vector3 Camera::get_world_position() const
	{
		Matrix3x3 rot3 = Matrix3x3::get_rotation(rotx, roty, rotz);
		return target - (distance_to_target / scale) * (rot3 * direction);
	}

	Camera::Camera() {
		direction = { 0,0,1 };
		target = { 0,0,0 };
		up = { 0,1,0 };
	}

}