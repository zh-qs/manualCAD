#include "bilinear_form_raycastable.h"
#include "renderer.h"

namespace ManualCAD 
{
	void BilinearFormRaycastable::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_bilinear_form_raycastable(*this, color, width, height, thickness);
	}
}


