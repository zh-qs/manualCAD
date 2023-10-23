#pragma once

#include "algebra.h"
#include <vector>
#include <tuple>

namespace ManualCAD
{
	class HeightMap {
		std::vector<float> pixels;
	public:
		int size_x = 0, size_y = 0;
		
		Vector3 size;

		HeightMap() {}
		HeightMap(int size_x, int size_y, const Vector3& size) : size_x(size_x), size_y(size_y), size(size), pixels(size_x * size_y, 1.0f) {}

		void fill(const Vector3& size) { 
			this->size = size; 
			pixels.assign(pixels.size(), 1.0f);
		}
		void resize(int size_x, int size_y, const Vector3& size) { 
			this->size_x = size_x; 
			this->size_y = size_y; 
			pixels.resize(size_x * size_y);
			fill(size);
		}

		inline const float* data() const { return pixels.data(); }

		inline bool are_coords_valid(int x, int y) { return x >= 0 && x < size_x && y >= 0 && y < size_y; }

		inline float get_pixel(int x, int y) const { 
			if (x < 0 || x >= size_x || y < 0 || y >= size_y)
				return 0.0f;
			return pixels[x + y * size_x] * size.y; 
		}

		inline void set_pixel(int x, int y, float value) {
			if (x < 0 || x >= size_x || y < 0 || y >= size_y)
				return;

			float value_to_map = value / size.y;
			if (value_to_map < pixels[x + y * size_x])
				pixels[x + y * size_x] = value_to_map;
		}

		inline Vector2 position_to_pixel(Vector2 pos) {
			return {
				(pos.x / size.x + 0.5f) * size_x,
				(pos.y / size.z + 0.5f) * size_y // this is correct: in milling programs height is Z (thus pos.y is depth), in CAD height is Y (thus size.z is depth)
			};
		}

		inline Vector2 position_to_pixel(Vector3 pos) {
			return {
				(pos.x / size.x + 0.5f) * size_x,
				(pos.y / size.z + 0.5f) * size_y // this is correct: in milling programs height is Z (thus pos.y is depth), in CAD height is Y (thus size.z is depth)
			};
		}

		inline Vector2 pixel_to_position(Vector2 pix) {
			return {
				(pix.x / size_x - 0.5f) * size.x,
				(pix.y / size_y - 0.5f) * size.z // this is correct: in milling programs height is Z (thus pos.y is depth), in CAD height is Y (thus size.z is depth)
			};
		}

		inline float length_to_pixels_x(float l) {
			return l / size.x * size_x;
		}

		inline float length_to_pixels_y(float l) {
			return l / size.z * size_y;
		}

		inline float pixels_to_length_x(float pix) {
			return pix / size_x * size.x;
		}

		inline float pixels_to_length_y(float pix) {
			return pix / size_y * size.z;
		}

		void cut_pixel_line(const std::pair<int, int>& from, const std::pair<int, int>& to); // TODO additional parameters
	};
}