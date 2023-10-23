#include "workpiece_renderable.h"
#include "renderer.h"

namespace ManualCAD
{
	void WorkpieceRenderable::set_data_from_map(HeightMap& height_map)
	{
		texture.bind();
		texture.set_image(height_map.size_x, height_map.size_y, height_map.data());

		if (divisions_x != height_map.size_x || divisions_y != height_map.size_y)
		{
			// generate workpiece model
			divisions_x = height_map.size_x;
			divisions_y = height_map.size_y;

			//std::vector<Vector3> vertices((divisions_x + 2) * (divisions_y + 2));
			std::vector<Vector2> uvs((divisions_x + 2) * (divisions_y + 2));
			std::vector<unsigned int> indices;
			indices.reserve(6 * (divisions_x + 1) * (divisions_y + 1));

			for (int y = 0; y < divisions_y + 2; ++y)
			{
				for (int x = 0; x < divisions_x + 2; ++x)
				{
					uvs[x + y * (divisions_x + 2)] = { static_cast<float>(x - 1) / (divisions_x - 1), static_cast<float>(y - 1) / (divisions_y - 1) };
				}
			}

			for (int y = 0; y < divisions_y + 1; ++y)
			{
				for (int x = 0; x < divisions_x + 1; ++x)
				{
					indices.push_back(x + y * (divisions_x + 2));
					indices.push_back(x + 1 + y * (divisions_x + 2));
					indices.push_back(x + (y + 1) * (divisions_x + 2));
					indices.push_back(x + (y + 1) * (divisions_x + 2));
					indices.push_back(x + 1 + y * (divisions_x + 2));
					indices.push_back(x + 1 + (y + 1) * (divisions_x + 2));
				}
			}

			// set data
			vao.unbind(); // TODO pomyœleæ nad dopilnowaniem, ¿eby VAO by³ zbindowany tylko na renderowanie i tworzenie obiektu, bo mog¹ byæ z tym problemy
			vbo.bind();
			vbo.set_dynamic_data(reinterpret_cast<const float*>(uvs.data()), uvs.size() * sizeof(Vector2));
			ebo.bind();
			ebo.set_dynamic_data(indices.data(), indices.size() * sizeof(unsigned int));

			indices_count = indices.size();
		}
	}

	void WorkpieceRenderable::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_workpiece_renderable(*this, color, width, height, thickness);
		if (line.visible)
			line.render(renderer, width, height); // TODO model matrix line'a mno¿yæ przez model workpiece!
		if (cutter_mesh.visible)
			cutter_mesh.render(renderer, width, height);
	}
}
