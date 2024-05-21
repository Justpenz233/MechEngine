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
void LinesComponent::AddLine(const FVector& WorldStart, const FVector& WorldEnd, const FColor& Color, double Thickness, double LifeTime)
{
	Thickness = Thickness < 0 ? 2 : Thickness;
	auto ID = GetWorld()->GetScene()->GetLineProxy()->AddLine(
		Rendering::ToLuisaVector(WorldStart), Rendering::ToLuisaVector(WorldEnd), Thickness, Rendering::ToLuisaVector(Color));
}