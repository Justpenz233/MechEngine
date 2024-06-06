#include "ParametricSurfaceComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Mesh/MeshBoolean.h"
#include "Game/Actor.h"
#include "Mesh/StaticMesh.h"
#include "Object/Object.h"
#include "Surface/ParametricSurface.h"
#include "igl/per_corner_normals.h"

ParametricSurfaceComponent::ParametricSurfaceComponent(ObjectPtr<ParametricSurface> NewSurface, double InThickness)
:SurfaceData(NewSurface) {MeshThickness = InThickness;}

void ParametricSurfaceComponent::Init()
{
	ActorComponent::Init();
	Remesh();
	MarkAsDirty(DIRTY_RENDERDATA);
}

void ParametricSurfaceComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	StaticMeshComponent::PostEdit(Field);
	if (Field == NAME(MeshThickness))
	{
		Remesh();
		MarkAsDirty(DIRTY_RENDERDATA);
	}
}

ObjectPtr<StaticMesh> ParametricSurfaceComponent::TriangularSurface(int NumU, int NumV, std::function<FVector(double, double)> SampleFunc ,bool NormalInside , bool ClosedSurface)
{
    assert(NumU >= 3 && NumV >= 2);
    ObjectPtr<StaticMesh> Result = NewObject<StaticMesh>();

    bool IsClosedPolygon    = ((SampleFunc(1., 0.) - SampleFunc(0., 0.)).norm() < 0.00001) | ClosedSurface;
    // When closed, NumU gap with NumU lines, when open minus 1
    int  InnerTriangleNum   = IsClosedPolygon? NumU * (NumV - 1) * 2 : (NumU - 1) * (NumV - 1) * 2;
    int  EncloseTriangleNum = (!IsClosedPolygon) * 4;
    int  VertexNum          = NumU * NumV;

    Result->verM.resize(VertexNum, 3);
    Result->triM.resize(InnerTriangleNum, 3);

    int    VertexIndex = 0;
    double StepU        = IsClosedPolygon ? 1. / (double)NumU : 1. / (double) (NumU - 1);
    double StepV        = 1. / (double)(NumV - 1);


    for (int VIndex = 0; VIndex < NumV; VIndex++) {
        for (int UIndex = 0; UIndex < NumU; UIndex++) {
            double u = (double)UIndex * StepU;
            double v = (double)VIndex * StepV;
            if(VIndex == NumV - 1) v = 1.;
            Result->verM.row(VertexIndex++) = SampleFunc(u, v);
        }
    }
    int TriangleIndex = 0;

    for (int VIndex = 0; VIndex < NumV - 1; VIndex++) {
        for (int UIndex = 0; UIndex < NumU; UIndex++) {
            int This    = UIndex + VIndex * NumU;
            int Right   = (UIndex == NumU - 1)? This - NumU + 1 : This + 1;
            int Top     = This + NumU;
            int TopLeft = (UIndex == 0)? This + NumU + NumU - 1 : This + NumU - 1;

            assert(This < VertexNum && Right < VertexNum && Top < VertexNum && TopLeft < VertexNum);
            assert(This >= 0 && Right >= 0 && Top >= 0 && TopLeft >= 0);

            if (IsClosedPolygon || UIndex != 0)
                if(NormalInside)
                    Result->triM.row(TriangleIndex++) = Vector3i{This, TopLeft,Top};
                else
                    Result->triM.row(TriangleIndex++) = Vector3i{Top, TopLeft,This};
            if (IsClosedPolygon || UIndex != NumU - 1)
                if(NormalInside)
                    Result->triM.row(TriangleIndex++) = Vector3i{Top, Right, This};
                else
                    Result->triM.row(TriangleIndex++) = Vector3i{This, Right, Top};

        }
    }

    assert(InnerTriangleNum == TriangleIndex);
    return Result;
}


void ParametricSurfaceComponent::Triangular() {
    double ThicknessFix = MeshThickness < 1e-4 ? 1e-3 : MeshThickness; // When nearlly zero, set to 1e-3 as alternative of two sided surface

    auto Inner  = TriangularSurface(RullingLineNumU, RullingLineNumV, [this, ThicknessFix](double u,double v){return SampleThickness(u, v, -ThicknessFix*0.5);}, true, SurfaceData->bIsClosed);
    auto Outter = TriangularSurface(RullingLineNumU, RullingLineNumV, [this, ThicknessFix](double u,double v){return SampleThickness(u, v, ThicknessFix*0.5);}, false, SurfaceData->bIsClosed);

    assert(Inner->GetVertexNum() == Outter->GetVertexNum());

    
    MeshData = MeshBoolean::MeshConnect(Inner, Outter);
    
    bool IsClosedPolygon = SurfaceData->bIsClosed;

    int VertexNum           = Inner->GetVertexNum();
    int TriangleIndex       = MeshData->GetFaceNum();
    int PreTriangleNum      = TriangleIndex;
    int SupossedTriangleNum = TriangleIndex + 4 * (RullingLineNumU - 1) + IsClosedPolygon * 4 + (!IsClosedPolygon) * 4 * (RullingLineNumV - 1);
    // Base triNum + Close along U + Close along V

    MeshData->triM.conservativeResize(SupossedTriangleNum, 3);

    for (int UIndex = 0; UIndex < RullingLineNumU; UIndex++)
    {
        if(!IsClosedPolygon && UIndex == RullingLineNumU - 1) continue;

        int This     = UIndex;
        int Right    = (UIndex == RullingLineNumU - 1)? 0 : This + 1;
        MeshData->triM.row(TriangleIndex ++) = Vector3i{This, Right, Right + VertexNum};
        MeshData->triM.row(TriangleIndex ++) = Vector3i{Right + VertexNum, This + VertexNum, This};

        //!!!
        This  += RullingLineNumU * (RullingLineNumV - 1);
        Right += RullingLineNumU * (RullingLineNumV - 1);
        MeshData->triM.row(TriangleIndex ++) = Vector3i{This, This + VertexNum, Right + VertexNum};
        MeshData->triM.row(TriangleIndex ++) = Vector3i{Right + VertexNum, Right, This};
    }

    if(!IsClosedPolygon)
    {
        for (int VIndex = 0; VIndex < RullingLineNumV - 1; VIndex++)
        {
            int Left    = VIndex * RullingLineNumU;
            int LeftTop = Left + RullingLineNumU;
            MeshData->triM.row(TriangleIndex ++) = Vector3i{Left, Left + VertexNum, LeftTop + VertexNum};
            MeshData->triM.row(TriangleIndex ++) = Vector3i{LeftTop + VertexNum, LeftTop, Left};

            int Right    = VIndex * RullingLineNumU + RullingLineNumU - 1;
            int RightTop = Right + RullingLineNumU;
            MeshData->triM.row(TriangleIndex ++) = Vector3i{Right, RightTop, RightTop + VertexNum};
            MeshData->triM.row(TriangleIndex ++) = Vector3i{RightTop + VertexNum, Right + VertexNum, Right};
        }
    }
	MeshData->OnGeometryUpdate();
    assert(SupossedTriangleNum == TriangleIndex);
}

FVector ParametricSurfaceComponent::Sample(const double u, const double v) const {
    return SurfaceData->Sample(u, v);
}

FVector ParametricSurfaceComponent::SampleThickness(double u, double v) const {
    return SurfaceData->SampleThickness(u, v, MeshThickness * 0.5);
}

FVector ParametricSurfaceComponent::SampleThickness(const double u, const double v, const double ThicknessSample) const
{
    return SurfaceData->SampleThickness(u, v, ThicknessSample);
}

void ParametricSurfaceComponent::Remesh()
{
	StaticMeshComponent::Remesh();
	Triangular();
}

void ParametricSurfaceComponent::SetThickness(double InThickness)
{
    MeshThickness = InThickness;
	Remesh();
	MarkAsDirty(DIRTY_RENDERDATA);
}

FVector ParametricSurfaceComponent::GetRulingLineDir() const
{
    return SampleThickness(0., 1., 0.) - SampleThickness(0., 0., 0.);
}

TArray<FVector> ParametricSurfaceComponent::UVPath(const FVector& Start, const FVector& End) const
{
	FVector2 EndUV = SurfaceData->Projection(End);
	FVector2 StartUV = SurfaceData->Projection(Start);
	double EU = EndUV.x();
	double EV = EndUV.y();
	double SU = StartUV.x();
	double SV = StartUV.y();
	TArray<FVector> Path;
	if(EU - SU > EV - SV) {
		if(SU < EU)
		{
			for(double U = SU; U < EU; U += 0.01) {
				double V = SV + (EV - SV) * (U - SU) / (EU - SU);
				Path.emplace_back(Sample(U, V));
			}
		}
		else
		{
			for(double U = SU; U >= EU; U -= 0.01) {
				double V = SV + (EV - SV) * (U - SU) / (EU - SU);
				Path.emplace_back(Sample(U, V));
			}
		}
		Path.emplace_back(Sample(EU, EV));
	}
	else {
		if(SV < EV)
		{
			for(double V = SV; V < EV; V += 0.01) {
				double U = SU + (EU - SU) * (V - SV) / (EV - SV);
				Path.emplace_back(Sample(U, V));
			}
		}
		else
		{
			for(double V = SV; V >= EV; V -= 0.01) {
				double U = SU + (EU - SU) * (V - SV) / (EV - SV);
				Path.emplace_back(Sample(U, V));
			}
		}
		Path.emplace_back(Sample(EU, EV));
	}
	return Path;
}