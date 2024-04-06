//
// Created by MarvelLi on 2024/2/3.
//

#pragma once

#include "Path.h"
#include "Core/CoreMinimal.h"
#include "Game/StaticMeshActor.h"
#include "Game/World.h"
#include "Mesh/BasicShapesLibrary.h"
#include "Mesh/MeshBoolean.h"

inline void MeshBooleanTestMain()
{
	GWorld.InitScript = [](World& World, class ImGuiPlugin* UIManager)
	{

		ObjectPtr<StaticMesh> Mesh1 = BasicShapesLibrary::GenerateCylinder(1., 0.5);
		ObjectPtr<StaticMesh> Mesh2 = BasicShapesLibrary::GenerateSphere(0.505);
		Mesh2->Translate({0,0,0.5});
		ObjectPtr<StaticMesh> MeshR1 = MeshBoolean::MeshUnion(Mesh1, Mesh2);
		MeshR1->SaveOBJ(Path::LogPath() / "1.obj");
		ObjectPtr<StaticMesh> MeshR2 = MeshBoolean::MeshMinus(Mesh1, Mesh2);


		auto Cylinder = World.SpawnActor<StaticMeshActor>("Mesh1");
		Cylinder->GetStaticMeshComponent()->SetMeshData(Mesh1);

		auto Sphere = World.SpawnActor<StaticMeshActor>("Mesh2");
		Sphere->GetStaticMeshComponent()->SetMeshData(Mesh2);
		Sphere->SetTranslation({0,0,0.5});

		auto Result = World.SpawnActor<StaticMeshActor>("ResultUnion");
		Result->GetStaticMeshComponent()->SetMeshData(MeshR1);
		Result->SetTranslation({0.,2.,0.});


		// auto Result2 = World.SpawnActor<StaticMeshActor>("ResultSub");
		// Result2->GetStaticMeshComponent()->SetMeshData(MeshR2);
		// Result2->SetTranslation({0.,-2.,0.});
	};
}