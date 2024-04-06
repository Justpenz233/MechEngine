//
// Created by MarvelLi on 2024/2/28.
//

#pragma once

#include "CoreMinimal.h"
#include "ImguiPlugin.h"
#include "ImguiPlus.h"
#include "LambdaUIWidget.h"
#include "Actors/CurveActor.h"
#include "Game/StaticMeshActor.h"
#include "Game/World.h"
#include "Math/Intersect.h"
#include "Mesh/BasicShapesLibrary.h"
#include "spdlog/stopwatch.h"

inline void IntersectTestMain()
{
	GWorld.InitScript = [](World& World, class ImGuiPlugin* UIManager) {
		auto Mesh1 = World.SpawnActor<StaticMeshActor>("Mesh1",BasicShapesLibrary::GenerateHemisphere(10));
		auto Mesh2 = World.SpawnActor<CurveActor>("Mesh2", Curve::Heart());


		UIManager->AddWidget<LambdaUIWidget>(
			[Mesh1, Mesh2]()
			{
				ImGui::Begin("Intersect Test");
				if(ImGui::Button("Mesh1 Intersect Mes2"))
				{
					spdlog::stopwatch sw;
					auto Mesh1Copy = NewObject<StaticMesh>(*Mesh1->GetStaticMeshComponent()->GetMeshData());
					auto Mesh2Copy = NewObject<StaticMesh>(*Mesh2->GetCurveComponent()->GetMeshData());
					Mesh1Copy->TransformMesh(Mesh1->GetTransformMatrix());
					Mesh2Copy->TransformMesh(Mesh2->GetTransformMatrix());
					bool Intersect = Math::MeshIntersectMesh(Mesh1Copy, Mesh2Copy);
					LOG_INFO("Mesh1 Intersect Mes2 {}, time {}", Intersect, sw);
				}

				if(ImGui::Button("Mesh1 Intersect Mes2 Box"))
				{
					spdlog::stopwatch sw;
					auto Mesh1Copy = NewObject<StaticMesh>(*Mesh1->GetStaticMeshComponent()->GetMeshData());
					Mesh1Copy->TransformMesh(Mesh1->GetTransformMatrix());
					auto Mesh2Box = Mesh2->GetCurveComponent()->GetMeshData()->GetBoundingBox().Translate(Mesh2->GetTranslation());;
					bool Intersect = Math::MeshIntersectBox(Mesh1Copy, Mesh2Box);
					LOG_INFO("Mesh1 Intersect Mes2 {}, time {}", Intersect, sw);
				}
				ImGui::End();
			});
	};
}