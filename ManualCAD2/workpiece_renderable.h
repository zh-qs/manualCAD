#pragma once

#include "renderable.h"
#include "texture.h"
#include "height_map.h"
#include "milling_program.h"
#include "triangle_mesh.h"
#include "line.h"

namespace ManualCAD {
	class WorkpieceRenderable : public Renderable {

		TexMap texture;
		ElementBuffer ebo;

		const Line& line;
		const TriangleMesh& cutter_mesh;

		int divisions_x = 0, divisions_y = 0;
		size_t indices_count = 0;

		void init_additional_buffers() {
			texture.init();
			texture.bind();
			//texture.configure();
			texture.configure(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

			ebo.init();
			ebo.bind();

			vao.unbind();
		}
	public:
		const Vector3& parent_size;

		WorkpieceRenderable(const Vector3& parent_size, const Line& line, const TriangleMesh& cutter_mesh) : Renderable(2), parent_size(parent_size), texture(), ebo(), line(line), cutter_mesh(cutter_mesh) {
			init_additional_buffers();
		}

		void set_data_from_map(HeightMap& height_map);

		size_t get_indices_count() const { return indices_count; }
		const TexMap& get_texture() const { return texture; }
		Vector2 get_uv_offset() const { return { 1.0f / (divisions_x - 1), 1.0f / (divisions_y - 1) }; }

		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;
		void dispose() override { Renderable::dispose(); texture.dispose(); ebo.dispose(); }
	};
}