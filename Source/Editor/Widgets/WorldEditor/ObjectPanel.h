//
// Created by MarvelLi on 2024/5/11.
//

#pragma once

#include "PrettifyName.h"
#include "UIWidget.h"
#include <ImguiPlus.h>
#include "Game/World.h"
#include "Widgets/PropertyEditor.h"
#include "Widgets/WidgetUtils.h"

class ObjectPanel : public UIWidget
{
public:
	ObjectPanel() : UIWidget("ObjectPanel") {}

	virtual void Draw() override
	{
		if(!World) LOG_ERROR("World is nullptr");
		auto Actor = World->GetSelectedActor();
		if(ImGui::Begin(ICON_FA_INFO "  Object Panel", nullptr))
		{
			if(!Actor)
			{
				ImGui::End();
				return;
			}
			if (auto SelectedObject = GetActorSelectedChild(Actor))
				DrawObjectPanel(SelectedObject);
			else
				DrawObjectPanel(Actor);

			ImGui::End();
		}
	}

protected:
	Object* GetObjectSelectedChild(Object* Obj)
	{
		auto Properties = Obj->GetAllPropertyAccessors();
		for (auto Property : Properties)
		{
			if (Property.isPointerType())
			{
				auto Accessor = Property.GetPointerAccessor(Obj);
				if(auto Ptr = static_cast<Object*>(Accessor.GetPointer()))
				{
					if (Ptr->IsSelected())
					{
						return Ptr;
					}
					else
					{
						if (auto SelectedObject = GetObjectSelectedChild(Ptr))
						{
							return SelectedObject;
						}
					}
				}
			}
		}
		return nullptr;
	}

	// Get the only one selected object starting from the Actor owner tree
	Object* GetActorSelectedChild(Actor* Actor)
	{
		// First check the object inside the actor
		if (auto SelectedObject = GetObjectSelectedChild(Actor))
		{
			return SelectedObject;
		}

		for (const auto& Component : Actor->GetAllComponents())
		{
			if (Component->IsSelected())
			{
				return Component.get();
			}
			if (auto SelectedObject = GetObjectSelectedChild(Component.get()))
			{
				return SelectedObject;
			}
		}
		return nullptr;
	}

	static void DrawObjectPanel(Object* Obj)
	{
		auto Properties = Obj->GetAllPropertyAccessors();
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
				// Don't unfold cause it's not selected
				continue;
			}
			else
			{
				auto EditWidget = Widgets::PropertyEditDraw(Property);
				if(EditWidget) EditWidget(Obj, Property);
			}
		}
		ImGui::PopID();
	}
};