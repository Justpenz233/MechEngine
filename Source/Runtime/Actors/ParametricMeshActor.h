#pragma once
#include "Components/ParametricSurfaceComponent.h"
#include "Components/SCAFParametricMeshComponent.h"
#include "Components/SCParametricMeshComponent.h"
#include "CoreMinimal.h"
#include "Game/Actor.h"

enum ParametrizationMethod
{
	SphereicalConformal, // Require a closed mesh with genus 0, overlapping free
	SCAF // Require an open mesh (Simplicial Complex Augmentation Framework)
};

MCLASS(ParametricMeshActor)
class ParametricMeshActor : public Actor
{
    REFLECTION_BODY(ParametricMeshActor)
protected:
	ParametricMeshActor() = default;
    ObjectPtr<ParametricMeshComponent> SurfaceComponent;
public:
	double ThicknessToSample = 0.;

	/**
	 * Construct with a parametric surface give a name and assoicate parameters
	 */
    template<class ...Args>
    ParametricMeshActor(String Type, Args ...args) {
        if(Type == "Cylinder")
            SurfaceComponent = AddComponent<CylinderMeshComponent>(args...);
        if(Type == "Cone")
            SurfaceComponent = AddComponent<ConeMeshComponent>(args...);
        if(Type == "MobiusStrip")
            SurfaceComponent = AddComponent<MobiusStripMeshComponent>(args...);
        if(Type == "Catenoid")
            SurfaceComponent = AddComponent<CatenoidMeshComponent>(args...);
    }

	// Construct with a parametric surface
    ParametricMeshActor(const ObjectPtr<ParametricSurface>& SurfaceData, double Thickness = 0.05)
    {
        SurfaceComponent = AddComponent<ParametricSurfaceComponent>(SurfaceData, 0.05);
    }

	// Construct with a SARFParametricMeshComponent
	ParametricMeshActor(const ObjectPtr<StaticMesh>& InitMesh, ParametrizationMethod Method)
    {
    	if(Method == SphereicalConformal)
			SurfaceComponent = AddComponent<SCParametricMeshComponent>(InitMesh);
    	if(Method == SCAF)
    		SurfaceComponent = AddComponent<SCAFParametricMeshComponent>(InitMesh);
    }

    ~ParametricMeshActor() {};

	FORCEINLINE bool ValidUV(const FVector2& UV) const
	{
		return SurfaceComponent->ValidUV(UV[0], UV[1]);
	}

	FORCEINLINE virtual bool ValidUV(double u, double v) const
	{
		return SurfaceComponent->ValidUV(u, v);
	}

	FORCEINLINE FVector Sample(const FVector2& UV) const
    {
        return GetTransform() * SurfaceComponent->SampleThickness(UV[0], UV[1], ThicknessToSample);
    }

    FORCEINLINE FVector Sample(double u, double v) const
    {
        return GetTransform() * SurfaceComponent->SampleThickness(u, v, ThicknessToSample);
    }

    FORCEINLINE FVector SampleNormal(double u, double v) const
    {
        return SurfaceComponent->SampleNormal(u, v);
    }

    FORCEINLINE FVector SampleTangent(double u, double v) const
    {
        return SurfaceComponent->SampleTangent(u, v);
    }

    FORCEINLINE FVector SampleTangentU(double u, double v) const
    {
        return SurfaceComponent->SampleTangentU(u, v);
    }

    FORCEINLINE FVector SampleTangentV(double u, double v) const
    {
        return SurfaceComponent->SampleTangentV(u, v);
    }

    FORCEINLINE FVector SampleThickness(double u, double v)
    {
        return GetTransform() * SurfaceComponent->SampleThickness(u, v);
    }

    FORCEINLINE FVector SampleThickness(double u, double v, double ThicknessSample)
    {
        return GetTransform() * SurfaceComponent->SampleThickness(u, v, ThicknessSample);
    }

    FORCEINLINE void SetThickness(double InThickness)
    {
        SurfaceComponent->SetThickness(InThickness);
    }

	FORCEINLINE double GetThickness() const
    {
        return SurfaceComponent->GetThickness();
    }

    FORCEINLINE Vector2d Projection(const FVector& Point) const
    {
        return SurfaceComponent->Projection(GetFTransform().ToLocalSpace(Point));
    }

	FORCEINLINE FVector2 ProjectionThickness(const FVector& Point) const
	{
		return SurfaceComponent->ProjectionThickness(GetFTransform().ToLocalSpace(Point));
	}

	FORCEINLINE FVector2 ProjectionThickness(const FVector& Point, double ThicknessSample) const
	{
		return SurfaceComponent->ProjectionThickness(GetFTransform().ToLocalSpace(Point), ThicknessSample);
	}

	FORCEINLINE bool IsClosed()
    {
	    return false;
    }

    FORCEINLINE ObjectPtr<ParametricMeshComponent> GetParametricMeshComponent()
    {
	    return SurfaceComponent;
    }

	TArray<FVector> GeodicShortestPath(const FVector& Start, const FVector& End) const
	{
		auto LocalPath = SurfaceComponent->GeodicShortestPath(GetTransform().inverse() * Start, GetTransform().inverse() * End);
    	for (auto& Point : LocalPath)
		{
			Point = GetTransform() * Point;
		}
    	return LocalPath;
	}
};
