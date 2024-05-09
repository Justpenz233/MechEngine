//
// Created by MarvelLi on 2024/1/21.
//

#include "WidgetUtils.h"
#include "PropertyEditor.h"
#include "Game/Actor.h"
#include "imgui.h"
#include "UI/IconsFontAwesome6.h"

void UI::DrawObjectPanel(Object* Obj)
{
	auto Properties = Obj->GetAllPropertyAceessors();
	if(Properties.empty()) return;
	ImGui::PushID(Obj);
	for (auto Property : Properties)
	{
		if(Property.isEnumType())
		{
			auto Accessor = Property.GetEnumAccessor(Obj);
			if(auto EditWidget = Widgets::PropertyEditDraw(Accessor))
				EditWidget(Accessor);
		}
		else if (Property.isPointerType())
		{
			auto Accessor = Property.GetPointerAccessor(Obj);
			if(auto Ptr = static_cast<Object*>(Accessor.GetPointer()))
			{
				if(ImGui::TreeNodeEx((ICON_FA_LINK "  " + UI::GetObjectDisplayName(Ptr)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					DrawObjectPanel(Ptr);
					ImGui::TreePop();
				}
			}
		}
		else
		{
			auto EditWidget = Widgets::PropertyEditDraw(Property);
			if(EditWidget) EditWidget(Obj, Property);
		}
	}
	ImGui::PopID();
}

void UI::DrawActorPanel(ObjectPtr<class Actor> Actor)
{
	std::string HeaderName = UI::GetObjectDisplayName(Actor) + " properties panel";

	ImGui::Begin(ICON_FA_INFO "Actor Panel",nullptr, ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5);

	if(ImGui::CollapsingHeader((ICON_FA_PEN_TO_SQUARE "  " + UI::GetObjectDisplayName(Actor)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		DrawObjectPanel(Actor.get());
		// Show all component property
		for (auto Component : Actor->GetAllComponents())
		{
			if(ImGui::TreeNodeEx((ICON_FA_CUBE "  " + UI::GetObjectDisplayName(Component)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawObjectPanel(Component.get());
				ImGui::TreePop();
			}
		}
	}
	ImGui::PopItemWidth();
	ImGui::End();
}