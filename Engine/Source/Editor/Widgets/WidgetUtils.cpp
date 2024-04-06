//
// Created by MarvelLi on 2024/1/21.
//

#include "WidgetUtils.h"
#include "PropertyEditor.h"
#include "Game/Actor.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "UI/IconsFontAwesome6.h"

void UI::DrawActorPanel(ObjectPtr<class Actor> Actor)
{
	std::string HeaderName = UI::GetObjectDisplayName(Actor) + " properties panel";

	ImGui::Begin((ICON_FA_INFO "  " + HeaderName).c_str(), nullptr, ImGuiTreeNodeFlags_DefaultOpen);
	if(ImGui::CollapsingHeader((ICON_FA_PEN_TO_SQUARE "  " + UI::GetObjectDisplayName(Actor)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushID(Actor.get());
		for (auto Property : Actor->GetAllPropertyAceessors())
		{
			auto EditWidget = Widgets::PropertyEditDraw(Property);
			if(EditWidget)
				EditWidget(Actor.get(), Property);
		}
		ImGui::PopID();
		// Show all component property
		for (auto Component : Actor->GetAllComponents())
		{
			auto Properties = Component->GetAllPropertyAceessors();
			if(Properties.empty()) continue;
			if(ImGui::TreeNodeEx((ICON_FA_CUBE "  " + UI::GetObjectDisplayName(Component)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushID(Component.get());
				for (auto Property : Properties)
				{
					auto EditWidget = Widgets::PropertyEditDraw(Property);
					if(EditWidget)
						EditWidget(Component.get(), Property);
				}
				ImGui::PopID();
				ImGui::TreePop();
			}
		}
	}
	ImGui::End();
}