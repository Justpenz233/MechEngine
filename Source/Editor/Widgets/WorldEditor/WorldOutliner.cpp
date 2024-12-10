//
// Created by MarvelLi on 2024/1/15.
//

#include "WorldOutliner.h"

#include "PrettifyName.h"
#include "imgui.h"
#include "Game/Actor.h"
#include "Game/World.h"
#include "Widgets/WidgetUtils.h"
#include "UI/IconsFontAwesome6.h"

void WorldOutliner::Draw()
{
	ASSERTMSG("WorldOutliner should be added to a World", World != nullptr);
	auto WindowSize = ImGui::GetMainViewport()->WorkSize;
	auto WindowPos = ImGui::GetMainViewport()->WorkPos;

	// Set Next window at most right
	ImGui::SetNextWindowPos(ImVec2(WindowPos.x + WindowSize.x - 300, WindowPos.y), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, WindowSize.y), ImGuiCond_FirstUseEver);

	if(ImGui::Begin( ICON_FA_EARTH_ASIA "  World Outliner", nullptr))
	{
		ObjectPtr<Actor> SelectedActor = nullptr;
		if(ImGui::BeginTable("WorldOutliner", 2, ImGuiTableFlags_BordersH))
		{
			// Table header with colorful "Name" "Type"
			ImGui::TableSetupColumn(ICON_FA_LIST "  Name");
			ImGui::TableSetupColumn("Type");
			ImGui::TableHeadersRow();

			for (const auto& Actor : World->GetAllActors())
			{
				std::string DisplayName = UI::GetObjectDisplayName(Cast<Object>(Actor));
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::PushID(Actor.get());
				if(ImGui::Selectable(DisplayName.c_str(), Actor->IsSelected(), ImGuiSelectableFlags_SpanAllColumns))
				{
					World->SelectActor(Actor);
				}
				ImGui::TableNextColumn();
				ImGui::Text("%s", UI::PretifyUIName(Actor->ClassName()).c_str());
				if(Actor->IsSelected())
				{
					SelectedActor = Actor;
				}
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		if(SelectedActor) UI::DrawActorPanel(SelectedActor);

		if(ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			if (SelectedActor)
			{
				World->DestroyActor(SelectedActor.get());
			}
		}
		ImGui::End();
	}


}