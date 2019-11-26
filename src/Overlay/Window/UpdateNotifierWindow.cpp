#include "UpdateNotifierWindow.h"

#include "Overlay/imgui_utils.h"
#include "Web/update_check.h"
#include "Core/info.h"


void UpdateNotifierWindow::BeforeDraw()
{
	ImGui::SetNextWindowPosCenter(ImGuiCond_Once);
	ImGui::SetNextWindowSizeConstraints(ImVec2(400, 100), ImVec2(500, 500));
}

void UpdateNotifierWindow::Draw()
{
	TextAlignedHorizontalCenter("BBTAG Improvement Mod %s has been released!", GetNewVersionNum().c_str());
	ImGui::Spacing();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 7));

	const ImVec2 buttonSize = ImVec2(120, 23);
	AlignItemHorizontalCenter(buttonSize.x);
	if (ButtonUrl("Show", MOD_LINK_FORUM, buttonSize))
	{
		Close();
	}

	AlignItemHorizontalCenter(buttonSize.x);
	if (ImGui::Button("Remind me later", buttonSize))
	{
		Close();
	}

	ImGui::PopStyleVar();

	ImGui::Spacing();
}
