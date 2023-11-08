#pragma once

#include "algebra.h"
#include "texture.h"
#include "frame_buffer.h"

namespace ManualCAD
{
	class RoughPath {
		FrameBuffer fbo;
		RenderTexture texture;
	public:
		RoughPath();
	};
}