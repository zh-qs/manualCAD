#include "camera.h"
#include <stdio.h>
#include <cmath>
#include "application_settings.h"

namespace ManualCAD
{
	Matrix4x4 Camera::get_perspective_projection_matrix(int width, int height) const
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

	Matrix4x4 Camera::get_inverse_perspective_projection_matrix(int width, int height) const
	{
		// no eye_distance mentioned because we don't use that function in rendering (yet!)
		// TODO split inverse perspective for rendering (with eye_distance) and for UI interaction (w/o eye_distance)
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

	Matrix4x4 Camera::get_orthographic_projection_matrix(int width, int height) const
	{
		if (eye_distance == 0)
		{
			Matrix4x4 result;
			float inv_aspect = static_cast<float>(height) / width;
			result.elem[0][0] = 2.0f * inv_aspect;
			result.elem[1][1] = 2.0f;
			result.elem[2][2] = 2.0f / (far - near);
			result.elem[2][3] = -(far + near) / (far - near);
			result.elem[3][3] = 1.0f;
			return result;
		}

		Matrix4x4 result;
		float inv_aspect = static_cast<float>(height) / width;
		result.elem[0][0] = 2.0f * inv_aspect;
		result.elem[0][3] = eye_distance * 2.0f / focus_plane * 0.5f; // TODO is stereography in orthographic projection even sensible?
		result.elem[1][1] = 2.0f;
		result.elem[2][2] = 2.0f / (far - near);
		result.elem[2][3] = -(far + near) / (far - near);
		result.elem[3][3] = 1.0f;
		return result * Matrix4x4::translation({ -eye_distance * 0.5f,0.0f,0.0f });
	}

	Matrix4x4 Camera::get_inverse_orthographic_projection_matrix(int width, int height) const
	{
		// no eye_distance mentioned because we don't use that function in rendering (yet!)
		// TODO split inverse perspective for rendering (with eye_distance) and for UI interaction (w/o eye_distance)
		Matrix4x4 result;
		float aspect = static_cast<float>(width) / height;
		result.elem[0][0] = 0.5f * aspect;
		result.elem[1][1] = 0.5f;
		result.elem[2][2] = 0.5f * (far - near);
		result.elem[2][3] = 0.5f * (far + near);
		result.elem[3][3] = 1.0f;
		return result;
	}

	Matrix4x4 Camera::get_view_matrix() const {
		Matrix3x3 rot3 = Matrix3x3::get_rotation(rotx, roty, rotz);
		auto n_dir = scale.z * normalize(rot3 * direction);
		auto n_up = scale.y * normalize(rot3 * up);
		auto right = scale.x * normalize(cross(n_dir, n_up));
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
		auto n_dir = (1 / scale.z) * normalize(rot3 * direction);
		auto n_up = (1 / scale.y) * normalize(rot3 * up);
		auto right = (1 / scale.x) * normalize(cross(n_dir, n_up));
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
		auto n_dir = (1 / scale.x) * normalize(rot3 * direction);
		auto n_up = (1 / scale.y) * normalize(rot3 * up);
		auto right = (1 / scale.z) * normalize(cross(n_dir, n_up));
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
		switch (projection_type)
		{
		case ProjectionType::Perspective:
			return get_perspective_projection_matrix(width, height);
		case ProjectionType::Ortographic:
			return get_orthographic_projection_matrix(width, height);
		}
	}

	Matrix4x4 Camera::get_inverse_projection_matrix(int width, int height) const
	{
		switch (projection_type)
		{
		case ProjectionType::Perspective:
			return get_inverse_perspective_projection_matrix(width, height);
		case ProjectionType::Ortographic:
			return get_inverse_orthographic_projection_matrix(width, height);
		}
	}

	void Camera::move_by(const Vector3& vec)
	{
		Matrix3x3 rot3 = Matrix3x3::get_rotation(rotx, roty, rotz);
		auto n_dir = (1 / scale.x) * normalize(rot3 * direction);
		auto n_up = (1 / scale.y) * normalize(rot3 * up);
		auto right = (1 / scale.z) * normalize(cross(n_dir, n_up));

		target += vec.x * right + vec.y * n_up + vec.z * n_dir;
	}

	void Camera::zoom(float x, float y, float z)
	{
		// TODO make scaling more 3D (especially in zoom policy MultiplyDistance)
		if constexpr (ApplicationSettings::ZOOM_POLICY == ApplicationSettings::CameraZoomPolicy::MultiplyScale)
		{
			if (scale.x * x <= ApplicationSettings::MAX_ZOOM_SCALE && scale.x * x >= ApplicationSettings::MIN_ZOOM_SCALE)
			{
				scale.x *= x;
				scale.y *= y;
				scale.z *= z;
			}
		}
		else
		{
			if (distance_to_target * x <= ApplicationSettings::MAX_ZOOM_SCALE && distance_to_target * x >= ApplicationSettings::MIN_ZOOM_SCALE)
				distance_to_target *= x;
		}
	}

	void Camera::rotate(float ang_x_rad, float ang_y_rad, float ang_z_rad)
	{
		rotx += ang_x_rad;
		roty += ang_y_rad;
		rotz += ang_z_rad;
	}

	Vector3 Camera::get_rotation_deg() const
	{
		return (180.0f / PI) * Vector3 { rotx, roty, rotz };
	}

	Vector3 Camera::screen_to_world(const Vector3& screen, int width, int height) const
	{
		float w = projection_type == ProjectionType::Perspective ? (-2.0f * far * near / (screen.z * (far - near) - far - near)) : 1.0f;
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
		Vector3 invscale = { 1.0f / scale.x, 1.0f / scale.y,1.0f / scale.z };
		return target - distance_to_target * invscale * (rot3 * direction);
	}

	Ray Camera::get_ray_to(const Vector3& world_pos) const
	{
		switch (projection_type)
		{
		case ProjectionType::Perspective:
		{
			auto cam_pos = get_world_position();
			return { cam_pos, normalize(world_pos - cam_pos) };
		}
		case ProjectionType::Ortographic:
		{
			Matrix3x3 rot3 = Matrix3x3::get_rotation(rotx, roty, rotz);
			auto dir = rot3 * direction;
			Vector3 invscale = { 1.0f / scale.x, 1.0f / scale.y,1.0f / scale.z };
			auto cam_pos = target - distance_to_target * invscale * dir; // copied from Camera::get_world_position (in order not to generate rotation matrix twice)

			auto n_dir = normalize(dir);
			return { world_pos + dot(cam_pos - world_pos, n_dir) * n_dir, n_dir };
		}
		}
	}

	Camera::Camera() {
		direction = { 0,0,1 };
		target = { 0,0,0 };
		up = { 0,1,0 };
	}

}