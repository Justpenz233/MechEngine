#pragma once
#include "Misc/Path.h"
#include "Core/CoreMinimal.h"
#include "Game/StaticMeshActor.h"
#include "Game/World.h"
#include "Mesh/BasicShapesLibrary.h"

inline auto BasicShapeTest()
{
    return [](World& World)
    {
        auto Cylinder = World.SpawnActor<StaticMeshActor>("Cylinder"); Cylinder->SetTranslation({-1,0,0});
        Cylinder->GetStaticMeshComponent()->SetMeshData(BasicShapesLibrary::GenerateCylinder(1., 0.5));

        auto Sphere = World.SpawnActor<StaticMeshActor>("Sphere"); Sphere->SetTranslation({1,0,0});
        Sphere->GetStaticMeshComponent()->SetMeshData(BasicShapesLibrary::GenerateSphere(0.5));

        auto HollowCylinder = World.SpawnActor<StaticMeshActor>("HollowCylinder"); HollowCylinder->SetTranslation({2,0,0});
        HollowCylinder->GetStaticMeshComponent()->SetMeshData(BasicShapesLibrary::GenerateHollowCylinder(0.5, 0.25, 0.75));
    
        auto Cube = World.SpawnActor<StaticMeshActor>("Cube"); Cube->SetTranslation({-2,0,0});
        Cube->GetStaticMeshComponent()->SetMeshData(BasicShapesLibrary::GenerateCuboid({0.75, 0.5, 0.25}));

    	auto Capsule = World.SpawnActor<StaticMeshActor>("Capsule"); Capsule->SetTranslation({0,0,1});
		Capsule->GetStaticMeshComponent()->SetMeshData(BasicShapesLibrary::GenerateCapsule(0.5, 1.));

    	auto Hemisphere = World.SpawnActor<StaticMeshActor>("Hemisphere"); Hemisphere->SetTranslation({1,0,1});
    	Hemisphere->GetStaticMeshComponent()->SetMeshData(BasicShapesLibrary::GenerateHemisphere(0.5, false));

    	auto OneHatCapsule = World.SpawnActor<StaticMeshActor>("OneHatCapsule"); OneHatCapsule->SetTranslation({2,0,1});
    	OneHatCapsule->GetStaticMeshComponent()->SetMeshData(BasicShapesLibrary::GenerateOneHatCapsule(0.5, 1., false));
    };
}