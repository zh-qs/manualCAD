#include "surface_with_contour.h"
#include "renderer.h"

namespace ManualCAD
{
	void SurfaceWithBezierContour::set_data(const std::vector<Vector3>& points, int patches_x, int patches_y)
	{
		this->patches_x = patches_x;
		this->patches_y = patches_y;

		vbo.bind();
		vbo.set_dynamic_data(reinterpret_cast<const float*>(points.data()), points.size() * sizeof(Vector3));

		if (ebos_set) return;

		std::vector<unsigned int> patch_indices;
		patch_indices.reserve(16 * patches_x * patches_y);

		const int points_y = 3 * patches_y + 1;
		const int points_x = 3 * patches_x + 1;

		for (int i = 0; i < patches_x; ++i)
		{
			for (int j = 0; j < patches_y; ++j)
			{
				for (int x = 0; x < 4; ++x)
					for (int y = 0; y < 4; ++y)
						patch_indices.push_back((3 * i + x) + (3 * j + y) * points_x);
			}
		}
		ebo.bind();
		ebo.set_dynamic_data(patch_indices.data(), patch_indices.size() * sizeof(unsigned int));

		patch_indices_count = patch_indices.size();

		patch_indices.clear();
		int edges = 2 * points_x * points_y - points_x - points_y;
		patch_indices.reserve(2 * edges);
		for (int i = 0; i < points_x; ++i)
		{
			for (int j = 0; j < points_y - 1; ++j)
			{
				patch_indices.push_back(i + j * points_x);
				patch_indices.push_back(i + (j + 1) * points_x);
			}
		}
		for (int i = 0; i < points_x - 1; ++i)
		{
			for (int j = 0; j < points_y; ++j)
			{
				patch_indices.push_back(i + j * points_x);
				patch_indices.push_back(i + 1 + j * points_x);
			}
		}
		contour_ebo.bind();
		contour_ebo.set_dynamic_data(patch_indices.data(), patch_indices.size() * sizeof(unsigned int));

		contour_indices_count = patch_indices.size();

		ebos_set = true;
	}

	void SurfaceWithBezierContour::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_surface_and_bezier_contour(*this, color, width, height, thickness);
	}

	void SurfaceWithDeBoorContour::set_data(const std::vector<Vector3>& points, int patches_x, int patches_y)
	{
		this->patches_x = patches_x;
		this->patches_y = patches_y;

		vbo.bind();
		vbo.set_dynamic_data(reinterpret_cast<const float*>(points.data()), points.size() * sizeof(Vector3));

		if (ebos_set) return;

		std::vector<unsigned int> patch_indices;
		patch_indices.reserve(16 * patches_x * patches_y);

		const int points_y = patches_y + 3;
		const int points_x = patches_x + 3;

		for (int i = 0; i < patches_x; ++i)
		{
			for (int j = 0; j < patches_y; ++j)
			{
				for (int x = 0; x < 4; ++x)
					for (int y = 0; y < 4; ++y)
						patch_indices.push_back((i + x) + (j + y) * points_x);
			}
		}
		ebo.bind();
		ebo.set_dynamic_data(patch_indices.data(), patch_indices.size() * sizeof(unsigned int));

		patch_indices_count = patch_indices.size();

		patch_indices.clear();
		int edges = 2 * points_x * points_y - points_x - points_y;
		patch_indices.reserve(2 * edges);
		for (int i = 0; i < points_x; ++i)
		{
			for (int j = 0; j < points_y - 1; ++j)
			{
				patch_indices.push_back(i + j * points_x);
				patch_indices.push_back(i + (j + 1) * points_x);
			}
		}
		for (int i = 0; i < points_x - 1; ++i)
		{
			for (int j = 0; j < points_y; ++j)
			{
				patch_indices.push_back(i + j * points_x);
				patch_indices.push_back(i + 1 + j * points_x);
			}
		}
		contour_ebo.bind();
		contour_ebo.set_dynamic_data(patch_indices.data(), patch_indices.size() * sizeof(unsigned int));

		contour_indices_count = patch_indices.size();

		ebos_set = true;
	}

	void SurfaceWithDeBoorContour::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_surface_and_de_boor_contour(*this, color, width, height, thickness);
	}

	void NURBSWithDeBoorContour::set_data(const std::vector<Vector3>& points, const std::vector<float>& weights, int patches_x, int patches_y)
	{
		this->patches_x = patches_x;
		this->patches_y = patches_y;

		vbo.bind();
		vbo.set_dynamic_data(reinterpret_cast<const float*>(points.data()), points.size() * sizeof(Vector3));
		weight_vbo.bind();
		weight_vbo.set_dynamic_data(weights.data(), weights.size() * sizeof(float));

		if (ebos_set) return;

		std::vector<unsigned int> patch_indices;
		patch_indices.reserve(16 * patches_x * patches_y);

		const int points_y = patches_y + 3;
		const int points_x = patches_x + 3;

		for (int i = 0; i < patches_x; ++i)
		{
			for (int j = 0; j < patches_y; ++j)
			{
				for (int x = 0; x < 4; ++x)
					for (int y = 0; y < 4; ++y)
						patch_indices.push_back((i + x) + (j + y) * points_x);
			}
		}
		ebo.bind();
		ebo.set_dynamic_data(patch_indices.data(), patch_indices.size() * sizeof(unsigned int));

		patch_indices_count = patch_indices.size();

		patch_indices.clear();
		int edges = 2 * points_x * points_y - points_x - points_y;
		patch_indices.reserve(2 * edges);
		for (int i = 0; i < points_x; ++i)
		{
			for (int j = 0; j < points_y - 1; ++j)
			{
				patch_indices.push_back(i + j * points_x);
				patch_indices.push_back(i + (j + 1) * points_x);
			}
		}
		for (int i = 0; i < points_x - 1; ++i)
		{
			for (int j = 0; j < points_y; ++j)
			{
				patch_indices.push_back(i + j * points_x);
				patch_indices.push_back(i + 1 + j * points_x);
			}
		}
		contour_ebo.bind();
		contour_ebo.set_dynamic_data(patch_indices.data(), patch_indices.size() * sizeof(unsigned int));

		contour_indices_count = patch_indices.size();

		ebos_set = true;
	}

	void NURBSWithDeBoorContour::render(Renderer& renderer, int width, int height, float thickness) const
	{
		renderer.render_nurbs_and_de_boor_contour(*this, color, width, height, thickness);
	}
}
