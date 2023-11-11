#pragma once

#include "algebra.h"
#include "texture.h"
#include "frame_buffer.h"
#include "object.h"
#include "renderer.h"
#include <list>

namespace ManualCAD
{
	class RoughPath {
		static constexpr int TEX_DIM = 300;

		Box box;
		float bottom_height;

		const std::list<const ParametricSurfaceObject*>& surfaces;
		HeightMap render_height_map(const Vector3& size);
	public:
		RoughPath(const std::list<const ParametricSurfaceObject*>& surfaces, const Vector3& center, const Vector2& min, const Vector2& max, const float bottom_height, const float height, const float scale);

		std::vector<Vector3> generate_path(int levels, const Vector3& size, const float radius, const float r_epsilon, const float h_epsilon);
	};
}