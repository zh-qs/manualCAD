#include "axes_cursor.h"
#include "renderer.h"

namespace ManualCAD
{
	void AxesCursor::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_axes_cursor(*this, width, height, thickness);
	}
}
