#pragma once
#include "algebra.h"
#include "renderable.h"

namespace ManualCAD
{
	class AxesCursor : public Renderable {
		static constexpr unsigned int COORDS_SIZE = 6;
		Vector3 coords[COORDS_SIZE] = { {1.0f,0.0f,0.0f}, {1.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f}, {0.0f,1.0f,0.0f}, {0.0f,0.0f,1.0f}, {0.0f,0.0f,1.0f} };
	public:
		AxesCursor() : Renderable() {
			vbo.bind();
			vbo.set_static_data(reinterpret_cast<const float*>(coords), sizeof(coords));
			vao.unbind();
		}
		inline constexpr unsigned int get_vertex_count() const { return COORDS_SIZE; }
		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;
	};
}