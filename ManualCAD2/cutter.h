#pragma once

#include "height_map.h"
#include "triangle_mesh.h"
#include "logger.h"

namespace ManualCAD
{
	class Cutter {
	protected:
		float radius;
	public:
		float cutting_part_height = 4.0f;

		Cutter(float diameter) : radius(0.5f * diameter) {}
		float get_diameter() const { return 2.0f * radius; }

		void cut_pixel(HeightMap& height_map, int instruction_number, int x, int y, float height, float max_depth) const;
		virtual void cut_pixel_virtual(HeightMap& height_map, int instruction_number, int x, int y, float height, float max_depth) const = 0;
		virtual float get_height_offset(const float& distance) const = 0;
		virtual void generate_cutter_mesh(TriangleMesh& mesh) const = 0;
		virtual const char* get_type() const = 0;

		inline void check_cutter(const HeightMap& height_map, int i, int j, const int& instruction_number, const float& height, const float& max_depth) const {
			if (height_map.size.y - height > max_depth)
				Logger::log_warning("[WARNING] N%d at (%d,%d): Cutter too deep\n", instruction_number, i, j);
			if (height_map.get_pixel(i, j) > height + cutting_part_height)
				Logger::log_warning("[WARNING] N%d at (%d,%d): Using non-cutting part\n", instruction_number, i, j);
		}
	};

	class BallCutter : public Cutter {
	public:
		BallCutter(float diameter) : Cutter(diameter) {}

		void cut_pixel_virtual(HeightMap& height_map, int instruction_number, int x, int y, float height, float max_depth) const override;
		float get_height_offset(const float& distance) const override;
		void generate_cutter_mesh(TriangleMesh& mesh) const override;
		const char* get_type() const override { return "Ball"; }
	};

	class FlatCutter : public Cutter {
	public:
		FlatCutter(float diameter) : Cutter(diameter) {}

		void cut_pixel_virtual(HeightMap& height_map, int instruction_number, int x, int y, float height, float max_depth) const override;
		float get_height_offset(const float& distance) const override;
		void generate_cutter_mesh(TriangleMesh& mesh) const override;
		const char* get_type() const override { return "Flat"; }
	};
}