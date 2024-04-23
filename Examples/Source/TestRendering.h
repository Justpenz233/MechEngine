//
// Created by MarvelLi on 2024/3/29.
//

#pragma once
#include <imgui.h>

#include "LambdaUIWidget.h"
#include "Actors/CameraActor.h"
#include "Actors/PointLightActor.h"
#include "Game/StaticMeshActor.h"
#include "Game/World.h"
#include "ImguiToggle/imgui_toggle.h"
#include "Mesh/BasicShapesLibrary.h"
#include "Mesh/StaticMesh.h"
#include "Misc/Path.h"

class StaticMeshActor;

inline auto TestRendering()
{
	return [](World& world) {
		auto Camera = world.SpawnActor<CameraActor>("MainCamera");
		Camera->SetTranslation({-5, 0, 0});

		auto Cow = StaticMesh::LoadFromObj(Path("spot.obj"));
		Cow->Normlize();

		// auto Cow = BasicShapesLibrary::GenerateSphere(0.5);

		auto Ball = world.SpawnActor<StaticMeshActor>("Point",
			BasicShapesLibrary::GenerateSphere(0.04f));
		Ball->SetTranslation({0., 0., 1.});

		auto CowActor = world.SpawnActor<StaticMeshActor>("Cow", Cow);
		CowActor->SetTranslation({0, 0, -0.1});
		CowActor->SetRotation({M_PI_2, 0., 0.});

		static bool LightMotion = true;
		world.AddWidget<LambdaUIWidget>([=]() {
			auto NormalClipSpace = Camera->GetCameraComponent()->Project(Ball->GetTranslation());
			auto UnprojectPos = Camera->GetCameraComponent()->UnProject(NormalClipSpace);
			ImGui::Begin("Test");
			ImGui::Text("World Position: (%.2f, %.2f, %.2f)", Ball->GetTranslation().x(), Ball->GetTranslation().y(), Ball->GetTranslation().z());
			ImGui::Text("Clip Space Position: (%.2f, %.2f, %.2f)", NormalClipSpace.x(), NormalClipSpace.y(), NormalClipSpace.z());
			ImGui::Text("Unproject Position: (%.2f, %.2f, %.2f)", UnprojectPos.x(), UnprojectPos.y(), UnprojectPos.z());
			ImGui::NewLine();

			FVector2 MousePos = {ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y};
			FVector2 ViewportPos = world.GetViewport()->ScreenToViewport(MousePos);
			ImGui::Text("ViewportPos Position: (%.2f, %.2f)", ViewportPos.x(), ViewportPos.y());

			FVector2 NDCPos = world.GetViewport()->ViewportToNDC(ViewportPos);
			ImGui::Text("NDC Space: (%.2f, %.2f, %.2f, %.2f)", NDCPos.x(), NDCPos.y(), NormalClipSpace.z(), 1.);

			FVector MouseUP = Camera->GetCameraComponent()->UnProject({NDCPos.x(), NDCPos.y(), NormalClipSpace.z()});
			ImGui::Text("Mouse UnProject: (%.2f, %.2f, %.2f,)", MouseUP.x(), MouseUP.y(), MouseUP.z());

			ImGui::Toggle("Light Motion", &LightMotion);
			ImGui::End();
		});

		auto Light = world.SpawnActor<PointLightActor>("Light");
		world.TickFunction = [&, Light](float DeltaTime, World& world) {
			if(LightMotion) {
				static float TotalTime = 0;
				TotalTime += DeltaTime * 2.f;
				Light->SetTranslation({cos(TotalTime), sin(TotalTime), 1});
			}
		};
	};
}