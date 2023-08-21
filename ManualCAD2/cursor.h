#pragma once

#include "algebra.h"
#include "axes_cursor.h"
#include "camera.h"
#include "mouse_trackable.h"

namespace ManualCAD
{
	class Cursor : public MouseTrackable {
		AxesCursor ac;
	public:
		explicit Cursor() : MouseTrackable() {}
		inline const AxesCursor& get_renderable() { ac.set_model_matrix(Matrix4x4::translation(get_world_position()) * Matrix4x4::scale(0.3f, 0.3f, 0.3f)); return ac; }
	};
}