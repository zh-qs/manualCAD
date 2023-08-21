#pragma once

#include <vector>
#include "algebra.h"
#include "renderable.h"
#include "texture.h"

namespace ManualCAD
{
	class SurfaceWithBezierContour : public Renderable {
		//std::vector<Vector3> points;
		size_t patch_indices_count = 0;
		size_t contour_indices_count = 0;
		int patches_x = 0, patches_y = 0;
		ElementBuffer ebo, contour_ebo;
		bool ebos_set = false; // ebos will have constant value in the lifetime of a patch (except preview -> fn reset_ebos()), this flag is for setting them only once
	
		const Texture* texture;
	public:
		Vector4 contour_color = { 1.0f,1.0f,1.0f,1.0f };
		bool draw_patch = true;
		bool draw_contour = false;
		unsigned int divisions_x = 4;
		unsigned int divisions_y = 4;

		SurfaceWithBezierContour(const Matrix4x4& model, const Texture* texture) : Renderable(model), ebo(), contour_ebo(), texture(texture) { ebo.init(); contour_ebo.init(); contour_ebo.bind(); }
		SurfaceWithBezierContour(const Texture* texture) : Renderable(), ebo(), contour_ebo(), texture(texture) { ebo.init(); contour_ebo.init(); contour_ebo.bind(); }

		inline size_t get_patch_indices_count() const { return patch_indices_count; }
		inline size_t get_contour_indices_count() const { return contour_indices_count; }
		inline int get_patches_x_count() const { return patches_x; }
		inline int get_patches_y_count() const { return patches_y; }
		inline void bind_patch_to_render() const { bind_to_render(); ebo.bind(); }
		inline void bind_contour_to_render() const { bind_to_render(); contour_ebo.bind(); }

		inline bool has_texture() const { return texture != nullptr; }
		inline const Texture& get_texture() const { return *texture; }

		// Points should be aligned in a grid of points_x columns and points_y rows
		inline void reset_ebos() { ebos_set = false; }
		void set_data(const std::vector<Vector3>& points, int patches_x, int patches_y);
		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;
		void dispose() override { Renderable::dispose(); ebo.dispose(); contour_ebo.dispose(); }
	};

	class SurfaceWithDeBoorContour : public Renderable {
		size_t patch_indices_count = 0;
		size_t contour_indices_count = 0;
		int patches_x = 0, patches_y = 0;
		ElementBuffer ebo, contour_ebo;
		bool ebos_set = false; // ebos will have constant value in the lifetime of a patch (except preview -> fn reset_ebos()), this flag is for setting them only once
	
		const Texture* texture;
	public:
		Vector4 contour_color = { 1.0f,1.0f,1.0f,1.0f };
		bool draw_patch = true;
		bool draw_contour = false;
		unsigned int divisions_x = 4;
		unsigned int divisions_y = 4;

		SurfaceWithDeBoorContour(const Matrix4x4& model, const Texture* texture) : Renderable(model), ebo(), contour_ebo(), texture(texture) { ebo.init(); contour_ebo.init(); contour_ebo.bind(); }
		SurfaceWithDeBoorContour(const Texture* texture) : Renderable(), ebo(), contour_ebo(), texture(texture) { ebo.init(); contour_ebo.init(); contour_ebo.bind(); }

		inline size_t get_patch_indices_count() const { return patch_indices_count; }
		inline size_t get_contour_indices_count() const { return contour_indices_count; }
		inline int get_patches_x_count() const { return patches_x; }
		inline int get_patches_y_count() const { return patches_y; }
		inline void bind_patch_to_render() const { bind_to_render(); ebo.bind(); }
		inline void bind_contour_to_render() const { bind_to_render(); contour_ebo.bind(); }

		inline bool has_texture() const { return texture != nullptr; }
		inline const Texture& get_texture() const { return *texture; }

		// Points should be aligned in a grid of points_x columns and points_y rows
		inline void reset_ebos() { ebos_set = false; }
		void set_data(const std::vector<Vector3>& points, int patches_x, int patches_y);
		void render(Renderer& renderer, int width, int height, float thickness = 1.0f) const override;
		void dispose() override { Renderable::dispose(); ebo.dispose(); contour_ebo.dispose(); }
	};
}