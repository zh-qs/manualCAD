#pragma once

#include <list>
#include "algebra.h"
#include "frame_buffer.h"
#include "texture.h"
#include "renderer.h"
#include "object.h"
#include "height_map.h"
#include "box.h"

namespace ManualCAD
{
	class HeightMapRenderer {
		static constexpr int TEX_DIM = 2000; // TODO 1000 suffices

		FrameBuffer fbo;
		RenderTexMap texture;
		Renderer renderer; // doesn't have to be initialized

		const std::list<const ParametricSurfaceObject*>& surfaces;
	public:
		HeightMapRenderer(const std::list<const ParametricSurfaceObject*>& surfaces, const Box& box);
		~HeightMapRenderer();
		HeightMap render_height_map(const Vector3& size);
		HeightMap render_index_map(const Vector3& size);

		const auto& get_texture() const { return texture; }
	};
}