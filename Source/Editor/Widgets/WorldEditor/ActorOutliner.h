//
// Created by MarvelLi on 2024/5/10.
//

#pragma once
#include "PrettifyName.h"
#include "UIWidget.h"
#include "Game/World.h"
#include "Widgets/WidgetUtils.h"
#include <ImguiPlus.h>

/**
 * This widget is used to display the actors' components in the world editor.
 * This is a independent widget, get selected actor from world.
 */
class ActorOutliner : public UIWidget
{
public:
	ActorOutliner() : UIWidget("ActorOutliner") {}

	virtual void Draw() override
	{
		if(!World) LOG_ERROR("World is nullptr");

		Object* SelectedObject;

		if(ImGui::Begin(ICON_FA_INFO "  Actor Outliner", nullptr))
		{
			auto Actor = World->GetSelectedActor();
			if(!Actor)
			{
				ImGui::End();
				return;
			}
			// If the selected actor is changed, unselect the previous object cause it's not selected anymore
			if (Actor != PreSelectedActor.lock().get())
			{
				PreSelectedActor = Actor->GetThis();
				if (!PreSelectedObject.expired())
				{
					PreSelectedObject.lock()->SetSelected(false);
				}
			}
			ImGui::PushID(Actor);
			ImGui::SeparatorText( UI::PretifyUIName(Actor->GetName()).c_str());
			// Here should handle the case that the actor it self is selected
			if (ImGui::Selectable("Actor", PreSelectedObject.expired(), ImGuiSelectableFlags_SpanAllColumns))
			{
				if (!PreSelectedObject.expired())
				{
					PreSelectedObject.lock()->SetSelected(false);
					PreSelectedObject.reset();
				}
			}

			for (const auto& Component : Actor->GetAllComponents())
			{
				RecursiveDrawObjectOutliner(Component.get());
			}
			ImGui::PopID();
			ImGui::End();
		}
	}


protected:
	void RecursiveDrawObjectOutliner(Object* InObject)
	{
		auto Properties = InObject->GetAllPropertyAceessors();

		// Check if the object has any child object, if not draw as leaf node
		bool bHasChildObject = false;
		for (auto Property : Properties)
		{
			if (Property.isPointerType())
			{
				auto Accessor = Property.GetPointerAccessor(InObject);
				if(auto Ptr = static_cast<Object*>(Accessor.GetPointer()))
				{
					bHasChildObject = true;
					break;
				}
			}
		}

		String ClassName = UI::PretifyUIName(InObject->ClassName());
		// String ObjectName = UI::PretifyUIName(InObject->GetName());
		// String Label = ClassName + "(" + ObjectName + ")";
		String Label = ClassName;

		auto Flag = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
		if (InObject->IsSelected()) Flag |= ImGuiTreeNodeFlags_Selected;
		if (!bHasChildObject) Flag |= ImGuiTreeNodeFlags_Leaf;

		if (ImGui::TreeNodeEx(Label.c_str(), Flag))
		{
			if (ImGui::IsItemClicked())  // Check if the TreeNode itself was clicked
			{
				if (!PreSelectedObject.expired())
				{
					PreSelectedObject.lock()->SetSelected(false);
				}
				InObject->SetSelected(true);
				PreSelectedObject = InObject->GetThis();
			}
			if (bHasChildObject)
			{
				for (auto Property : Properties)
				{
					if (Property.isPointerType())
					{
						auto Accessor = Property.GetPointerAccessor(InObject);
						if(auto Ptr = static_cast<Object*>(Accessor.GetPointer()))
						{
							RecursiveDrawObjectOutliner(Ptr);
						}
					}
				}
			}
			ImGui::TreePop();
		}
	}

	WeakObjectPtr<Actor> PreSelectedActor;
	WeakObjectPtr<Object> PreSelectedObject;
};
