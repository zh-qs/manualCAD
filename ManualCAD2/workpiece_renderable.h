#pragma once

#include "drawable.h"
#include "texture.h"
#include "height_map.h"
#include "milling_program.h"
#include "triangle_mesh.h"
#include "line.h"

namespace ManualCAD {
	class WorkpieceRenderable : public Drawable {

		TexMap texture;
		ElementBuffer ebo;

		const Line& line;
		const TriangleMesh& cutter_mesh;

		int divisions_x = 0, divisions_y = 0;
		size_t indices_count = 0;

	public:
		const Vector3& parent_size;

		WorkpieceRenderable(const Vector3& parent_size, const Line& line, const TriangleMesh& cutter_mesh) : Drawable(2), parent_size(parent_size), texture(), ebo(), line(line), cutter_mesh(cutter_mesh) {
			texture.init();
			texture.bind();
			//texture.configure();
			texture.configure(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

			ebo.init();
			ebo.bind();
		}

		void set_data_from_map(HeightMap& height_map);

		size_t get_indices_count() const { return indices_count; }
		const TexMap& get_texture() const { return texture; }

		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;
		void dispose() override { Drawable::dispose(); ebo.dispose(); }
	};
}