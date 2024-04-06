#include "CurveActor.h"
#include "Components/StaticCurveComponent.h"
#include "Object/Object.h"


CurveActor::CurveActor(String CurveFilePath)
{
    CurveComponent = AddComponent<StaticCurveComponent>(CurveFilePath);
}

CurveActor::CurveActor(ObjectPtr<Curve> CurveData)
{
    CurveComponent = AddComponent<StaticCurveComponent>(CurveData);
}

CurveActor::CurveActor(TArray<FVector> CurveData, bool bClosed)
{
	auto CurveObject = NewObject<Curve>(CurveData, bClosed);
	CurveComponent = AddComponent<StaticCurveComponent>(CurveObject);
}

CurveActor::~CurveActor()
{

}

ObjectPtr<StaticCurveComponent> CurveActor::GetCurveComponent()
{
    return CurveComponent; 
}

FVector CurveActor::Sample(double u) const
{
    return GetTransform() * CurveComponent->Sample(u);
}

FVector CurveActor::SampleIndex(int Index) const
{
    return GetTransform() * CurveComponent->SampleIndex(Index);
}