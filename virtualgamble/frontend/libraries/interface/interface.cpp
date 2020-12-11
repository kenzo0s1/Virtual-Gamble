#include "interface.hpp"

#include <windows.h>
#include <shobjidl.h>

QuarcInterface::QuarcInterface()
{
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize - ImVec2(0, 18));
	ImGui::SetNextWindowPos({ 0, 19 });
	ImGui::Begin("##maincontent", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImGui::GetWindowPos() - ImVec2(0, 19), ImGui::GetWindowPos() + ImVec2(50, ImGui::GetIO().DisplaySize.y), ImColor(0.19f, 0.19f, 0.19f));
	}
	ImGui::End();
}
