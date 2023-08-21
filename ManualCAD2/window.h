#pragma once

#include <imgui.h>
#include <memory>

#define make_window std::make_shared

namespace ManualCAD
{
	class Window {
	protected:
		static void help_marker(const char* desc)
		{
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(desc);
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		}
	public:
		bool visible = true;
		virtual void build() = 0;
		virtual void dispose() {}
	};

	class ImGuiDemoWindow : public Window {
		virtual void build() override {
			ImGui::ShowDemoWindow(&visible);
		}
	};

	class SimpleWindow : public Window {
	public:
		explicit SimpleWindow(bool& show_demo_window, bool& show_another_window, ImVec4& clear_color) {
			p_show_demo_window = &show_demo_window;
			p_show_another_window = &show_another_window;
			p_clear_color = &clear_color;
		}

		virtual void build() override {
			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", p_show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", p_show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)p_clear_color);        // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
	private:
		bool* p_show_demo_window;
		bool* p_show_another_window;
		ImVec4* p_clear_color;

		float f = 0.0f;
		int counter = 0;
	};

	class AnotherWindow : public Window {
		virtual void build() override {
			ImGui::Begin("Another Window", &visible);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				visible = false;
			ImGui::End();
		}
	};

	template <class T>
	using WHandle = std::shared_ptr<T>;
	using WindowHandle = WHandle<Window>;
}