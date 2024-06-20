#pragma once
#include "ParametricMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Mesh/StaticMesh.h"
#include "Surface/ParametricSurface.h"

MCLASS(ParametricSurfaceComponent)
class ENGINE_API ParametricSurfaceComponent : public ParametricMeshComponent
{
	REFLECTION_BODY(ParametricSurfaceComponent)
protected:
	MPROPERTY()
    ObjectPtr<ParametricSurface> SurfaceData;

	ParametricSurfaceComponent() = default;

	MPROPERTY()
	int RullingLineNumU = 128;

	MPROPERTY()
	int RullingLineNumV = 32;

public:
	// Thickness will be sampled from [-HalfThickness, HalfThickness]
    explicit ParametricSurfaceComponent(ObjectPtr<ParametricSurface> NewSurface, double InThickness = 0.05);

	template<class T, class ...Args>
	explicit ParametricSurfaceComponent(double InThickness = 0.05, Args&& ...args) : ParametricSurfaceComponent(NewObject<T>(args...)) { MeshThickness = InThickness; }

    virtual void Init() override;

	virtual void PostEdit(Reflection::FieldAccessor& Field) override;
	
    static ObjectPtr<StaticMesh> TriangularSurface(int NumU, int NumV, std::function<FVector(double, double)> SampleFunc, bool NormalInside, bool ClosedSurface = false);

	FORCEINLINE bool ValidUV(double u, double v) const override { return true; }

    //Sample at inner surface (thickness = 0)
    virtual FVector Sample(double u, double v) const override;

    //Sample at outter surface (Sample thickness at Thickness*0.5f)
    virtual FVector SampleThickness(double u, double v) const override;

    /// Override this method Is ALL YOU NEED for building a mesh of a parametric surface
    /// @math x(u,v) = Directrix(u) + v * DirectorCurve(u),
    /// @see https://mathworld.wolfram.com/RuledSurface.html
    virtual FVector SampleThickness(double u, double v, double ThicknessSample) const override;

    /// Return normalized vertex normal
    /// @math (Sample(u + 0.01, v) - Sample(u - 0.01, v)) X (Sample(u, v + 0.01) - Sample(u, v - 0.01)) norm
    FORCEINLINE virtual FVector SampleNormal(double u, double v) const override;

    FORCEINLINE virtual FVector SampleTangent(double u, double v) const override;

	FORCEINLINE virtual FVector SampleTangentU(double u, double v) const override;
	
	FORCEINLINE virtual FVector SampleTangentV(double u, double v) const override;

	/***
	 * Project a 3D point to 2D UV coordinate, at thickness = 0 space.
	 * @param Point 3D point
	 * @return 2D UV coordinate
	 */
	FORCEINLINE virtual FVector2 Projection(const FVector& Point) const override;

	/***
	 * Project a 3D point to 2D UV coordinate, at thickness = ThicknessSample space.
	 * @param Point 3D point
	 * @param ThicknessSample Thickness sample
	 * @return 2D UV coordinate
	 */
	FORCEINLINE virtual FVector2 ProjectionThickness(const FVector& Point, double ThicknessSample) const override;
	FORCEINLINE virtual FVector2 ProjectionThickness(const FVector& Point) const override; // Projection at outter surface


	/**
	 * A UV Path, direct path from Start to End
	 * @param Start Path Start Point
	 * @param End Path End Point
	 * @return A list of points that represent the shortest path from Start to End
	 */
	TArray<FVector> UVPath(const FVector& Start, const FVector& End) const;

	virtual void Remesh() override;

    /// Triangular this surface 
    virtual void Triangular();

	virtual double GetThickness() const override { return MeshThickness; }

	virtual void SetThickness(double InThickness) override;

    FVector GetRulingLineDir() const;

	bool IsClosed() { return SurfaceData->bIsClosed; }
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

FVector2 ParametricSurfaceComponent::ProjectionThickness(const FVector& Point, double ThicknessSample) const
{
	return SurfaceData->ProjectionThickness(Point, ThicknessSample);
}

FVector2 ParametricSurfaceComponent::ProjectionThickness(const FVector& Point) const
{
	return SurfaceData->ProjectionThickness(Point, MeshThickness * 0.5);
}