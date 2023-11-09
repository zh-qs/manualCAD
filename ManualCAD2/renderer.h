#pragma once
#include "vertex_array.h"
#include "buffer.h"
#include "shader.h"
#include "frame_buffer.h"
#include "texture.h"
#include "camera.h"
#include "window.h"
#include "wireframe_mesh.h"
#include "object_controller.h"
#include "point_set.h"
#include "curve_with_polyline.h"
#include "surface_with_contour.h"
#include "rational_20_param_surface.h"
#include "axes_cursor.h"
#include "render_step.h"
#include "simple_rect.h"
#include "workpiece_renderable.h"
#include "triangle_mesh.h"

#include <list>
#include <memory>
#include "line.h"
#include "textured_wireframe_mesh.h"

namespace ManualCAD
{
	class Renderer {
		static const int STEREOSCOPY_POVS = 2;
		FrameBuffer fbo[STEREOSCOPY_POVS];
		RenderTexture texture[STEREOSCOPY_POVS];
		VertexArray quad_vao;
		VertexBuffer quad_screen_vbo;
		VertexBuffer quad_uvs_vbo;
		Shader quad_shader;
		GLint uq_tex_location[STEREOSCOPY_POVS];
		GLint uq_lcolor_location, uq_saturation_location;

		Texture white_texture;

		Camera camera;

		std::list<std::unique_ptr<RenderStep>> render_steps;
	public:
		struct StereoscopySettings {
			bool enabled = false;
			Vector4 left_eye_color = { 1.0f,0.0f,0.0f,1.0f };
			Vector4 right_eye_color = { 0.0f,1.0f,1.0f,1.0f };
			float interocular_distance = 0.1f;
			float focus_plane = 10.0f;
			float view_saturation = 1.0f;
		} stereoscopy_settings;

		Renderer();

		void init();

		//void clear_render_steps() { render_steps.clear(); }
		void add(const Renderable& renderable, float thickness = 1.0f);
		void render_all(int width, int height);

		void render_wireframe(const WireframeMesh& mesh, const Vector4& color, int width, int height, float thickness = 1.0f);
		void render_textured_wireframe(const TexturedWireframeMesh& mesh, const Vector4& color, int width, int height, float thickness = 1.0f);
		void render_points(const PointSet& mesh, const Vector4& color, int width, int height, float thickness = 1.0f);
		void render_curve_and_polyline(const CurveWithPolyline& curve, const Vector4& color, int width, int height, float thickness = 1.0f);
		void render_axes_cursor(const AxesCursor& ac, int width, int height, float thickness = 1.0f);
		void render_surface_and_bezier_contour(const SurfaceWithBezierContour& surf, const Vector4& color, int width, int height, float thickness = 1.0f);
		void render_surface_and_de_boor_contour(const SurfaceWithDeBoorContour& surf, const Vector4& color, int width, int height, float thickness = 1.0f);
		void render_rational_20_param_surface(const Rational20ParamSurface& surf, const Vector4& color, int width, int height, float thickness = 1.0f);
		void render_simple_rect(const SimpleRect& rect, int width, int height, float thickness = 1.0f);
		void render_line(const Line& line, const Vector4& color, int width, int height, float thickness = 1.0f);
		void render_line_2d(const Line2D& line, const Vector4& color, int width, int height, float thickness = 1.0f);
		void render_workpiece_renderable(const WorkpieceRenderable& workpiece_renderable, const Vector4& color, int width, int height, float thickness = 1.0f);
		void render_triangle_mesh(const TriangleMesh& mesh, const Vector4& color, int width, int height, float thickness = 1.0f);
		void enable_depth_testing();
		void disable_depth_testing();
		void enable_depth_buffer_write();
		void disable_depth_buffer_write();
		Camera& get_camera() { return camera; }
		void dispose();
	};
}