#include "parameter_space_view_window.h"

namespace ManualCAD
{
	void ParameterSpaceViewWindow::build()
	{
		ImGui::SetNextWindowSize(ImVec2(ApplicationSettings::TRIM_TEXTURE_SIZE_PIXELS, ApplicationSettings::TRIM_TEXTURE_SIZE_PIXELS), ImGuiCond_FirstUseEver);
		ImGui::Begin("Parameter space", &visible);
		if (surface == nullptr)
		{
			ImGui::Text("No surface selected!");
		}
		else
		{
			help_marker("Click on a region to toggle visibility.");
			ImGui::SameLine();
			ImGui::SeparatorText(surface->name.c_str());
			ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
			ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
			auto& io = ImGui::GetIO();
			
			//// draw curves to texture
			//GLint old_viewport[4];
			//glGetIntegerv(GL_VIEWPORT, old_viewport);

			//texture.set_size(canvas_sz.x, canvas_sz.y);

			//fbo.bind();
			//glViewport(0, 0, canvas_sz.x, canvas_sz.y);
			//glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//
			//for (const auto& curve : surface->intersection_curves)
			//	curve->get_uvs_line_for(*surface).render(renderer, canvas_sz.x, canvas_sz.y, 1.0f);
			//fbo.unbind();

			//glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);

			// draw texture to imgui
			//ImGui::Image((void*)(intptr_t)texture.get_id(), canvas_sz);
			surface->trim_texture.render(renderer);
			ImGui::Image((void*)(intptr_t)surface->trim_texture.get_texture().get_id(), canvas_sz);
			const bool is_hovered = ImGui::IsItemHovered();
			const ImVec2 position(
				(io.MousePos.x - canvas_p0.x) * surface->trim_texture.get_width() / canvas_sz.x, 
				(io.MousePos.y - canvas_p0.y) * surface->trim_texture.get_height() / canvas_sz.y);
			if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				surface->trim_texture.toggle_region(position.x, position.y);
			}
		}
		ImGui::End();
	}

}