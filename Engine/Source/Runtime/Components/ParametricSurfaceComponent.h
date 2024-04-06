#pragma once
#include "ParametricMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Mesh/StaticMesh.h"
#include "Surface/ParametricSurface.h"

MCLASS(ParametricSurfaceComponent)
class ParametricSurfaceComponent : public ParametricMeshComponent
{
	REFLECTION_BODY(ParametricSurfaceComponent)
protected:
    ObjectPtr<ParametricSurface> SurfaceData;

	ParametricSurfaceComponent() {}
    int RullingLineNumU = 128;
    int RullingLineNumV = 32;

	MPROPERTY()
    double Thickness      = 0.05;

public:
	// Thickness will be sampled from [-HalfThickness, HalfThickness]
    ParametricSurfaceComponent(ObjectPtr<ParametricSurface> NewSurface, double InThickness = 0.05);

	template<class T, class ...Args>
	ParametricSurfaceComponent(double InThickness = 0.05, Args&& ...args) : ParametricSurfaceComponent(NewObject<T>(args...)) { Thickness = InThickness; }

    virtual void Init() override;

	virtual void PostEdit(Reflection::FieldAccessor& Field) override;
	
    static ObjectPtr<StaticMesh> TriangularSurface(int NumU, int NumV, std::function<FVector(double, double)> SampleFunc, bool NormalInside, bool ClosedSurface = false);

	FORCEINLINE bool ValidUV(double u, double v) const override
	{
		return true;
	}

    //Sample at inner surface (thickness = 0)
    virtual FVector Sample(double u, double v) const override;

    //Sample at outter surface 
    virtual FVector SampleThickness(double u, double v) const override;

    /// Override this method Is ALL YOU NEED
    /// @math x(u,v) = Directrix(u) + v * DirectorCur    ve(u),
    /// @see https://mathworld.wolfram.com/RuledSurface.html
    virtual FVector SampleThickness(double u, double v, double ThicknessSample) const override;

    /// Return normalized vertex normal
    /// @math (Sample(u + 0.01, v) - Sample(u - 0.01, v)) X (Sample(u, v + 0.01) - Sample(u, v - 0.01)) norm
    FORCEINLINE virtual FVector SampleNormal(double u, double v) const override;

    FORCEINLINE virtual FVector SampleTangent(double u, double v) const override;

	FORCEINLINE virtual FVector SampleTangentU(double u, double v) const override;
	
	FORCEINLINE virtual FVector SampleTangentV(double u, double v) const override;

	FORCEINLINE virtual FVector2 Projection(const FVector& Point) const override;

	/**
	 * Geodic Shortest Path, in practice, we assume UV shortest equivalent to 3D shortest
	 * @param Start Path Start Point
	 * @param End Path End Point
	 * @return A list of points that represent the shortest path from Start to End
	 */
	virtual TArray<FVector> GeodicShortestPath(const FVector& Start, const FVector& End) const override;

	virtual void Remesh() override;
    /// Triangular this surface 
    virtual void Triangular();

	virtual double GetThickness() const override { return Thickness; }

	virtual void SetThickness(double InThickness) override;

    FVector GetRulingLineDir() const;

	bool IsClosed() { return SurfaceData->bIsClosed; }
};

class HyperbolicCylinderMesh : public ParametricSurfaceComponent {
public:
	double Height = 1.0;
	double A = 4.0;
	double B = 2.0;

	HyperbolicCylinderMesh(double _A = 4., double _B = 2.0, double _Hegiht = 1.0) : Height(_Hegiht), A(_A), B(_B)
	{
        SurfaceData = NewObject<HyperbolicCylinderSurface>(A, B, Height);
    }
};

class CylinderMeshComponent : public ParametricSurfaceComponent
{
protected:
    double Height = 1.;
    double Radius = 1.;

public:
    CylinderMeshComponent(double InHeight = 1., double InRadius = 1.):Height(InHeight), Radius(InRadius)
    {
        SurfaceData = NewObject<CylinderSurface>(Height, Radius);
    }
};

class ConeMeshComponent : public ParametricSurfaceComponent
{
protected:
	double Height = 1;
	double Radius = 1;

public:
	ConeMeshComponent(double InHeigt = 1, double InRadius = 1) : Height(InHeigt), Radius(InRadius) { 
		SurfaceData = NewObject<ConeSurface>(Height, Radius);
	}

	double GetHegiht() { return Height; }
	double GetRadius() { return Radius; }
};

class MobiusStripMeshComponent : public ParametricSurfaceComponent
{
public:
	MobiusStripMeshComponent()
	{
		SurfaceData = NewObject<MobiusStripSurface>();
	}
};

class CatenoidMeshComponent : public ParametricSurfaceComponent
{
public:
	CatenoidMeshComponent(double InC = 1.)
	{
		SurfaceData = NewObject<CatenoidSurface>(InC);
	}
};

class PluckeConoidMeshComponent : public ParametricSurfaceComponent
{
public:

};

FVector ParametricSurfaceComponent::SampleNormal(double u, double v) const {
	return SurfaceData->SampleNormal(u, v);
}

FVector ParametricSurfaceComponent::SampleTangent(double u, double v) const {
	return SurfaceData->SampleTangent(u, v);
}

FVector ParametricSurfaceComponent::SampleTangentU(double u, double v) const
{
	return SurfaceData->SampleTangentU(u, v);
}

FVector ParametricSurfaceComponent::SampleTangentV(double u, double v) const
{
	return SurfaceData->SampleTangentV(u, v);
}

FVector2 ParametricSurfaceComponent::Projection(const FVector& Point) const
{
	return SurfaceData->Projection(Point);
}