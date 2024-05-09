#pragma once

#include "LambdaUIWidget.h"
#include "Widgets/MenuBarWidgets/MenuBar.h"
#include "Widgets/TransformGizmo.h"
#include "Widgets/MenuBarWidgets/SaveMeshMenuBar.h"
#include "Widgets/WorldEditor/WorldOutliner.h"
#include "UI/ImguiPlus.h"
#include "Widgets/WorldEditor/ViewGizmo.h"

inline void LoadDefaultEditorLayout(World* CurrentWorld)
{
	auto MainGizmoManager = CurrentWorld->AddWidget<TransformGizmo>();
	auto MainViewGizmo = CurrentWorld->AddWidget<ViewGizmo>();
	auto MainMenuBar = CurrentWorld->AddWidget<MenuBar>("MainMenuBar");

	MainMenuBar->AddItem("View/Edit Object", "", { [=]() {
		MainGizmoManager->ToggleVisible();
	} });

	MainMenuBar->AddItem("View/Buffer/FrameBuffer", "", { [=]() {
		CurrentWorld->SetViewMode(FrameBuffer);
	}});

	MainMenuBar->AddItem("View/Buffer/Depth", "", { [=]() {
		CurrentWorld->SetViewMode(DepthBuffer);
	}});

	MainMenuBar->AddItem("View/Buffer/NormalWorld", "", { [=]() {
		CurrentWorld->SetViewMode(NormalWorldBuffer);
	}});

	MainMenuBar->AddItem("View/Buffer/BaseColor", "", { [=]() {
		CurrentWorld->SetViewMode(BaseColorBuffer);
	}});

	MainMenuBar->AddItem<SaveMeshMenuBar>("File/Export Selected Mesh", "");
	MainMenuBar->AddItem<SaveSceneMenuBar>("File/Export Scene", "");

	CurrentWorld->AddWidget<WorldOutliner>();

	// Handle notifications window
	CurrentWorld->AddWidget<LambdaUIWidget>([]() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.2f);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.01f, 0.0f, 0.1f, 1.00f));

		ImGui::RenderNotifications();

		ImGui::PopStyleColor(1);

		ImGui::PopStyleVar(2);

	});
}
