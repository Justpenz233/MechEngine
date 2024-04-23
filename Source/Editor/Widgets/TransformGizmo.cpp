#include "TransformGizmo.h"
#include "Imguizmo/Imguizmo.h"
#include "Game/Actor.h"
#include "UI/UIWidget.h"
#include "imgui.h"
#include "Game/World.h"
#include "Render/RendererInterface.h"
#include <iostream>

void TransformGizmo::Draw()
{
	if(ImGui::IsKeyPressed(ImGuiKey_G))
		ToggleVisible();
	if(ImGui::IsKeyPressed(ImGuiKey_W))
		operation = ImGuizmo::OPERATION::TRANSLATE;
	if(ImGui::IsKeyPressed(ImGuiKey_E))
		operation = ImGuizmo::OPERATION::ROTATE;
	if(ImGui::IsKeyPressed(ImGuiKey_S))
		operation = ImGuizmo::OPERATION::SCALE;

	if(!Visible) return;
	// Add new
	if(!World) return;

	auto SelectedActor = World->GetSelectedActor();
	if(SelectedActor == nullptr) return;


	int ThisId;
	if(GizemoID.contains(SelectedActor))
		ThisId = GizemoID[SelectedActor];
	else
	{
		ThisId = TotalId++;
		GizemoID[SelectedActor] = ThisId;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGuizmo::BeginFrame();
	ImGui::PopStyleVar();

	ImGuizmo::SetID(ThisId);

	// Eigen::Matrix4f view = (viewer->core().view / viewer->core().camera_zoom);
	Eigen::Matrix4f view = World->GetScene()->GetViewMatrix().cast<float>();
	Eigen::Matrix4f proj = World->GetScene()->GetProjectionMatrix().cast<float>();
	const Eigen::Matrix4f T0 = SelectedActor->GetTransformMatrix().cast<float>();
	Eigen::Matrix4f T = T0.eval();
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	ImGuizmo::AllowAxisFlip(false);
	ImGuizmo::Manipulate(view.data(), proj.data(), TransformGizmo::operation, ImGuizmo::LOCAL, T.data(), NULL, NULL);
	const float diff = (T - T0).array().abs().maxCoeff();
	// Only call if actually changed; otherwise, triggers on all mouse events
	if (LastUsing == ThisId && diff > 1e-7)
	{
		SelectedActor->SetTransform(Affine3d(T.cast<double>()));
	}
	if (ImGuizmo::IsUsing()) LastUsing = ThisId;
}