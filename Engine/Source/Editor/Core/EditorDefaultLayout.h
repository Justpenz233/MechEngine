#pragma once

#include "UI/ImguiPlugin.h"
#include "Widgets/MenuBarWidgets/MenuBar.h"
#include "Widgets/MenuBarWidgets/GroundWidget.h"
#include "Widgets/TransformGizmo.h"
#include "Widgets/MenuBarWidgets/SaveMeshMenuBar.h"
#include "Widgets/WorldEditor/WorldOutliner.h"

inline void LoadDefaultEditorLayout(World* CurrentWorld)
{
	auto MainGizmoManager = CurrentWorld->AddWidget<TransformGizmo>();
	auto MainMenuBar = CurrentWorld->AddWidget<MenuBar>("MainMenuBar");

	MainMenuBar->AddItem("View/Edit Object", "", { [=]() {
		MainGizmoManager->ToggleVisible();
	} });

	MainMenuBar->AddItem<SaveMeshMenuBar>("File/Save Selected Mesh", "");

	CurrentWorld->AddWidget<WorldOutliner>();
}