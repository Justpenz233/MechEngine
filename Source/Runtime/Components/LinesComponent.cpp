//
// Created by MarvelLi on 2024/5/12.
//

#include "LinesComponent.h"

#include "Game/TimerManager.h"
#include "Render/GPUSceneInterface.h"
#include "Render/SceneProxy/LineSceneProxy.h"
#include "Render/Core/TypeConvertion.h"


void LinesComponent::AddPoint(const FVector& WorldPosition, double Radius, const FColor& Color, double LifeTime)
{
	auto ID = GetWorld()->GetScene()->GetLineProxy()->AddPoint(
		Rendering::ToLuisaVector(WorldPosition), Radius, Rendering::ToLuisaVector(Color));

	if (LifeTime > 0)
	{
		// TODO Add delay to remove the point
		// GetWorld()->GetTimerManager()->AddTimer(LifeTime)
	}
}