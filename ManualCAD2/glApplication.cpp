#include "glApplication.h"
#include "exception.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace ManualCAD
{
	static void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	}

	GlApplication::GlApplication(int width, int height, const char* title, const ImVec4& clear_color, /*Raycaster& raycaster, */ Renderer& renderer) : clear_color(clear_color), /*raycaster(raycaster),*/ renderer(renderer), controller(task_manager) {
		if constexpr (ApplicationSettings::DEBUG)
			printf("[INFO] Running in DEBUG mode");
		
		glfwSetErrorCallback(glfw_error_callback);
		if (!glfwInit())
			THROW_EXCEPTION;

		// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
		const char* glsl_version = "#version 100";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
	// GL 3.2 + GLSL 150
		const char* glsl_version = "#version 150";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
		const char* glsl_version = "#version 130";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);  // 3.2+ only
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	// Create window with graphics context
		main_window = glfwCreateWindow(width, height, title, NULL, NULL);
		if (main_window == NULL)
			THROW_EXCEPTION;
		glfwMakeContextCurrent(main_window);
		glfwSwapInterval(1); // Enable vsync

		//raycaster.init(ApplicationSettings::INITIAL_DOWNSAMPLING_SCALE);// initialize raycaster
		renderer.init();

		cursor = std::make_unique<Cursor>();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(main_window, true);
		ImGui_ImplOpenGL3_Init(glsl_version);

		// Load Fonts
		// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
		// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
		// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
		// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
		// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
		// - Read 'docs/FONTS.md' for more instructions and details.
		// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
		// - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
		//io.Fonts->AddFontDefault();
		//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
		//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
		//IM_ASSERT(font != NULL);

		grid = std::make_unique<WireframeMesh>();
		grid->generate_grid(100, 100, 1.0f, 1.0f);
		grid->color = { 0.5f,0.5f,0.5f,1.0f };

		selection_rect = std::make_unique<SimpleRect>();

		parameter_view = make_window<ParameterSpaceViewWindow>(renderer);
		current_object_settings = controller.create_current_object_settings_window(*parameter_view);
	}

	void GlApplication::maximize()
	{
		glfwMaximizeWindow(main_window);
	}

	void GlApplication::run() {
		// Main loop
#ifdef __EMSCRIPTEN__
	// For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
	// You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
		io.IniFilename = NULL;
		EMSCRIPTEN_MAINLOOP_BEGIN
#else
		while (!glfwWindowShouldClose(main_window))
#endif
		{
			// Poll and handle events (inputs, window resize, etc.)
			// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
			// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
			// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
			// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
			glfwPollEvents();

			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			auto& io = ImGui::GetIO();
			// zoom using mouse wheel
			if (!io.WantCaptureMouse && io.MouseWheel != 0.0f) {
				renderer.get_camera().zoom(powf(ApplicationSettings::ZOOM_FACTOR, io.MouseWheel));
				//raycaster.reset_downsampling();
			}

			// Build windows from list
			for (auto w : windows) {
				if (w->visible) {
					w->build();
				}
			}

			// Rendering
			ImGui::Render();
			glfwGetFramebufferSize(main_window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			auto& selected_objects = controller.get_selected_objects();
			bool selection_transformable = selected_objects.is_transformable();

			if (!io.WantCaptureMouse) {
				bool selection_hovered = selected_objects.is_hovered(io.MousePos.x, io.MousePos.y, renderer.get_camera(), display_w, display_h);

				if (selection_hovered) {
					ImGui::SetMouseCursor(ApplicationSettings::SELECTION_HOVER_CURSOR);
				}
				// move scene
				if (ImGui::IsMouseDragging(ApplicationSettings::MOVE_BUTTON)) {
					renderer.get_camera().move_by({ -io.MouseDelta.x / display_w * ApplicationSettings::MOVE_SPEED, io.MouseDelta.y / display_h * ApplicationSettings::MOVE_SPEED, 0 });
					//if (io.MouseDelta.x > 0 || io.MouseDelta.y > 0)
					//	raycaster.reset_downsampling();
				}
				// rotate scene
				if (ImGui::IsMouseDown(ApplicationSettings::ROTATE_BUTTON)) {
					renderer.get_camera().rotate(io.MouseDelta.y * ApplicationSettings::ROTATE_SPEED, -io.MouseDelta.x * ApplicationSettings::ROTATE_SPEED, 0);
					//if (io.MouseDelta.x > 0 || io.MouseDelta.y > 0)
					//	raycaster.reset_downsampling();
				}
				// set cursor
				if (ImGui::IsMouseClicked(ApplicationSettings::SET_CURSOR_BUTTON) && ImGui::IsKeyDown(ApplicationSettings::SET_CURSOR_KEY)) {
					cursor->set_screen_position_from_pixels(io.MousePos.x, io.MousePos.y, renderer.get_camera(), display_w, display_h);
				}
				// select object
				else if (ImGui::IsMouseClicked(ApplicationSettings::SELECT_OBJECT_BUTTON)) {
					if (!selection_hovered) {
						selection_dragging = false;
						int i = controller.get_nearest_object_from_pixels(io.MousePos.x, io.MousePos.y, renderer.get_camera(), display_w, display_h);
						if (i != -1) {
							if (ImGui::IsKeyDown(ApplicationSettings::ADD_TO_SELECTION_KEY)) {
								if (controller.is_selected(i)) {
									controller.remove_from_selection(i);
								}
								else {
									controller.add_to_selection(i);
								}
							}
							else if (ImGui::IsKeyDown(ApplicationSettings::BIND_TO_SELECTED_KEY)) {
								if (!controller.is_selected(i)) {
									selected_objects.bind_with(controller.get_object(i));
								}
							}
							else {
								controller.select_object(i);
							}
						}
						else if (!ImGui::IsKeyDown(ApplicationSettings::ADD_TO_SELECTION_KEY)) {
							controller.clear_selection();
						}
						current_object_settings->visible = controller.selected_objects_count() > 0;
					}
					else {
						selection_dragging = true;
					}
				}

				selection_rect->visible = false;
				if (ImGui::IsMouseDragging(ApplicationSettings::SELECT_OBJECT_BUTTON))
				{
					// move selected objects
					if (selection_transformable && selection_dragging) {
						if (ImGui::IsKeyDown(ApplicationSettings::MOVE_ALONG_X_AXIS)) {
							selected_objects.set_screen_position_from_pixels_along_x_axis(io.MousePos.x, io.MousePos.y, renderer.get_camera(), display_w, display_h);
						}
						else if (ImGui::IsKeyDown(ApplicationSettings::MOVE_ALONG_Y_AXIS)) {
							selected_objects.set_screen_position_from_pixels_along_y_axis(io.MousePos.x, io.MousePos.y, renderer.get_camera(), display_w, display_h);
						}
						else if (ImGui::IsKeyDown(ApplicationSettings::MOVE_ALONG_Z_AXIS)) {
							selected_objects.set_screen_position_from_pixels_along_z_axis(io.MousePos.x, io.MousePos.y, renderer.get_camera(), display_w, display_h);
						}
						else
							selected_objects.set_screen_position_from_pixels(io.MousePos.x, io.MousePos.y, renderer.get_camera(), display_w, display_h);
					}
					// select by drag (rectangle)
					else {
						auto delta = ImGui::GetMouseDragDelta(ApplicationSettings::SELECT_OBJECT_BUTTON);
						auto start = io.MouseClickedPos[ApplicationSettings::SELECT_OBJECT_BUTTON];

						Rectangle rect;
						if (delta.x < 0) {
							rect.x_min = start.x + delta.x;
							rect.x_max = start.x;
						}
						else {
							rect.x_max = start.x + delta.x;
							rect.x_min = start.x;
						}
						if (delta.y < 0) {
							rect.y_max = start.y + delta.y;
							rect.y_min = start.y;
						}
						else {
							rect.y_min = start.y + delta.y;
							rect.y_max = start.y;
						}
						rect.x_min = 2.0f * rect.x_min / display_w - 1.0f;
						rect.x_max = 2.0f * rect.x_max / display_w - 1.0f;
						rect.y_min = 1.0f - 2.0f * rect.y_min / display_h;
						rect.y_max = 1.0f - 2.0f * rect.y_max / display_h;

						selection_rect->position = { 2.0f * start.x / display_w - 1.0f, 1.0f - 2.0f * start.y / display_h };
						selection_rect->scale_x = 2.0f * delta.x / display_w;
						selection_rect->scale_y = -2.0f * delta.y / display_h;
						selection_rect->visible = true;

						if (ImGui::IsKeyDown(ApplicationSettings::ADD_TO_SELECTION_KEY))
							controller.toggle_selection_objects_inside(rect, renderer.get_camera(), display_w, display_h);
						else
							controller.select_objects_inside(rect, renderer.get_camera(), display_w, display_h);
						current_object_settings->visible = controller.selected_objects_count() > 0;
					}
				}
				if (ImGui::IsMouseReleased(ApplicationSettings::SELECT_OBJECT_BUTTON))
				{
					controller.end_multi_selection();
				}
			}

			renderer.disable_depth_buffer_write();

			// render grid
			renderer.add(*grid);

			renderer.enable_depth_buffer_write();

			// render cursor
			renderer.add(cursor->get_renderable());

			// draw objects
			//raycaster.render(display_w, display_h);
			auto objects = controller.get_objects();

			// perform tasks
			task_manager.execute_tasks();

			// acquire renderable list
			std::vector<const Renderable*> renderables(objects.size());
			for (int i = 0; i < objects.size(); ++i)
			{
				if (objects[i].selected)
					renderables[i] = &objects[i].object
					->get_renderable(selected_objects.get_transformation(), selected_objects.get_world_position() - selected_objects.get_transformation().position);
				else
					renderables[i] = &objects[i].object->get_renderable();
			}
			// now render; we have to do with two passes in order to update all model matrices (necessary for bindings such as Bezier Curves)
			for (int i = 0; i < objects.size(); ++i)
			{
				if (!renderables[i]->visible) continue;
				if (objects[i].selected)
					renderer.add(*renderables[i], ApplicationSettings::SELECTED_THICKNESS);
				else
					renderer.add(*renderables[i], ApplicationSettings::NORMAL_THICKNESS);
			}


			if (selection_transformable && !selected_objects.empty())
			{
				renderer.disable_depth_testing();
				renderer.add(selected_objects.get_point_set(), ApplicationSettings::SELECTED_THICKNESS);
				renderer.enable_depth_testing();
			}
			if (selection_rect->visible)
				renderer.add(*selection_rect, ApplicationSettings::SELECTED_THICKNESS);

			renderer.render_all(display_w, display_h);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(main_window);
		}
#ifdef __EMSCRIPTEN__
		EMSCRIPTEN_MAINLOOP_END;
#endif
	}

	void GlApplication::dispose() {
		// Cleanup
		//raycaster.dispose();
		for (auto& w : windows)
			w->dispose();

		renderer.dispose();
		grid->dispose();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(main_window);
		glfwTerminate();
	}
}