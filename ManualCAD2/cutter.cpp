#include "cutter.h"
#include "logger.h"

namespace ManualCAD
{
	void Cutter::cut_pixel(HeightMap& height_map, int instruction_number, int x, int y, float height, float max_depth) const
	{
		float pixels_x = height_map.length_to_pixels_x(radius),
			pixels_y = height_map.length_to_pixels_y(radius);

		int lbound_x = lroundf(x - pixels_x), rbound_x = lroundf(x + pixels_x),
			lbound_y = lroundf(y - pixels_y), rbound_y = lroundf(y + pixels_y);

		for (int i = lbound_x; i <= rbound_x; ++i)
			for (int j = lbound_y; j <= rbound_y; ++j)
			{
				float lx = height_map.pixels_to_length_x(x - i),
					ly = height_map.pixels_to_length_y(y - j);
				float d = sqrtf(lx * lx + ly * ly);
				if (d <= radius)
				{
					check_cutter(height_map, i, j, instruction_number, height, max_depth);
					height_map.set_pixel(i, j, height + get_height_offset(d));
				}
			}
	}

	void BallCutter::cut_pixel_virtual(HeightMap& height_map, int instruction_number, int x, int y, float height, float max_depth) const
	{
		float pixels_x = height_map.length_to_pixels_x(radius),
			pixels_y = height_map.length_to_pixels_y(radius);

		int lbound_x = lroundf(x - pixels_x), rbound_x = lroundf(x + pixels_x),
			lbound_y = lroundf(y - pixels_y), rbound_y = lroundf(y + pixels_y);

		for (int i = lbound_x; i <= rbound_x; ++i)
			for (int j = lbound_y; j <= rbound_y; ++j)
			{
				float lx = height_map.pixels_to_length_x(x - i),
					ly = height_map.pixels_to_length_y(y - j);
				float d = sqrtf(lx * lx + ly * ly);
				if (d <= radius)
				{
					check_cutter(height_map, i, j, instruction_number, height, max_depth);
					height_map.set_pixel(i, j, height + radius - sqrtf(radius * radius - d * d));
				}
			}
	}

	float BallCutter::get_height_offset(const float& distance) const
	{
		return radius - sqrtf(radius * radius - distance * distance);
	}

	void BallCutter::generate_cutter_mesh(TriangleMesh& mesh) const
	{
		mesh.generate_bottom_capsule(radius, 10.0f, 10); // TODO cutter height!!! -> pobawiæ siê z vertex shaderem
	}

	void FlatCutter::cut_pixel_virtual(HeightMap& height_map, int instruction_number, int x, int y, float height, float max_depth) const
	{
		float pixels_x = height_map.length_to_pixels_x(radius),
			pixels_y = height_map.length_to_pixels_y(radius);

		int lbound_x = lroundf(x - pixels_x), rbound_x = lroundf(x + pixels_x),
			lbound_y = lroundf(y - pixels_y), rbound_y = lroundf(y + pixels_y);

		for (int i = lbound_x; i < rbound_x; ++i)
			for (int j = lbound_y; j < rbound_y; ++j)
			{
				float lx = height_map.pixels_to_length_x(x - i),
					ly = height_map.pixels_to_length_y(y - j);
				float d = sqrtf(lx * lx + ly * ly);
				if (d <= radius)
				{
					check_cutter(height_map, i, j, instruction_number, height, max_depth);
					height_map.set_pixel(i, j, height);
				}
			}
	}

	float FlatCutter::get_height_offset(const float& distance) const
	{
		return 0.0f;
	}

	void FlatCutter::generate_cutter_mesh(TriangleMesh& mesh) const
	{
		mesh.generate_cylinder(radius, 10.0f, 10); // TODO cutter height!!! -> pobawiæ siê z vertex shaderem
	}
}