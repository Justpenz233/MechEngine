//
// Created by MarvelLi on 2024/1/19.
//

#pragma once
#include "CoreMinimal.h"


namespace MechEngine::Algorithm
{

	//WARNING: Not test yet

	// Discrete Frechet Distance
	// Math : https://en.wikipedia.org/wiki/Fr%C3%A9chet_distance
	double DiscreteFrechetDistance(const TArray<FVector> &CurveA, const TArray<FVector> &CurveB);

	// @see https://en.wikipedia.org/wiki/Dynamic_time_warping
	double DTWDistance(const TArray<FVector>& CurveA, const TArray<FVector>& CurveB);

}