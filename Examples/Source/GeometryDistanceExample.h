//
// Created by MarvelLi on 2024/4/22.
//

#pragma once
#include "Actors/CurveActor.h"
#include "Game/StaticMeshActor.h"
#include "Game/World.h"
#include "Math/Geometry.h"
#include "Math/Random.h"
#include "Mesh/BasicShapesLibrary.h"

inline auto GeometryDistanceExample()
{
	return [](World& world){

		// Point - Segment
		FVector PointA = Random::RandomFVector();

		auto SegmentA = TArray<FVector>{Random::RandomFVector() * 3, Random::RandomFVector() * 3};


		world.SpawnActor<StaticMeshActor>("Point", BasicShapesLibrary::GenerateSphere(0.1))->SetTranslation(PointA);
		world.SpawnActor<CurveActor>("SegmentA", SegmentA);

		auto Result = Math::PointSegmentDistance(PointA, SegmentA[0], SegmentA[1]);
		world.SpawnActor<CurveActor>("Distance", TArray<FVector>{PointA, std::get<1>(Result)})->GetCurveComponent()->SetRadius(0.01);



		// Segment - Segment
		auto SegmentB = TArray<FVector>{Random::RandomFVector() * 3, Random::RandomFVector() * 3};

		world.SpawnActor<CurveActor>("SegmentB", SegmentB);

		auto Result1 = Math::SegmentSegmentDistance(SegmentA[0], SegmentA[1], SegmentB[0], SegmentB[1]);
		world.SpawnActor<CurveActor>("Distance", TArray<FVector>{std::get<1>(Result1), std::get<2>(Result1)})->GetCurveComponent()->SetRadius(0.01);

	};
}