//
// Created by MarvelLi on 2024/2/29.
//

#pragma once

#include "Editor.h"
#include "MenuBarItem.h"
#include "Widgets/OsDialogs.h"
#include "Components/StaticMeshComponent.h"
#include "Game/Actor.h"

class SaveMeshMenuBar : public MenuBarItem
{
public:

	SaveMeshMenuBar()
		: MenuBarItem(NAME(SaveMeshMenuBar)) {}

	virtual void OnClick() override
	{
		if(auto SelectedActor = GEditor.GetWorld()->GetSelectedActor())
		{
			if(auto MeshComponent = SelectedActor->GetComponent<StaticMeshComponent>())
			{
				auto SaveFilePath = SaveFileDialog("Save Mesh", "Mesh.obj", ObjFilesFilter);
				MeshComponent->GetMeshData()->SaveOBJ(SaveFilePath.result());
			}
		}
	}

	virtual void Draw() override {}
};
