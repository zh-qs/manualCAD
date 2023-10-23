#pragma once

#include <vector>
#include "algebra.h"
#include "drawable.h"
#include "texture.h"

namespace ManualCAD
{
	class TexturedWireframeMesh : public Drawable {
		//std::vector<Vector3> points;
		//std::vector<IndexPair> line_indices;
		ElementBuffer ebo;
		VertexBuffer texvbo;
		const Texture& texture;

		size_t point_count = 0, line_count = 0;

		static const Matrix4x4 IDENTITY;
	public:
		TexturedWireframeMesh(const Matrix4x4& model, const Texture& texture) : Drawable(model), ebo(), texvbo(), texture(texture) { 
			texvbo.init();
			texvbo.bind();
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			ebo.init(); 
			ebo.bind(); 
		}
		TexturedWireframeMesh(const Texture& texture) : TexturedWireframeMesh(Matrix4x4::identity(), texture) {}
		
		inline size_t get_point_count() const { return point_count; }
		inline size_t get_line_count() const { return line_count; }

		inline const Texture& get_texture() const { return texture; }

		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;
		void dispose() override { Drawable::dispose(); texvbo.dispose(); ebo.dispose(); }

		void set_data(const std::vector<Vector3>& points, const std::vector<Vector2>& tex_coords, const std::vector<IndexPair>& line_indices);
		void generate_torus(float large_radius, float small_radius, unsigned int divisions_x, unsigned int divisions_y);
	};
}