#pragma once

#include "algebra.h"
#include <vector>
#include <tuple>

namespace ManualCAD
{
	class HeightMap {
		std::vector<float> pixels;
	public:
		int width = 0, height = 0;

		Vector3 size;

		HeightMap() {}
		HeightMap(int size_x, int size_y, const Vector3& size) : width(size_x), height(size_y), size(size), pixels(size_x * size_y, 1.0f) {}

		void fill(const Vector3& size) { 
			this->size = size; 
			pixels.assign(pixels.size(), 1.0f);
		}
		void resize(int size_x, int size_y, const Vector3& size) { 
			this->width = size_x; 
			this->height = size_y; 
			pixels.resize(size_x * size_y);
			fill(size);
		}

		inline const float* data() const { return pixels.data(); }
		inline float* data() { return pixels.data(); }

		inline bool are_coords_valid(int x, int y) { return x >= 0 && x < width && y >= 0 && y < height; }

		inline float get_pixel(int x, int y) const { 
			if (x < 0 || x >= width || y < 0 || y >= height)
				return 0.0f;
			return pixels[x + y * width] * size.y; 
		}

		inline void set_pixel(int x, int y, float value) {
			if (x < 0 || x >= width || y < 0 || y >= height)
				return;

			float value_to_map = value / size.y;
			if (value_to_map < pixels[x + y * width])
				pixels[x + y * width] = value_to_map;
		}

		inline Vector2 position_to_pixel(Vector2 pos) const {
			return {
				(pos.x / size.x + 0.5f) * width,
				(pos.y / size.z + 0.5f) * height // this is correct: Vector2 doesn't have height component (thus pos.y is depth) and in CAD height is Y (thus size.z is depth)
			};
		}

		inline Vector2 position_to_pixel(Vector3 pos) const {
			return {
				(pos.x / size.x + 0.5f) * width,
				(pos.z / size.z + 0.5f) * height
			};
		}

		inline Vector2 pixel_to_position(Vector2 pix) const {
			return {
				(pix.x / width - 0.5f) * size.x,
				(pix.y / height - 0.5f) * size.z // this is correct: Vector2 doesn't have height component (thus pix.y is depth), in CAD height is Y (thus size.z is depth)
			};
		}

		inline float length_to_pixels_x(float l) const {
			return l / size.x * width;
		}

		inline float length_to_pixels_y(float l) const {
			return l / size.z * height;
		}

		inline float pixels_to_length_x(float pix) const {
			return pix / width * size.x;
		}

		inline float pixels_to_length_y(float pix) const {
			return pix / height * size.z;
		}
	};
}