#include "rough_path.h"
#include "renderer.h"

namespace ManualCAD
{
	RoughPath::RoughPath()
	{
		fbo.init();
		fbo.bind();
		texture.init();
		texture.bind();
		texture.configure();
		fbo.unbind();
	}
}
