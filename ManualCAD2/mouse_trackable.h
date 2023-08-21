#pragma once
#include "algebra.h"
#include "camera.h"

namespace ManualCAD
{
	class MouseTrackable {
	protected:
		Vector3 position = { 0.0f,0.0f,0.0f };
	public:
		virtual void move(const Vector3& direction) {
			position += direction;
		}
		virtual void set_world_position(const Vector3& position) {
			this->position = position;
		}
		void set_screen_position_from_pixels(float x, float y, const Camera& camera, int width, int height) {
			float z = get_screen_position(camera, width, height).z;
			set_screen_position({ 2.0f * x / width - 1.0f, 1.0f - 2 * y / height, z }, camera, width, height);
		}
		void set_screen_position(const Vector3& position, const Camera& camera, int width, int height) {
			set_world_position(camera.screen_to_world(position, width, height));
		}
		virtual const Vector3 get_world_position() const {
			return position;
		}
		Vector3 get_screen_position(const Camera& camera, int width, int height) const {
			return camera.world_to_screen(get_world_position(), width, height);
		}
		Vector2 get_screen_position_pixels(const Camera& camera, int width, int height) const {
			auto ndc = get_screen_position(camera, width, height);
			return { (ndc.x + 1.0f) * 0.5f * width, (1.0f - ndc.y) * 0.5f * height };
		}

		void set_screen_position_from_pixels_along_x_axis(float x, float y, const Camera& camera, int width, int height) {
			Vector3 cam_pos = camera.get_world_position();
			Vector3 this_pos = get_world_position();
			Vector3 world_vec = normalize(camera.screen_to_world({ 2.0f * x / width - 1.0f, 1.0f - 2.0f * y / height, 0.0f }, width, height) - cam_pos);
			if (world_vec.y == 0) return;
			float t = (this_pos.y - cam_pos.y) / world_vec.y; // cam_pos + t*world_vec crosses xz plane, in which object is
			set_world_position({ cam_pos.x + t * world_vec.x, this_pos.y, this_pos.z });
		}

		void set_screen_position_from_pixels_along_y_axis(float x, float y, const Camera& camera, int width, int height) {
			Vector3 cam_pos = camera.get_world_position();
			Vector3 this_pos = get_world_position();
			Vector3 world_vec = normalize(camera.screen_to_world({ 2.0f * x / width - 1.0f, 1.0f - 2.0f * y / height, 0.0f }, width, height) - cam_pos);
			if (world_vec.z == 0) return;
			float t = (this_pos.z - cam_pos.z) / world_vec.z; // cam_pos + t*world_vec crosses xy plane, in which object is
			set_world_position({ this_pos.x, cam_pos.y + t * world_vec.y, this_pos.z });
		}

		void set_screen_position_from_pixels_along_z_axis(float x, float y, const Camera& camera, int width, int height) {
			Vector3 cam_pos = camera.get_world_position();
			Vector3 this_pos = get_world_position();
			Vector3 world_vec = normalize(camera.screen_to_world({ 2.0f * x / width - 1.0f, 1.0f - 2.0f * y / height, 0.0f }, width, height) - cam_pos);
			if (world_vec.x == 0) return;
			float t = (this_pos.x - cam_pos.x) / world_vec.x; // cam_pos + t*world_vec crosses yz plane, in which object is
			set_world_position({ this_pos.x, this_pos.y, cam_pos.z + t * world_vec.z });
		}
	};
}