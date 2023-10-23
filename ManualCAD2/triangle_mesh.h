#pragma once

#include <vector>
#include "algebra.h"
#include "drawable.h"

namespace ManualCAD
{
	class TriangleMesh : public Drawable {
		ElementBuffer ebo;
		VertexBuffer normal_vbo;

		size_t point_count = 0, triangle_count = 0;

		static const Matrix4x4 IDENTITY;
	public:
		TriangleMesh(const Matrix4x4& model) : Drawable(model), ebo(), normal_vbo() { 
			normal_vbo.init();
			normal_vbo.bind();
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			ebo.init(); 
			ebo.bind(); 
		}
		TriangleMesh() : Drawable(), ebo(), normal_vbo() {
			normal_vbo.init();
			normal_vbo.bind();
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			ebo.init();
			ebo.bind();
		}

		inline size_t get_point_count() const { return point_count; }
		inline size_t get_triangle_count() const { return triangle_count; }

		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;
		void dispose() override { Drawable::dispose(); normal_vbo.dispose();  ebo.dispose(); }

		void set_data(const std::vector<Vector3>& points, const std::vector<Vector3>& normals, const std::vector<IndexTriple>& triangle_indices);
		void generate_cylinder(float radius, float height, unsigned int circle_divisions);
		void generate_bottom_capsule(float radius, float height, unsigned int circle_divisions);
	};
}