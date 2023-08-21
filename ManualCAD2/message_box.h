#pragma once
#include "window.h"
#include <string>

namespace ManualCAD
{
	class MessageBox {
		MessageBox() = delete;
	public:

		enum class Result {
			None, Yes, No
		};

		// Popup has to be registered after show_popup call
		static void register_popup_error(const char* title, const char* msg) {
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, { 0.6f,0.125f,0.125f,1.0f });
			ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.4f,0.1f,0.1f,1.0f });
			if (ImGui::BeginPopupModal(title))
			{
				/*ImDrawList* draw_list = ImGui::GetWindowDrawList();
				ImU32 red = ImGui::GetColorU32(IM_COL32(255, 0, 0, 255));
				ImU32 white = ImGui::GetColorU32(IM_COL32(255, 255, 255, 255));
				const ImVec2 p = ImGui::GetCursorScreenPos();

				draw_list->AddCircleFilled({ p.x + 50,p.y + 50 }, 25, red);
				draw_list->AddLine({ p.x + 38,p.y + 38 }, { p.x + 62,p.y + 62 }, white, 2.0f);
				draw_list->AddLine({ p.x + 38,p.y + 62 }, { p.x + 62,p.y + 38 }, white, 2.0f);*/

				ImGui::Text("%s", msg);
				if (ImGui::Button("OK")) 
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}
		static void register_popup_question(const char* title, const char* msg, Result& result) {
			if (ImGui::BeginPopupModal(title))
			{
				ImGui::Text("%s", msg);
				if (ImGui::Button("Yes")) {
					result = Result::Yes;
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("No")) {
					result = Result::No;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
		static void show_popup(const char* title) {
			ImGui::OpenPopup(title);
		}
	};
}