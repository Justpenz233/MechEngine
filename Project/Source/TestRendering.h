//
// Created by MarvelLi on 2024/3/29.
//

#pragma once
#include <imgui.h>
#include "LambdaUIWidget.h"
#include "Actors/CameraActor.h"
#include "Game/StaticMeshActor.h"
#include "Game/World.h"
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
		auto Ball = world.SpawnActor<StaticMeshActor>("Point",
			BasicShapesLibrary::GenerateSphere(0.04f));
		Ball->SetTranslation({0., 0., 1.});
		auto CowActor = world.SpawnActor<StaticMeshActor>("Cow", Cow);
		CowActor->SetTranslation({0, 0, -0.1});
		CowActor->SetRotation({M_PI_2, 0., 0.});
		world.AddWidget<LambdaUIWidget>([=]() {
			auto ScreenPos = Camera->GetCameraComponent()->Project(Ball->GetTranslation());
			auto ClipSpace = Camera->GetCameraComponent()->ProjectClipSpace(Ball->GetTranslation());
			auto UnprojectPos = Camera->GetCameraComponent()->UnProject(ClipSpace);
			ImGui::Begin("Test");
			ImGui::Text("Point Position: (%.2f, %.2f, %.2f)", Ball->GetTranslation().x(), Ball->GetTranslation().y(), Ball->GetTranslation().z());
			ImGui::Text("Screen Position: (%.2f, %.2f, %.2f)", ScreenPos.x(), ScreenPos.y(), ScreenPos.z());
			ImGui::Text("Unproject Position: (%.2f, %.2f, %.2f)", UnprojectPos.x(), UnprojectPos.y(), UnprojectPos.z());
			ImGui::End();

		});
	};
}