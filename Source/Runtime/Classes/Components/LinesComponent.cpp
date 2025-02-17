//
// Created by MarvelLi on 2024/5/12.
//

#include "LinesComponent.h"

#include "Game/TimerManager.h"
#include "Render/GpuSceneInterface.h"
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

void LinesComponent::AddCube(const FVector& Center, const FVector& Size, const FTransform& Transform, const FColor& Color, double Thickness, double LifeTime)
{
	auto HalfSize = Size / 2;
	FVector Min = Center - HalfSize;
	FVector Max = Center + HalfSize;
	AddLine(Transform * Min, Transform * FVector(Min.x(), Min.y(), Max.z()), Color, Thickness, LifeTime);
	AddLine(Transform * Min, Transform * FVector(Min.x(), Max.y(), Min.z()), Color, Thickness, LifeTime);
	AddLine(Transform * Min, Transform * FVector(Max.x(), Min.y(), Min.z()), Color, Thickness, LifeTime);
	
	AddLine(Transform * Max, Transform * FVector(Max.x(), Max.y(), Min.z()), Color, Thickness, LifeTime);
	AddLine(Transform * Max, Transform * FVector(Max.x(), Min.y(), Max.z()), Color, Thickness, LifeTime);
	AddLine(Transform * Max, Transform * FVector(Min.x(), Max.y(), Max.z()), Color, Thickness, LifeTime);

	AddLine(Transform * FVector(Min.x(), Max.y(), Min.z()), Transform * FVector(Min.x(), Max.y(), Max.z()), Color, Thickness, LifeTime);
	AddLine(Transform * FVector(Min.x(), Max.y(), Min.z()), Transform * FVector(Max.x(), Max.y(), Min.z()), Color, Thickness, LifeTime);
	AddLine(Transform * FVector(Max.x(), Min.y(), Min.z()), Transform * FVector(Max.x(), Min.y(), Max.z()), Color, Thickness, LifeTime);

	AddLine(Transform * FVector(Min.x(), Min.y(), Max.z()), Transform * FVector(Max.x(), Min.y(), Max.z()), Color, Thickness, LifeTime);
	AddLine(Transform * FVector(Min.x(), Min.y(), Max.z()), Transform * FVector(Min.x(), Max.y(), Max.z()), Color, Thickness, LifeTime);
	AddLine(Transform * FVector(Max.x(), Max.y(), Min.z()), Transform * FVector(Max.x(), Min.y(), Min.z()), Color, Thickness, LifeTime);

}