#pragma once

// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "raycaster.h"
#include <imgui.h>
#include <stdio.h>
#include <list>
#include <memory>
#include "window.h"
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif
#include "application_settings.h"
#include "renderer.h"
#include "cursor.h"
#include "settings_window.h"
#include "parameter_space_view_window.h"
#include "task.h"

namespace ManualCAD
{
	class GlApplication {
	public:
		explicit GlApplication(int width, int height, const char* title, const ImVec4& clear_color, /*Raycaster& raycaster, */ Renderer& renderer);
		void maximize();
		void run();
		void dispose();

		void add_window(const WindowHandle& window_ptr) { windows.push_back(window_ptr); }

		void add_internal_windows() {
			add_window(make_window<SceneSettingsWindow>(*cursor, renderer, clear_color, display_w, display_h));
			add_window(current_object_settings);
			add_window(controller.create_settings_window(renderer.get_camera(), *cursor, current_object_settings));
			add_window(parameter_view);
		}
		ImVec4& get_background_color() { return clear_color; }
		ImGuiIO& get_io() { return ImGui::GetIO(); }
	private:
		GLFWwindow* main_window;
		//Raycaster& raycaster;
		Renderer& renderer;
		ObjectController controller;
		TaskManager task_manager;
		std::unique_ptr<Cursor> cursor;
		int display_w, display_h;
		
		WHandle<ParameterSpaceViewWindow> parameter_view;
		WindowHandle current_object_settings;

		std::unique_ptr<WireframeMesh> grid;
		std::unique_ptr<SimpleRect> selection_rect;

		bool selection_dragging = false;

		std::list<WindowHandle> windows;

		ImVec4 clear_color;
	};
}