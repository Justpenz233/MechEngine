//
// Created by MarvelLi on 2024/1/21.
//

#pragma once
#include "Core/CoreMinimal.h"
#include "Object/Object.h"
namespace MechEngine::UI
{
	inline String GetObjectDisplayName(Object* Obj)
	{
		std::string DisplayName = Obj->GetName();
		if(DisplayName.empty()) return Obj->ClassName();
		// add space between upper and lower case
		for(int i = 1; i < DisplayName.size(); i++)
		{
			if(isupper(DisplayName[i]) && islower(DisplayName[i - 1]))
			{
				DisplayName.insert(i, " ");
				i++;
			}
		}
		return DisplayName;
	}

	inline String GetObjectDisplayName(ObjectPtr<Object> Obj)
	{
		return GetObjectDisplayName(Obj.get());
	}


	void DrawActorPanel(ObjectPtr<Actor> Actor);

	void DrawObjectPanel(class Object* Obj);

} // namespace MechEngine::Editor