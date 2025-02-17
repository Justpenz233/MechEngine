#include "ParametricSurfaceComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Mesh/MeshBoolean.h"
#include "Game/Actor.h"
#include "Mesh/StaticMesh.h"
#include "Object/Object.h"
#include "Surface/ParametricSurface.h"
#include "igl/per_corner_normals.h"

ParametricSurfaceComponent::ParametricSurfaceComponent(const ObjectPtr<ParametricSurface>& NewSurface, double InThickness)
	: SurfaceData(NewSurface)
{
	MeshThickness = InThickness;
}

ParametricSurfaceComponent::ParametricSurfaceComponent(const ObjectPtr<ParametricSurface>& NewSurface, const ObjectPtr<StaticMesh>& InDisplayMesh)
	: SurfaceData(NewSurface)
{
	DisplayMesh = InDisplayMesh;
	SetMeshData(DisplayMesh);
	MarkAsDirty(DIRTY_RENDERDATA);
}

void ParametricSurfaceComponent::Init()
{
	ActorComponent::Init();
	Remesh();
	MarkAsDirty(DIRTY_RENDERDATA);

	AABBMesh  = TriangularSurface(RulingLineNumU, RulingLineNumV,
	[this](double u,double v){return Sample(u, v);}, false, SurfaceData->bIsClosed);
	AABB.clear();
	AABB.init(AABBMesh->GetVertices(), AABBMesh->GetTriangles());
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
	Result->SetUV(MatrixX2d::Zero(VertexNum, 2));
    int    VertexIndex = 0;
    double StepU        = IsClosedPolygon ? 1. / (double)NumU : 1. / (double) (NumU - 1);
    double StepV        = 1. / (double)(NumV - 1);


    for (int VIndex = 0; VIndex < NumV; VIndex++) {
        for (int UIndex = 0; UIndex < NumU; UIndex++) {
            double u = (double)UIndex * StepU;
            double v = (double)VIndex * StepV;
            if(VIndex == NumV - 1) v = 1.;
        	Result->SetUV(VertexIndex, {u, v});
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
	Result->CalcNormal();
    return Result;
}


ObjectPtr<StaticMesh> ParametricSurfaceComponent::Triangular() {
    double ThicknessFix = MeshThickness < 1e-4 ? 1e-3 : MeshThickness; // When nearlly zero, set to 1e-3 as alternative of two-sided surface

    auto Inner  = TriangularSurface(RulingLineNumU, RulingLineNumV, [this, ThicknessFix](double u,double v){return SampleThickness(u, v, -ThicknessFix*0.5);}, true, SurfaceData->bIsClosed);
    auto Outter = TriangularSurface(RulingLineNumU, RulingLineNumV, [this, ThicknessFix](double u,double v){return SampleThickness(u, v, ThicknessFix*0.5);}, false, SurfaceData->bIsClosed);

    assert(Inner->GetVertexNum() == Outter->GetVertexNum());

	ObjectPtr<StaticMesh> Result;
    Result = MeshBoolean::MeshConnect(Inner, Outter);
    
    bool IsClosedPolygon = SurfaceData->bIsClosed;

    int VertexNum           = Inner->GetVertexNum();
    int TriangleIndex       = Result->GetFaceNum();
    int PreTriangleNum      = TriangleIndex;
    int SupossedTriangleNum = TriangleIndex + 4 * (RulingLineNumU - 1) + IsClosedPolygon * 4 + (!IsClosedPolygon) * 4 * (RulingLineNumV - 1);
    // Base triNum + Close along U + Close along V

    Result->triM.conservativeResize(SupossedTriangleNum, 3);

    for (int UIndex = 0; UIndex < RulingLineNumU; UIndex++)
    {
        if(!IsClosedPolygon && UIndex == RulingLineNumU - 1) continue;

        int This     = UIndex;
        int Right    = (UIndex == RulingLineNumU - 1)? 0 : This + 1;
        Result->triM.row(TriangleIndex ++) = Vector3i{This, Right, Right + VertexNum};
        Result->triM.row(TriangleIndex ++) = Vector3i{Right + VertexNum, This + VertexNum, This};

        //!!!
        This  += RulingLineNumU * (RulingLineNumV - 1);
        Right += RulingLineNumU * (RulingLineNumV - 1);
        Result->triM.row(TriangleIndex ++) = Vector3i{This, This + VertexNum, Right + VertexNum};
        Result->triM.row(TriangleIndex ++) = Vector3i{Right + VertexNum, Right, This};
    }

    if(!IsClosedPolygon)
    {
        for (int VIndex = 0; VIndex < RulingLineNumV - 1; VIndex++)
        {
            int Left    = VIndex * RulingLineNumU;
            int LeftTop = Left + RulingLineNumU;
            Result->triM.row(TriangleIndex ++) = Vector3i{Left, Left + VertexNum, LeftTop + VertexNum};
            Result->triM.row(TriangleIndex ++) = Vector3i{LeftTop + VertexNum, LeftTop, Left};

            int Right    = VIndex * RulingLineNumU + RulingLineNumU - 1;
            int RightTop = Right + RulingLineNumU;
            Result->triM.row(TriangleIndex ++) = Vector3i{Right, RightTop, RightTop + VertexNum};
            Result->triM.row(TriangleIndex ++) = Vector3i{RightTop + VertexNum, Right + VertexNum, Right};
        }
    }
	Result->OnGeometryUpdate();
    assert(SupossedTriangleNum == TriangleIndex);
	return Result;
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
	if (DisplayMesh) return;
	StaticMeshComponent::Remesh();
	auto PreMaterial = MeshData->GetMaterialAsset();
	MeshData = Triangular();
	MeshData->SetMaterial(PreMaterial);
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

FVector2 ParametricSurfaceComponent::Projection(const FVector& Point) const
{
	ASSERTMSG(AABBMesh->HasValidUV(), "AABB Mesh has no valid UV");
	RowVector3d ClosetPoint; int TriangleIndex = 0;
	AABB.squared_distance(AABBMesh->verM, AABBMesh->triM, Point, TriangleIndex, ClosetPoint);

	auto Tri = AABBMesh->GetTriangle(TriangleIndex);
	MatrixX3d Bary;
	igl::barycentric_coordinates(ClosetPoint,
		AABBMesh->GetVertex(Tri[0]).transpose(),
		AABBMesh->GetVertex(Tri[1]).transpose(),
		AABBMesh->GetVertex(Tri[2]).transpose(), Bary);
	auto UV0 = AABBMesh->GetUV(Tri[0]); auto UV1 = AABBMesh->GetUV(Tri[1]); auto UV2 = AABBMesh->GetUV(Tri[2]);
	return UV0 * Bary(0,0) + UV1 * Bary(0, 1) + UV2 * Bary(0, 2);
}

FVector2 ParametricSurfaceComponent::ProjectionThickness(const FVector& Point, double ThicknessSample) const
{
	if (ThicknessSample == 0.) return Projection(Point);
	else ASSERTMSG(false, "Not implemented");
	return {};
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