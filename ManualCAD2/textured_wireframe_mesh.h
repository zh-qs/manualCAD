#pragma once

#include <vector>
#include "algebra.h"
#include "renderable.h"
#include "texture.h"

namespace ManualCAD
{
	class TexturedWireframeMesh : public Renderable {
		//std::vector<Vector3> points;
		//std::vector<IndexPair> line_indices;
		ElementBuffer ebo;
		VertexBuffer texvbo;
		const Texture& texture;

		size_t point_count = 0, line_count = 0;

		static const Matrix4x4 IDENTITY;

		void init_additional_buffers() {
			texvbo.init();
			texvbo.bind();
			texvbo.attrib_buffer(1, 2, GL_FLOAT);
			ebo.init();
			ebo.bind();
			vao.unbind();
		}
	public:
		TexturedWireframeMesh(const Matrix4x4& model, const Texture& texture) : Renderable(model), ebo(), texvbo(), texture(texture) { init_additional_buffers(); }
		TexturedWireframeMesh(const Texture& texture) : TexturedWireframeMesh(Matrix4x4::identity(), texture) { init_additional_buffers(); }
		
		inline size_t get_point_count() const { return point_count; }
		inline size_t get_line_count() const { return line_count; }

		inline const Texture& get_texture() const { return texture; }

		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;
		void dispose() override { Renderable::dispose(); texvbo.dispose(); ebo.dispose(); }

		void set_data(const std::vector<Vector3>& points, const std::vector<Vector2>& tex_coords, const std::vector<IndexPair>& line_indices);
		void generate_torus(float large_radius, float small_radius, unsigned int divisions_x, unsigned int divisions_y);
	};
}