#pragma once
#include "wireframe_mesh.h"
#include "textured_wireframe_mesh.h"
#include "point_set.h"
#include "application_settings.h"
#include "cursor.h"
#include "line.h"
#include "parametric_surface.h"
#include <string>
#include <memory>
#include <list>
#include "transformation.h"
#include "curve_with_polyline.h"
#include "surface_with_contour.h"
#include "rational_20_param_surface.h"
#include "rectangle.h"
#include "graph.h"
#include "toggling_texture.h"
#include "box.h"
#include "constant_parameter.h"
#include "parametric_surface_intersection.h"
#include "ray.h"
#include "parametric_curve.h"

namespace ManualCAD
{
	class Callbackable {
	public:
		virtual void on_child_move(int idx, const Vector3& prev_position, const Vector3& new_position) = 0;
	};

	class ObjectSettingsWindow;
	class Serializer;

	class Object {
		friend class ObjectSettings;
		friend class JSONSerializer;

		bool valid = false;
	protected:
		Renderable& renderable;
		virtual void generate_renderable() = 0;
		virtual void build_specific_settings(ObjectSettingsWindow& parent) = 0;
		bool transformable = true;
		bool illusory = false; // illusory objects are not displayed nor can be deleted, they cannot make multiple selections
		int persistence = 0; // persistent objects (>0) cannot be deleted by user
		int children = 0; // dependent objects
		int real_children = 0; // children that are not illusory

		std::list<Object*> observers;

		void copy_basic_attributes_to(Object& obj) const {
			obj.transformable = transformable;
			obj.illusory = illusory;
			obj.children = children;
			obj.real_children = real_children;
			obj.color = color;
			obj.transformation = transformation;
		}
	public:
		Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Transformation transformation;

		std::string name;

		Object(Renderable& renderable) : renderable(renderable) {}

		virtual void bind_with(Object& object) = 0;
		virtual void remove_binding_with(Object& object) = 0;

		virtual float intersect_with_ray(const Ray& ray) = 0;
		virtual bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const = 0;

		virtual void replace_child_by(Object& child, Object& other) = 0;

		virtual void on_move(const Vector3& move) {}
		virtual void on_delete() {}
		virtual void on_camera_move() {}

		void add_observer(Object& object) { observers.push_back(&object); }
		void remove_observer(Object& object) { observers.remove(&object); }
		void inherit_observers_from(const Object& object) { for (auto* o : object.observers) add_observer(*o); }
		void replace_in_observers_by(Object& object) { for (auto* o : observers) o->replace_child_by(*this, object); }
		bool is_observed_by(const Object& object) const { return std::find(observers.begin(), observers.end(), &object) != observers.end(); }

		const Renderable& get_const_renderable() const { return renderable; } // only for retrieving model matrices for bindings; avoid it during rendering!
		const Renderable& get_renderable();
		const Renderable& get_renderable(const Transformation& combine_transformation, const Vector3& center);
		void update_renderable_matrix();
		void update_renderable_matrix(const Transformation& combine_transformation, const Vector3& center);
		bool invalidate_if(bool pred) { if (pred) invalidate(); return pred; }
		void invalidate() { valid = false; }
		void invalidate_observers() { for (auto* o : observers) o->invalidate(); }
		void build_settings(ObjectSettingsWindow& parent);
		inline bool is_transformable() const { return transformable; }
		inline bool is_illusory() const { return illusory; }
		inline bool is_persistent() const { return persistence > 0 || illusory; }
		inline int children_count() const { return children; }
		inline int real_children_count() const { return real_children; }
		virtual bool is_point() const { return false; }

		inline void make_removable() { persistence = 0; }
		inline void increment_persistence() { ++persistence; };
		inline void decrement_persistence() { --persistence; };
		inline void add_persistence(const Object& other) { persistence += other.persistence; }

		// Serialization
		virtual void add_to_serializer(Serializer& serializer, int idx) {}

		virtual void dispose() { renderable.dispose(); }

		class Deleter {
		public:
			void operator()(Object* obj) {
				obj->dispose();
				delete obj;
			}
		};

		template <class O>
		using Handle = std::unique_ptr<O, Object::Deleter>;

		template <class O, class... Args>
		static inline Handle<O> create(Args&&... args) {
			return Handle<O>(new O(std::forward<Args>(args)...));
		}

		template <class O, class... Args>
		static Handle<O> create_at_cursor(const Cursor& cursor, Args&&... args) {
			auto obj = create<O>(std::forward<Args>(args)...);
			if constexpr (ApplicationSettings::DEBUG)
				Renderable::assert_unbound();
			obj->transformation.position = cursor.get_world_position();
			obj->renderable.set_model_matrix(obj->transformation.get_matrix());
			return obj;
		}

		virtual std::vector<Handle<Object>> clone() const = 0;
	};

	using ObjectHandle = Object::Handle<Object>;

	class IntersectionCurve;

	class ParametricCurveObject : public Object, public ParametricCurve {
	public:
		ParametricCurveObject(Renderable& renderable) : Object(renderable) {}
	};

	class ParametricSurfaceObject : public Object, public ParametricSurfaceWithSecondDerivative {
	public:
		//std::list<IntersectionCurve*> intersection_curves;
		TogglingTexture trim_texture;

		ParametricSurfaceObject(Renderable& renderable) : Object(renderable), trim_texture(ApplicationSettings::TRIM_TEXTURE_SIZE_PIXELS, ApplicationSettings::TRIM_TEXTURE_SIZE_PIXELS, *this) {}

		//void add_intersection(IntersectionCurve& curve) { 
		//	//intersection_curves.push_back(&curve);
		//	trim_texture.add_line(curve.get_uvs_line_for(*this));
		//}
		//void remove_intersection(IntersectionCurve& curve) { 
		//	//intersection_curves.remove(&curve); 
		//	trim_texture.remove_line(curve.get_uvs_line_for(*this));
		//}
	};

	class ObjectCollection : public MouseTrackable {
		friend class ObjectSettings;

		PointSet set;

		static Transformation EMPTY;
		Transformation transformation;

		std::vector<Object*> objects;
	public:
		Vector4 color = { 0.0f,0.0f,0.0f,1.0f };

		explicit ObjectCollection(const std::vector<Object*>& objects) : MouseTrackable(), objects(objects) {
			Vector3 avg = { 0.0f,0.0f,0.0f };
			for (const auto* obj : objects) {
				avg += obj->transformation.position;
			}
			if (objects.size() > 0) avg /= objects.size();
			position = avg;
			set.generate_point();
			set.set_model_matrix(Matrix4x4::translation(position));
			set.color = { 0.0f,0.0f,0.0f,1.0f };
		}

		inline size_t count() const { return objects.size(); }
		inline bool empty() const { return objects.empty(); }
		inline const Transformation& get_transformation() const { return transformation; }


		bool is_transformable() const {
			bool res = true;
			for (const auto* obj : objects)
				res &= obj->is_transformable();
			return res;
		}

		void move(const Vector3& vec) override {
			transformation.position += vec;
			//position += vec;
			/*for (auto* obj : objects) {
				obj->selection_transformation.position += vec;
			}*/
			auto move = transformation.position + position;
			set.set_model_matrix(Matrix4x4::translation(move));
			update_renderables();
			invalidate_observers();
			on_move(move);
		}
		void set_world_position(const Vector3& vec) override {
			move(vec - position - get_transformation().position);
		}
		const Vector3 get_world_position() const override {
			return get_transformation().position + position;
		}

		void build_settings(ObjectSettingsWindow& parent);
		const PointSet& get_point_set() const { return set; }

		bool is_hovered(float mouse_x, float mouse_y, const Camera& camera, int width, int height) const {
			if (objects.empty()) return false;
			if (!is_transformable()) return false; // TODO : optimize with variable changed when collection is created
			auto pos = get_screen_position_pixels(camera, width, height);
			return abs(mouse_x - pos.x) + abs(mouse_y - pos.y) <= ApplicationSettings::RENDER_POINT_SIZE;
		}

		void scale(const Vector3& scale_f) {
			/*for (auto* obj : objects) {
				obj->selection_transformation.scale *= scale_f;
			}*/
			transformation.scale *= scale_f;
			update_renderables();
			invalidate_observers();
		}

		void set_scale(const Vector3& scale_f) {
			/*for (auto* obj : objects) {
				obj->selection_transformation.scale = scale_f;
			}*/
			transformation.scale = scale_f;
			update_renderables();
			invalidate_observers();
		}

		void set_rotation(const Vector3& axis, float angle_deg) {
			// https://math.stackexchange.com/questions/382760/composition-of-two-axis-angle-rotations
			/*for (auto* obj : objects) {
				obj->selection_transformation.rotation_vector = axis;
				obj->selection_transformation.rotation_angle_deg = angle_deg;
			}*/
			transformation.rotation_vector = axis;
			transformation.rotation_angle_deg = angle_deg;
			update_renderables();
			invalidate_observers();
		}

		void apply_transformation_to_objects() {
			for (auto* obj : objects) {
				//obj->transformation.combine_with(obj->selection_transformation, position);
				//obj->selection_transformation.reset();
				if (!obj->is_illusory()) obj->transformation.combine_with(transformation, position);
			}
			position += get_transformation().position;
			transformation.reset();
			invalidate_observers();
		}

		inline std::vector<Object*>::iterator begin() { return objects.begin(); }
		inline std::vector<Object*>::iterator end() { return objects.end(); }

		void bind_with(Object& object) {
			for (auto* obj : objects)
				obj->bind_with(object);
		}

		void invalidate_observers() {
			for (auto* obj : objects)
				obj->invalidate_observers();
		}

		void update_renderables() {
			for (auto* obj : objects)
				obj->update_renderable_matrix(transformation, position);
		}

		void on_move(const Vector3& move) {
			for (auto* obj : objects)
				obj->on_move(move);
		}
	};

	class Torus : public ParametricSurfaceObject {
		friend class ObjectSettings;
		friend class JSONSerializer;

		static int counter;

		TexturedWireframeMesh mesh;

		float large_radius = 1.0f;
		float small_radius = 0.5f;
		unsigned int divisions_x = 20;
		unsigned int divisions_y = 20;
		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;
	public:
		Torus() : mesh(trim_texture.get_texture()), ParametricSurfaceObject(mesh) {
			name = "Torus " + std::to_string(counter++);
		}
		void bind_with(Object& object) override {}
		void remove_binding_with(Object& object) override {}
		float intersect_with_ray(const Ray& ray) override;
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }

		void add_to_serializer(Serializer& serializer, int idx) override;
		void replace_child_by(Object& child, Object& other) override {}

		Range<float> get_u_range() const override { return { 0.0f, TWO_PI }; }
		Range<float> get_v_range() const override { return { 0.0f, TWO_PI }; }

		Vector3 evaluate(float u, float v) const override {
			const float Rrcos = large_radius + small_radius * cosf(u);
			const Vector4 local = { Rrcos * cosf(v), small_radius * sinf(u), Rrcos * sinf(v), 1.0f };
			return (transformation.get_matrix() * local).xyz();
		}

		Vector3 normal(float u, float v) const override {
			const float cosu = cosf(u);
			const Vector4 local = { cosu * cosf(v), sinf(u), cosu * sinf(v), 0.0f };
			return normalize((transformation.get_inversed_transposed_matrix() * local).xyz());
		}

		Vector3 du(float u, float v) const override {
			const float sinu = sinf(u);
			const Vector4 local = { -small_radius * cosf(v) * sinu, small_radius * cosf(u), -small_radius * sinf(v) * sinu, 0.0f };
			return (transformation.get_matrix() * local).xyz();
		}

		Vector3 dv(float u, float v) const override {
			const float Rrcos = large_radius + small_radius * cosf(u);
			const Vector4 local = { -Rrcos * sinf(v), 0.0f, Rrcos * cosf(v), 0.0f };
			return (transformation.get_matrix() * local).xyz();
		}

		Vector3 duu(float u, float v) const override {
			const float cosu = cosf(u);
			const Vector4 local = { -small_radius * cosf(v) * cosu, -small_radius * sinf(u), -small_radius * sinf(v) * cosu, 0.0f };
			return (transformation.get_matrix() * local).xyz();
		}

		Vector3 duv(float u, float v) const override {
			const float sinu = sinf(u);
			const Vector4 local = { small_radius * sinf(v) * sinu, 0.0f, -small_radius * cosf(v) * sinu, 0.0f };
			return (transformation.get_matrix() * local).xyz();
		}

		Vector3 dvv(float u, float v) const override {
			const float Rrcos = large_radius + small_radius * cosf(u);
			const Vector4 local = { -Rrcos * cosf(v), 0.0f, -Rrcos * sinf(v), 0.0f };
			return (transformation.get_matrix() * local).xyz();
		}

		Box get_bounding_box() const override {
			return Box::degenerate(); // TODO
		}

		std::vector<RangedBox<float>> get_patch_bounds() const override {
			return { {get_bounding_box(), get_u_range(), get_v_range()} };
		}

		std::vector<ObjectHandle> clone() const override;
	};

	class Point : public Object {
		friend class ObjectSettings;
		friend class JSONSerializer;

		static int counter;

		PointSet set;

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;
	public:
		Point(bool persistent = false) : Object(set) {
			this->persistence = persistent ? 1 : 0;
			name = "Point " + std::to_string(counter++);
		}
		void bind_with(Object& object) override {}
		void remove_binding_with(Object& object) override {}
		float intersect_with_ray(const Ray& ray) override;
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override;

		inline Vector3 get_const_position() const {
			const auto& m = set.get_model_matrix();
			return { m.elem[0][3], m.elem[1][3], m.elem[2][3] };
		}

		void add_to_serializer(Serializer& serializer, int idx) override;
		bool is_point() const override { return true; }
		void replace_child_by(Object& child, Object& other) override {}

		std::vector<ObjectHandle> clone() const override;
	};

	class BezierC0Curve : public ParametricCurveObject {
		friend class ObjectSettings;
		friend class JSONSerializer;

		static int counter;

		CurveWithPolyline curve;

		std::list<Point*> points;
		bool polyline_visible = false;
		bool curve_visible = true;

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;
	public:
		Vector4 polyline_color = { 1.0f,1.0f,1.0f,1.0f };

		BezierC0Curve(const std::list<Point*>& points) : ParametricCurveObject(curve), points(points) {
			name = "Bezier C0 Curve " + std::to_string(counter++);
			transformable = false;
			for (auto* p : points)
				p->add_observer(*this);
		}
		void bind_with(Object& object) override;
		void remove_binding_with(Object& object) override;
		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }

		void add_to_serializer(Serializer& serializer, int idx) override;
		void replace_child_by(Object& child, Object& other) override {
			std::transform(points.begin(), points.end(), points.begin(), [&child, &other](Point* o) { return o == &child ? dynamic_cast<Point*>(&other) : o; });
		}

		std::vector<ObjectHandle> clone() const override;

		std::vector<Vector3> get_bezier_points() const override;
	};

	class PointCollection : public Object {
		friend class ObjectSettings;

		PointSet set;

		std::vector<Vector3> points;
		Callbackable& owner;
		int current_idx = -1;

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override {}
	public:
		PointCollection(Callbackable& owner) : Object(set), owner(owner) {
			illusory = true;
			set.visible = false;
			color = { 1.0f,1.0f,0.0f,1.0f };
		}

		void set_points(std::vector<Vector3>&& points_ref) {
			points = std::move(points_ref);
			invalidate();
		}
		void set_points(const std::vector<Vector3>& points_ref) {
			points = points_ref;
			invalidate();
		}

		std::vector<Vector3>& get_points() { return points; }

		void bind_with(Object& object) override {}
		void remove_binding_with(Object& object) override {}
		float intersect_with_ray(const Ray& ray) override;
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }

		void on_move(const Vector3& move) override {
			const Vector3& ppos = points[current_idx];
			owner.on_child_move(current_idx, ppos, move);
			//points[current_idx] = move;
			invalidate();
		}

		void replace_child_by(Object& child, Object& other) override {}

		std::vector<ObjectHandle> clone() const override;
	};

	enum class DeBoorPointsBehaviour {
		RotateAroundCenter = 0,
		MoveAdjacent = 1
	};

	class BezierC2Curve : public ParametricCurveObject, public Callbackable {
		friend class ObjectSettings;
		friend class JSONSerializer;

		static int counter;

		CurveWithPolyline curve;
		PointCollection* bernstein_points;

		std::vector<Point*> points;
		bool polyline_visible = false;
		bool curve_visible = true;
		//bool boor_polyline_visible = false;
		DeBoorPointsBehaviour behaviour = DeBoorPointsBehaviour::RotateAroundCenter;

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;
	public:
		Vector4 polyline_color = { 1.0f,1.0f,1.0f,1.0f };

		BezierC2Curve(const std::vector<Point*>& points) : ParametricCurveObject(curve), points(points) {
			name = "Bezier C2 Curve " + std::to_string(counter++);
			transformable = false;
			for (auto* p : points)
				p->add_observer(*this);
			children = 1;
		}
		void bind_with(Object& object) override;
		void remove_binding_with(Object& object) override;
		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }
		ObjectHandle make_linked_bernstein_points() {
			auto handle = Object::create<PointCollection>(*this);
			bernstein_points = handle.get();
			return handle;
		}

		void on_child_move(int idx, const Vector3& prev_position, const Vector3& new_position) override;

		void add_to_serializer(Serializer& serializer, int idx) override;

		void replace_child_by(Object& child, Object& other) override {
			std::transform(points.begin(), points.end(), points.begin(), [&child, &other](Point* o) { return o == &child ? dynamic_cast<Point*>(&other) : o; });
		}

		std::vector<ObjectHandle> clone() const override;

		std::vector<Vector3> get_bezier_points() const override;
	};

	class InterpolationSpline : public ParametricCurveObject {
		friend class ObjectSettings;
		friend class JSONSerializer;

		static int counter;

		CurveWithPolyline curve;

		std::vector<Point*> points;
		bool polyline_visible = false;
		bool curve_visible = true;

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;
	public:
		Vector4 polyline_color = { 1.0f,1.0f,1.0f,1.0f };

		InterpolationSpline(const std::vector<Point*>& points) : ParametricCurveObject(curve), points(points) {
			name = "Spline " + std::to_string(counter++);
			transformable = false;
			for (auto* p : points)
				p->add_observer(*this);
		}
		void bind_with(Object& object) override;
		void remove_binding_with(Object& object) override;
		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }

		void add_to_serializer(Serializer& serializer, int idx) override;

		void replace_child_by(Object& child, Object& other) override {
			std::transform(points.begin(), points.end(), points.begin(), [&child, &other](Point* o) { return o == &child ? dynamic_cast<Point*>(&other) : o; });
		}

		std::vector<ObjectHandle> clone() const override;

		std::vector<Vector3> get_bezier_points() const override;
	};

	class ObjectController;

	struct PatchEdgePosition {
		Vector3 edge[4];
		Vector3 second[4];

		PatchEdgePosition bisect_get_first() const;
		PatchEdgePosition bisect_get_second() const;
	};

	struct PatchEdge {
		Point* edge[4];
		Point* second[4];
		Object* patch_ref;

		PatchEdgePosition to_position() const;
		static PatchEdge from_surface_array(const std::vector<Point*>& points, Object& patch, int idx_from, int step_to, int step_second);
	};

	class BicubicC0BezierSurface : public ParametricSurfaceObject {
		friend class ObjectSettings;
		friend class JSONSerializer;

		static int counter;

		SurfaceWithBezierContour surf;

		std::vector<Point*> points;
		bool contour_visible = false;
		bool patch_visible = true;
		int patches_x, patches_y;
		bool cylinder; // uWrapped

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;

		void decompose_uv(float u, float v, float& uu, float& vv, int& ui, int& vi) const;
	public:
		Vector4 contour_color = { 1.0f,1.0f,1.0f,1.0f };

		BicubicC0BezierSurface(const std::vector<Point*>& points, int patches_x, int patches_y, bool cylinder) : surf(&trim_texture.get_texture()), ParametricSurfaceObject(surf), points(points), patches_x(patches_x), patches_y(patches_y), cylinder(cylinder) {
			name = "Bezier C0 surface " + std::to_string(counter++);
			transformable = false;
			for (auto* p : points)
			{
				p->increment_persistence();
				p->add_observer(*this);
			}
		}

		void bind_with(Object& object) override {}
		void remove_binding_with(Object& object) override {}
		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }
		void on_delete() override;

		static BicubicC0BezierSurface& create_and_add(ObjectController& controller, const Vector3& cursor_pos, int patches_x, int patches_y, float dist_between_points_x, float dist_between_points_y);
		static BicubicC0BezierSurface& create_cylinder_and_add(ObjectController& controller, const Vector3& cursor_pos, int patches_x, int patches_y, float radius, float dist_between_points_y);

		void add_to_serializer(Serializer& serializer, int idx) override;
		void add_boundary_points(Graph<Point*, PatchEdge>& graph);

		void replace_child_by(Object& child, Object& other) override {
			std::transform(points.begin(), points.end(), points.begin(), [&child, &other](Point* o) { return o == &child ? dynamic_cast<Point*>(&other) : o; });
		}

		Range<float> get_u_range() const override { return { 0, static_cast<float>(patches_x) }; }
		Range<float> get_v_range() const override { return { 0, static_cast<float>(patches_y) }; }

		Vector3 evaluate(float u, float v) const override;
		Vector3 normal(float u, float v) const override;
		Vector3 du(float u, float v) const override;
		Vector3 dv(float u, float v) const override;
		Vector3 duu(float u, float v) const override;
		Vector3 duv(float u, float v) const override;
		Vector3 dvv(float u, float v) const override;

		Box get_bounding_box() const override {
			Box box = Box::degenerate();
			for (const auto* p : points)
				box.add(p->transformation.position);
			return box;
		}

		std::vector<RangedBox<float>> get_patch_bounds() const override;

		std::vector<ObjectHandle> clone() const override;

		//std::vector<ObjectHandle> clone_subdivide_patches(int subdivisions_x, int subdivisions_y) const;
	};

	class BicubicC0BezierSurfacePreview : public Object {
		friend class ObjectSettings;

		SurfaceWithBezierContour surf;

		Vector3 position;
		int patches_x = 1;
		int patches_y = 1;
		/*float dist_between_points_x = 1.0f;
		float dist_between_points_y = 1.0f;*/
		float width_x = 3.0f;
		float width_y = 3.0f;

		bool cylinder = false;
		float radius = 2.0f;

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;
	public:
		BicubicC0BezierSurfacePreview(const Vector3& cursor_pos) : surf(nullptr), Object(surf), position(cursor_pos) {
			name = "This should not be displayed";
			transformable = false;
			illusory = true;
			surf.color = { 1.0f,1.0f,1.0f,1.0f };
			surf.draw_contour = false;
			surf.draw_patch = true;
		}

		void bind_with(Object& object) override {}
		void remove_binding_with(Object& object) override {}
		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }
		void replace_child_by(Object& child, Object& other) override {}

		std::vector<ObjectHandle> clone() const override;
	};

	class BicubicC2BezierSurface : public ParametricSurfaceObject {
		friend class ObjectSettings;
		friend class JSONSerializer;

		static int counter;

		SurfaceWithDeBoorContour surf;

		std::vector<Point*> points;
		bool contour_visible = false;
		bool patch_visible = true;
		int patches_x, patches_y;
		bool cylinder; // uWrapped

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;

		void decompose_uv(float u, float v, float& uu, float& vv, int& ui, int& vi) const;
	public:
		Vector4 contour_color = { 1.0f,1.0f,1.0f,1.0f };

		BicubicC2BezierSurface(const std::vector<Point*>& points, int patches_x, int patches_y, bool cylinder) : surf(&trim_texture.get_texture()), ParametricSurfaceObject(surf), points(points), patches_x(patches_x), patches_y(patches_y), cylinder(cylinder) {
			name = "Bezier C2 surface " + std::to_string(counter++);
			transformable = false;
			for (auto& p : points)
			{
				p->increment_persistence();
				p->add_observer(*this);
			}
		}

		void bind_with(Object& object) override {}
		void remove_binding_with(Object& object) override {}
		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }
		void on_delete() override;

		static BicubicC2BezierSurface& create_and_add(ObjectController& controller, const Vector3& cursor_pos, int patches_x, int patches_y, float dist_between_points_x, float dist_between_points_y);
		static BicubicC2BezierSurface& create_cylinder_and_add(ObjectController& controller, const Vector3& cursor_pos, int patches_x, int patches_y, float radius, float dist_between_points_y);

		void add_to_serializer(Serializer& serializer, int idx) override;

		void replace_child_by(Object& child, Object& other) override {
			std::transform(points.begin(), points.end(), points.begin(), [&child, &other](Point* o) { return o == &child ? dynamic_cast<Point*>(&other) : o; });
		}

		Range<float> get_u_range() const override { return { 0, static_cast<float>(patches_x) }; }
		Range<float> get_v_range() const override { return { 0, static_cast<float>(patches_y) }; }

		Vector3 evaluate(float u, float v) const override;
		Vector3 normal(float u, float v) const override;
		Vector3 du(float u, float v) const override;
		Vector3 dv(float u, float v) const override;
		Vector3 duu(float u, float v) const override;
		Vector3 duv(float u, float v) const override;
		Vector3 dvv(float u, float v) const override;

		Box get_bounding_box() const override {
			Box box = Box::degenerate();
			for (const auto* p : points)
				box.add(p->transformation.position);
			return box; // TODO maybe we can make smaller box?
		}

		std::vector<RangedBox<float>> get_patch_bounds() const override;

		std::vector<ObjectHandle> clone() const override;
	};

	class BicubicC2BezierSurfacePreview : public Object {
		friend class ObjectSettings;

		SurfaceWithDeBoorContour surf;

		Vector3 position;
		int patches_x = 1;
		int patches_y = 1;
		/*float dist_between_points_x = 1.0f;
		float dist_between_points_y = 1.0f;*/
		float width_x = 1.0f;
		float width_y = 1.0f;

		bool cylinder = false;
		float radius = 2.0f;

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;
	public:
		BicubicC2BezierSurfacePreview(const Vector3& cursor_pos) : surf(nullptr), Object(surf), position(cursor_pos) {
			name = "This should not be displayed";
			transformable = false;
			illusory = true;
			surf.color = { 1.0f,1.0f,1.0f,1.0f };
			surf.draw_contour = false;
			surf.draw_patch = true;
		}

		void bind_with(Object& object) override {}
		void remove_binding_with(Object& object) override {}
		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }
		void replace_child_by(Object& child, Object& other) override {}

		std::vector<ObjectHandle> clone() const override;
	};

	class BicubicC2NURBSSurface : public ParametricSurfaceObject {
		friend class ObjectSettings;
		friend class JSONSerializer;

		static int counter;

		NURBSWithDeBoorContour surf;

		std::vector<Point*> points;
		std::vector<float> weights;
		bool contour_visible = false;
		bool patch_visible = true;
		int patches_x, patches_y;
		bool cylinder; // uWrapped

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;

		void decompose_uv(float u, float v, float& uu, float& vv, int& ui, int& vi) const;
	public:
		Vector4 contour_color = { 1.0f,1.0f,1.0f,1.0f };

		BicubicC2NURBSSurface(const std::vector<Point*>& points, const std::vector<float> weigths, int patches_x, int patches_y, bool cylinder) : surf(&trim_texture.get_texture()), ParametricSurfaceObject(surf), points(points), weights(weigths), patches_x(patches_x), patches_y(patches_y), cylinder(cylinder) {
			name = "NURBS C2 surface " + std::to_string(counter++);
			transformable = false;
			for (auto& p : points)
			{
				p->increment_persistence();
				p->add_observer(*this);
			}
		}

		BicubicC2NURBSSurface(const std::vector<Point*>& points, int patches_x, int patches_y, bool cylinder) : BicubicC2NURBSSurface(points, {}, patches_x, patches_y, cylinder) {
			weights.assign(points.size(), 1.0f);
		}

		void bind_with(Object& object) override {}
		void remove_binding_with(Object& object) override {}
		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }
		void on_delete() override;

		static BicubicC2NURBSSurface& create_and_add(ObjectController& controller, const Vector3& cursor_pos, int patches_x, int patches_y, float dist_between_points_x, float dist_between_points_y);
		static BicubicC2NURBSSurface& create_cylinder_and_add(ObjectController& controller, const Vector3& cursor_pos, int patches_x, int patches_y, float radius, float dist_between_points_y);

		void add_to_serializer(Serializer& serializer, int idx) override;

		void replace_child_by(Object& child, Object& other) override {
			std::transform(points.begin(), points.end(), points.begin(), [&child, &other](Point* o) { return o == &child ? dynamic_cast<Point*>(&other) : o; });
		}

		Range<float> get_u_range() const override { return { 0, static_cast<float>(patches_x) }; }
		Range<float> get_v_range() const override { return { 0, static_cast<float>(patches_y) }; }

		Vector3 evaluate(float u, float v) const override;
		Vector3 normal(float u, float v) const override;
		Vector3 du(float u, float v) const override;
		Vector3 dv(float u, float v) const override;
		Vector3 duu(float u, float v) const override;
		Vector3 duv(float u, float v) const override;
		Vector3 dvv(float u, float v) const override;

		Box get_bounding_box() const override {
			Box box = Box::degenerate();
			for (const auto* p : points)
				box.add(p->transformation.position);
			return box; // TODO maybe we can make smaller box?
		}

		std::vector<RangedBox<float>> get_patch_bounds() const override;

		std::vector<ObjectHandle> clone() const override;
	};

	class BicubicC2NURBSSurfacePreview : public Object {
		friend class ObjectSettings;

		NURBSWithDeBoorContour surf;

		Vector3 position;
		int patches_x = 1;
		int patches_y = 1;
		/*float dist_between_points_x = 1.0f;
		float dist_between_points_y = 1.0f;*/
		float width_x = 1.0f;
		float width_y = 1.0f;

		bool cylinder = false;
		float radius = 2.0f;

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;
	public:
		BicubicC2NURBSSurfacePreview(const Vector3& cursor_pos) : surf(nullptr), Object(surf), position(cursor_pos) {
			name = "This should not be displayed";
			transformable = false;
			illusory = true;
			surf.color = { 1.0f,1.0f,1.0f,1.0f };
			surf.draw_contour = false;
			surf.draw_patch = true;
		}

		void bind_with(Object& object) override {}
		void remove_binding_with(Object& object) override {}
		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }
		void replace_child_by(Object& child, Object& other) override {}

		std::vector<ObjectHandle> clone() const override;
	};

	class Gregory20ParamSurface : public Object {
		friend class ObjectSettings;

		static int counter;

		// winded in a cycle
		std::vector<PatchEdge> adjacent_patches;
		bool contour_visible = false;
		bool patch_visible = true;

		Rational20ParamSurface surf;

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;
	public:
		Vector4 contour_color = { 1.0f,1.0f,1.0f,1.0f };

		Gregory20ParamSurface(std::vector<PatchEdge>&& edges) : Object(surf), adjacent_patches(std::move(edges)) {
			name = "Gregory fill-in " + std::to_string(counter++);
			transformable = false;
			for (auto& e : adjacent_patches)
			{
				e.patch_ref->increment_persistence();
				for (auto* p : e.edge)
					p->add_observer(*this);
				for (auto* p : e.second)
					p->add_observer(*this);
			}
		}

		void bind_with(Object& object) override {}
		void remove_binding_with(Object& object) override {}
		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }

		void on_delete() override {
			for (auto& e : adjacent_patches)
				e.patch_ref->decrement_persistence();
		}

		template <template <class T> class Container>
		static std::list<ObjectHandle> fill_all_holes(const Container<BicubicC0BezierSurface*>& surfaces) {
			Graph<Point*, PatchEdge> graph;
			for (auto* surf : surfaces)
				surf->add_boundary_points(graph);
			auto triangles = graph.find_triangles();
			std::list<ObjectHandle> patches;
			for (const auto& t : triangles)
			{
				std::vector<PatchEdge> adjacent_patches;
				adjacent_patches.push_back(t.edges[0].weight);
				adjacent_patches.push_back(t.edges[1].weight);
				adjacent_patches.push_back(t.edges[2].weight);
				patches.push_back(Object::create<Gregory20ParamSurface>(std::move(adjacent_patches)));
			}
			return patches;
		}

		void replace_child_by(Object& child, Object& other) override {
			for (auto& p : adjacent_patches)
			{
				for (int i = 0; i < 4; ++i)
				{
					if (p.edge[i] == &child)
						p.edge[i] = dynamic_cast<Point*>(&other);
					if (p.second[i] == &child)
						p.second[i] = dynamic_cast<Point*>(&other);
				}
			}
		}

		std::vector<ObjectHandle> clone() const override;
	};

	class IntersectionCurve : public Object {
		friend class ObjectSettings;

		static int counter;

		ParametricSurfaceObject& surf1;
		ParametricSurfaceObject& surf2;
		std::vector<Vector2> uvs1;
		std::vector<Vector2> uvs2;

		Line line;
		Line2D uvs1_line, uvs2_line;

		bool singular_crossed;
		bool too_short;

		void generate_renderable() override;
		void build_specific_settings(ObjectSettingsWindow& parent) override;

	public:
		IntersectionCurve(ParametricSurfaceObject& surf1, ParametricSurfaceObject& surf2, const ParametricSurfaceIntersection& i)
			: Object(line), surf1(surf1), surf2(surf2), uvs1(i.get_uvs1()), uvs2(i.get_uvs2()), uvs1_line(surf1.get_u_range(), surf1.get_v_range()), uvs2_line(surf2.get_u_range(), surf2.get_v_range()), singular_crossed(i.is_singular_crossed()), too_short(i.is_too_short()) {
			name = "Intersection curve " + std::to_string(counter++);
			transformable = false;
			line.looped = i.is_looped();
			// intersection curve is generated only once (it is static), so we do not intend to add any observers or persistence bindings

			uvs1_line.set_data(this->uvs1);
			uvs2_line.set_data(this->uvs2);
			uvs1_line.color = uvs2_line.color = { 1.0f,1.0f,1.0f,1.0f };
			uvs1_line.looped = uvs2_line.looped = i.is_looped();
			surf1.trim_texture.add_line(uvs1_line);
			surf2.trim_texture.add_line(uvs2_line);
		}

		IntersectionCurve(ParametricSurfaceObject& surf1, ParametricSurfaceObject& surf2, ParametricSurfaceIntersection&& i)
			: Object(line), surf1(surf1), surf2(surf2), uvs1(i.get_uvs1()), uvs2(i.get_uvs2()), uvs1_line(surf1.get_u_range(), surf1.get_v_range()), uvs2_line(surf2.get_u_range(), surf2.get_v_range()), singular_crossed(i.is_singular_crossed()), too_short(i.is_too_short()) {
			name = "Intersection curve " + std::to_string(counter++);
			transformable = false;
			line.looped = i.is_looped();
			// intersection curve is generated only once (it is static), so we do not intend to add any observers or persistence bindings

			uvs1_line.set_data(this->uvs1);
			uvs2_line.set_data(this->uvs2);
			uvs1_line.color = uvs2_line.color = { 1.0f,1.0f,1.0f,1.0f };
			uvs1_line.looped = uvs2_line.looped = i.is_looped();
			surf1.trim_texture.add_line(uvs1_line);
			surf2.trim_texture.add_line(uvs2_line);
		}

		//IntersectionCurve(ParametricSurfaceObject& surf1, ParametricSurfaceObject& surf2, const std::list<Vector2>& uvs1, const std::list<Vector2>& uvs2, bool looped = false, bool singular_crossed = false, bool too_short = false)
		//	: Object(line), surf1(surf1), surf2(surf2), uvs1(uvs1.begin(), uvs1.end()), uvs2(uvs2.begin(), uvs2.end()), uvs1_line(surf1.get_u_range(), surf1.get_v_range()), uvs2_line(surf2.get_u_range(), surf2.get_v_range()), singular_crossed(singular_crossed), too_short(too_short) {
		//	name = "Intersection curve " + std::to_string(counter++);
		//	transformable = false;
		//	line.looped = looped;
		//	// intersection curve is generated only once (it is static), so we do not intend to add any observers or persistence bindings

		//	uvs1_line.set_data(this->uvs1);
		//	uvs2_line.set_data(this->uvs2);
		//	uvs1_line.color = uvs2_line.color = { 1.0f,1.0f,1.0f,1.0f };
		//	uvs1_line.looped = uvs2_line.looped = looped;
		//	surf1.trim_texture.add_line(uvs1_line);
		//	surf2.trim_texture.add_line(uvs2_line);
		//}

		//IntersectionCurve(ParametricSurfaceObject& surf1, ParametricSurfaceObject& surf2, std::vector<Vector2>&& uvs1, std::vector<Vector2>&& uvs2, bool looped = false, bool singular_crossed = false, bool too_short = false)
		//	: Object(line), surf1(surf1), surf2(surf2), uvs1(std::move(uvs1)), uvs2(std::move(uvs2)), uvs1_line(surf1.get_u_range(), surf1.get_v_range()), uvs2_line(surf2.get_u_range(), surf2.get_v_range()), singular_crossed(singular_crossed), too_short(too_short) {
		//	name = "Intersection curve " + std::to_string(counter++);
		//	transformable = false;
		//	line.looped = looped;
		//	// intersection curve is generated only once (it is static), so we do not intend to add any observers or persistence bindings

		//	uvs1_line.set_data(this->uvs1);
		//	uvs2_line.set_data(this->uvs2);
		//	uvs1_line.color = uvs2_line.color = { 1.0f,1.0f,1.0f,1.0f };
		//	uvs1_line.looped = uvs2_line.looped = looped;
		//	surf1.trim_texture.add_line(uvs1_line);
		//	surf2.trim_texture.add_line(uvs2_line);
		//}

		void bind_with(Object& object) override {}
		void remove_binding_with(Object& object) override {};

		Line2D& get_uvs_line_for(const ParametricSurfaceObject& surf) { return &surf == &surf1 ? uvs1_line : uvs2_line; }

		float intersect_with_ray(const Ray& ray) override { return NAN; }
		bool is_inside_screen_rectangle(const Rectangle& rect, const Matrix4x4& transformation) const override { return false; }

		void replace_child_by(Object& child, Object& other) override {}

		void on_delete() override {
			surf1.trim_texture.remove_line(uvs1_line);
			surf2.trim_texture.remove_line(uvs2_line);
		}

		std::pair<std::vector<Object::Handle<Point>>, Object::Handle<InterpolationSpline>> to_spline() const;
		std::vector<ObjectHandle> clone() const override;

		//static Object::Handle<IntersectionCurve> intersect_surfaces(ParametricSurfaceObject& surf1, ParametricSurfaceObject& surf2, float step, size_t max_steps, const Vector2& uv1start, const Vector2& uv2start, const bool force_loop);
		//static Vector2 find_nearest_point(const ParametricSurfaceObject& surf, const Vector3& point);
		//static Vector2 find_nearest_point(const ParametricSurfaceObject& surf, const Vector3& point, const Vector2& uvstart, const ConstantParameter is_constant = ConstantParameter::None);
		//static Vector2 find_nearest_point_far_from(const ParametricSurfaceObject& surf, const Vector2& uv_far, float step);
		//static std::pair<Vector2, Vector2> find_first_common_point(const ParametricSurfaceObject& surf1, const ParametricSurfaceObject& surf2, const Vector2& uv1start, const Vector2& uv2start);
		//static std::pair<Vector2, Vector2> find_first_common_point(const ParametricSurfaceObject& surf1, const ParametricSurfaceObject& surf2, const Vector3& hint);
		//static std::pair<Vector2, Vector2> find_first_common_point_on_self_intersection(const ParametricSurfaceObject& surf, const Vector3& hint, float step);
		static Object::Handle<IntersectionCurve> intersect_surfaces_with_hint(ParametricSurfaceObject& surf1, ParametricSurfaceObject& surf2, float step, size_t max_steps, const Vector3& hint, const bool force_loop = false);
		static Object::Handle<IntersectionCurve> intersect_surfaces_without_hint(ParametricSurfaceObject& surf1, ParametricSurfaceObject& surf2, float step, size_t max_steps, size_t sample_count_x, size_t sample_count_y, const bool force_loop = false);
		static std::list<Object::Handle<IntersectionCurve>> find_many_intersections(ParametricSurfaceObject& surf1, ParametricSurfaceObject& surf2, float step, size_t max_steps, size_t sample_count_x, size_t sample_count_y, const bool force_loop = false);
		static Object::Handle<IntersectionCurve> self_intersect_surface_with_hint(ParametricSurfaceObject& surf, float step, size_t max_steps, const Vector3& hint);
		static Object::Handle<IntersectionCurve> self_intersect_surface_without_hint(ParametricSurfaceObject& surf, float step, size_t max_steps, size_t sample_count_x, size_t sample_count_y);
	};
}