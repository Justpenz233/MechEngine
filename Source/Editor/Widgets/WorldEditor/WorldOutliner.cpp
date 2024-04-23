//
// Created by MarvelLi on 2024/1/15.
//

#include "WorldOutliner.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Game/Actor.h"
#include "Game/World.h"
#include "Widgets/WidgetUtils.h"
#include "UI/IconsFontAwesome6.h"

void WorldOutliner::Draw()
{
	auto WindowSize = ImGui::GetMainViewport()->WorkSize;
	auto WindowPos = ImGui::GetMainViewport()->WorkPos;

	// Set Next window at most right
	ImGui::SetNextWindowPos(ImVec2(WindowPos.x + WindowSize.x - 300, WindowPos.y));
	ImGui::SetNextWindowSize(ImVec2(300, WindowSize.y * 0.4));

	ImGui::Begin( ICON_FA_EARTH_ASIA "  World Outliner", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::BeginTable("WorldOutliner", 2, ImGuiTableFlags_BordersH);

	// Table header with colorful "Name" "Type"
	ImGui::TableSetupColumn(ICON_FA_LIST "  Name");
	ImGui::TableSetupColumn("Type");
	ImGui::TableHeadersRow();

	for (auto Actor : World->GetAllActors())
	{
		std::string DisplayName = UI::GetObjectDisplayName(Cast<Object>(Actor));
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		if(ImGui::Selectable(DisplayName.c_str(), Actor->IsSelected(), ImGuiSelectableFlags_SpanAllColumns))
		{
			World->SelectActor(Actor);
		}
		ImGui::TableNextColumn();
		ImGui::Text("%s", Actor->ClassName().c_str());
		if(Actor->IsSelected())
		{
			ImGui::SetNextWindowPos(ImVec2(WindowPos.x + WindowSize.x - 300, WindowPos.y + WindowSize.y * 0.4));
			ImGui::SetNextWindowSize(ImVec2(300, WindowSize.y * 0.6));
			UI::DrawActorPanel(Actor);
		}
	}
	ImGui::EndTable();
	ImGui::End();

}