#pragma once
#include <memory>
#include <vector>
#include <set>
#include "object.h"
#include "window.h"
#include "message_box.h"
#include "rectangle.h"
#include "parameter_space_view_window.h"
#include "task.h"

namespace ManualCAD
{
	struct ObjectInfo {
		Object* object;
		bool selected;
	};

	class Serializer;

	class ObjectController {
		friend class Serializer;

		std::set<int> selected_indices;
		std::set<int> selected_indices_snapshot;
		bool selected_indices_snapshot_valid = false;
		std::vector<ObjectHandle> objects;
		ObjectHandle preview;
		TaskManager& task_manager;
		bool preview_present = false;
		bool unsaved = false;

		std::unique_ptr<ObjectCollection> selected_objects;
		bool is_collection_valid = false;

		void on_selection_changed() {
			if (is_collection_valid)
				selected_objects->apply_transformation_to_objects();
			is_collection_valid = false;
			unsaved = true;
			unset_preview();
		}
	public:
		ObjectController(TaskManager& task_manager) : task_manager(task_manager) { }

		bool is_preview_present() const { return preview_present; }
		bool is_unsaved() const { return unsaved; }
		void set_saved() { unsaved = false; }
		void set_preview(ObjectHandle&& obj) {
			preview = std::move(obj);
			preview_present = true;
		}
		Object& get_preview() {
			return *preview;
		}
		void unset_preview() {
			preview_present = false;
		}

		size_t get_object_count() const { return objects.size(); }
		std::vector<ObjectInfo> get_objects();
		Object& get_object(int idx) { return *objects[idx]; }
		int get_index(const Object& obj) const {
			for (int i = 0; i < objects.size(); ++i)
			{
				if (&obj == objects[i].get())
					return i;
			}
			return -1;
		}
		ObjectCollection& get_selected_objects() {
			if (!is_collection_valid)
			{
				std::vector<Object*> res(selected_indices.size());
				int i = 0;
				for (int idx : selected_indices)
				{
					res[i++] = objects[idx].get();
				}
				selected_objects = std::make_unique<ObjectCollection>(res);
				is_collection_valid = true;
			}
			return *selected_objects;
		}
		void add_object(ObjectHandle&& obj) {
			if (is_collection_valid)
				selected_objects->bind_with(*obj);
			objects.push_back(std::move(obj));
			unsaved = true;
		}

		template <class Collection>
		void add_objects(Collection& objs) {
			for (auto&& obj : objs)
				add_object(std::move(obj));
		}

		void clear_all() {
			is_collection_valid = false;
			unset_preview();
			objects.clear();
			selected_indices.clear();
			unsaved = true;
		}

		void remove_selected_objects() {
			on_selection_changed();

			auto s_it = selected_indices.begin();
			auto it = objects.begin();
			int i = 0;
			while (s_it != selected_indices.end()) {
				if (i == *s_it) {
					auto& to_delete = *it->get();
					if (!to_delete.is_persistent())
					{
						for (auto& obj : objects)
						{
							obj->remove_binding_with(to_delete);
						}

						to_delete.on_delete();
						it = objects.erase(it);

						// usuwanie kolekcji punktów razem z krzyw¹
						for (int j = 0; j < to_delete.children_count(); ++j)
						{
							if ((*it)->is_illusory())
							{
								it = objects.erase(it); // kolekcja jest zawsze za krzyw¹
								++i;
							}
						}
					}
					else
					{
						it++;
					}
					s_it++;
				}
				else {
					it++;
				}
				++i;
			}
			selected_indices.clear();
			is_collection_valid = false;
		}
		int get_nearest_object_from_pixels(float x, float y, const Camera& camera, int width, int height);

		void toggle_selection_objects_inside(const Rectangle& rect, const Camera& camera, int width, int height);
		void select_objects_inside(const Rectangle& rect, const Camera& camera, int width, int height) { selected_indices.clear(); toggle_selection_objects_inside(rect, camera, width, height); }
		void end_multi_selection() { selected_indices_snapshot.clear(); selected_indices_snapshot_valid = false; }

		void clear_selection() { selected_indices.clear(); on_selection_changed(); }

		template <class Obj, template <class T> class Container>
		void select_all_from(const Container<Obj*>& objs) { // TODO: make it work also in non-sorted containers
			selected_indices.clear();
			auto it = objs.begin();
			for (int i = 0; i < objects.size(); ++i)
			{
				if (it == objs.end()) break;
				if (objects[i].get() == *it && !objects[i]->is_illusory())
				{
					selected_indices.insert(i);
					++it;
				}
			}
			on_selection_changed();
		}

		void select_observing(const Object& observer) {
			selected_indices.clear();
			for (int i = 0; i < objects.size(); ++i)
			{
				if (objects[i]->is_observed_by(observer))
					selected_indices.insert(i);
			}
			on_selection_changed();
		}

		void select_object(int idx) { selected_indices.clear(); selected_indices.insert(idx); on_selection_changed(); }
		void add_to_selection(int idx) {
			if (selected_indices.empty() || (!objects[*selected_indices.cbegin()]->is_illusory() && !objects[idx]->is_illusory()))
			{
				selected_indices.insert(idx);
				on_selection_changed();
			}
		}
		void remove_from_selection(int idx) { selected_indices.erase(idx); on_selection_changed(); }

		bool is_selected(int idx) const { return selected_indices.find(idx) != selected_indices.cend(); }
		size_t selected_objects_count() const { return selected_indices.size(); }

		WindowHandle create_settings_window(Camera& camera, const Cursor& cursor, WindowHandle& object_settings_window);
		WindowHandle create_current_object_settings_window(ParameterSpaceViewWindow& parameter_view);

		// Serialization
		void add_objects_to_serializer(Serializer& serializer) const;

		void merge_points(Point& p1, Point& p2);
	};
}