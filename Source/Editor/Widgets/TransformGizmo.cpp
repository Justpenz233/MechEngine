#include "TransformGizmo.h"
#include "Imguizmo/Imguizmo.h"
#include "Game/Actor.h"
#include "UI/UIWidget.h"
#include "imgui.h"
#include "Game/World.h"
#include "Render/GpuSceneInterface.h"
#include "Render/SceneProxy/CameraSceneProxy.h"

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
	Eigen::Matrix4f view = World->GetScene()->GetCameraProxy()->GetViewMatrix().cast<float>();
	Eigen::Matrix4f proj = World->GetScene()->GetCameraProxy()->GetProjectionMatrix().cast<float>();
	const Eigen::Matrix4f T0 = SelectedActor->GetTransformMatrix().cast<float>();
	Eigen::Matrix4f T = T0.eval();
	auto WindowSize = ImGui::GetMainViewport()->WorkSize;
	auto WindowPos = ImGui::GetMainViewport()->Pos;
	ImGuizmo::SetRect(WindowPos.x, WindowPos.y, WindowSize.x, WindowSize.y);
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