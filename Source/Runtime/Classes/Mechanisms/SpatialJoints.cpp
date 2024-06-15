#include "Animation/IKJoint.h"
#include "Core/CoreMinimal.h"
#include "Math/Math.h"
#include "Log/Log.h"
#include "Mesh/MeshBoolean.h"
#include "SpatialJoints.h"
#include <numeric>
#include "Math/Intersect.h"
#include "Math/LinearAlgebra.h"
#include "Mesh/BasicShapesLibrary.h"

SpatialJoint::SpatialJoint(char InType, const FTransform& InInitTransform, bool InIsRoot)
: Type(InType), IKJoint(EDOF3D::FreeNone, EDOF3D::FreeNone, InInitTransform, InIsRoot)
{
    if (Type == 'R')
        SetDOF(FreeZ, FreeNone);
    else if (Type == 'P'){
        SetDOF(FreeNone, FreeX);
        AddTranslationRange[0].x() = -0.2;
        AddTranslationRange[1].x() = 0.2;
    }
    else if (Type == 'C')
    {
        SetDOF(FreeX, FreeX);
        AddTranslationRange[0].x() = -0.2;
        AddTranslationRange[1].x() = 0.2;
    }
    else if (Type == 'U'){
        SetDOF(FreeXZ, FreeNone);
        AddRotationRange[0].z() = Math::DegToRad(-45.);
        AddRotationRange[1].z() = Math::DegToRad(45.);
    }
    else if (Type == 'S'){
        SetDOF(FreeXYZ, FreeNone);
        AddRotationRange[0].y() = Math::DegToRad(-45.);
        AddRotationRange[1].y() = Math::DegToRad(45.);
        AddRotationRange[0].z() = Math::DegToRad(-45.);
        AddRotationRange[1].z() = Math::DegToRad(45.);
    }
    else if (Type == 'E')
        SetDOF(FreeNone, FreeNone);
    else
        ASSERTMSG(false, "Type {0} is not a valid Spatial joint", Type);
}

bool SpatialJointComponent::IsRoot() const
{
    return JointPtr->IsRootJoint();
}

TArray<FVector> SpatialJointComponent::GetJointPortLocation()
{
	TArray<FVector> Result = GetJointPortInMeshLocation();
	for(FVector& InMesh : Result)
	{
		InMesh = InMesh + InMesh.normalized() * PortLength;
	}
	return Result;
}

TArray<FVector> SpatialJointComponent::GetSocketPortLocation()
{
	TArray<FVector> Result = GetSocketPortInMeshLocation();
	for(FVector& InMesh : Result)
	{
		InMesh = InMesh + InMesh.normalized() * PortLength;
	}
	return Result;
}


ObjectPtr<StaticMesh> SpatialJointComponent::GenerateJointMeshWithPort(int PortIndex)
{
    ASSERTMSG(PortIndex < GetJointPortInMeshLocation().size() && PortIndex >= 0, "PortIndex out of range");
    auto JointMesh = GenerateJointMesh();
    FVector PortLocation = GetJointPortInMeshLocation()[PortIndex];
    FVector PortEnd = GetJointPortLocation()[PortIndex];
    auto PortMesh = BasicShapesLibrary::GenerateCylinder(PortLocation, PortEnd, LinkageRadius);
    auto Sphere = BasicShapesLibrary::GenerateSphere(LinkageRadius);
    Sphere->Translate(PortEnd);
    PortMesh = MeshBoolean::MeshUnion(PortMesh, Sphere);
    return MeshBoolean::MeshUnion(JointMesh, PortMesh);
}

ObjectPtr<StaticMesh> SpatialJointComponent::GenerateJointMeshWithPort(TArray<int> PortIndexs)
{
    auto PortLocations = GetJointPortInMeshLocation();
    auto JointMesh = GenerateJointMesh();

    for (int PortIndex : PortIndexs)
    {
        ASSERTMSG(PortIndex < PortLocations.size() && PortIndex >= 0, "PortIndex out of range");
        FVector PortLocation = GetJointPortInMeshLocation()[PortIndex];
        FVector PortEnd = GetJointPortLocation()[PortIndex];

        auto PortMesh = BasicShapesLibrary::GenerateCylinder(PortLocation, PortEnd, LinkageRadius);
        auto Sphere = BasicShapesLibrary::GenerateSphere(LinkageRadius);
        Sphere->Translate(PortEnd);
        PortMesh = MeshBoolean::MeshUnion(PortMesh, Sphere);
        JointMesh = MeshBoolean::MeshUnion(JointMesh, PortMesh);
    }
    return JointMesh;
}

ObjectPtr<StaticMesh> SpatialJointComponent::GenerateSocketMeshWithPort(int PortIndex)
{
    ASSERTMSG(PortIndex < GetSocketPortInMeshLocation().size() && PortIndex >= 0, "PortIndex out of range");
    FVector PortLocation = GetSocketPortInMeshLocation()[PortIndex];
    FVector PortEnd = GetSocketPortLocation()[PortIndex];
    auto SocketMesh = GenerateSocketMesh();
    auto PortMesh = BasicShapesLibrary::GenerateCylinder(PortLocation, PortEnd, LinkageRadius);
    auto Sphere = BasicShapesLibrary::GenerateSphere(LinkageRadius);
    Sphere->Translate(PortEnd);
    PortMesh = MeshBoolean::MeshUnion(PortMesh, Sphere);
    return MeshBoolean::MeshUnion(SocketMesh, PortMesh);
}

ObjectPtr<StaticMesh> SpatialJointComponent::GenerateSocketMeshWithPort(TArray<int> PortIndexs)
{
    auto PortLocations = GetSocketPortInMeshLocation();
    auto SocketMesh = GenerateSocketMesh();

    for (int PortIndex : PortIndexs)
    {
        ASSERTMSG(PortIndex < PortLocations.size() && PortIndex >= 0, "PortIndex out of range");
        FVector PortLocation = PortLocations[PortIndex];
        FVector PortEnd = GetSocketPortLocation()[PortIndex];

        auto PortMesh = BasicShapesLibrary::GenerateCylinder(PortLocation, PortEnd, LinkageRadius);
        auto Sphere = BasicShapesLibrary::GenerateSphere(LinkageRadius);
        Sphere->Translate(PortEnd);
        PortMesh = MeshBoolean::MeshUnion(PortMesh, Sphere);
        SocketMesh = MeshBoolean::MeshUnion(SocketMesh, PortMesh);
    }
    return SocketMesh;
}

ObjectPtr<StaticMesh> SpatialJointComponent::GenerateLinkageTo(FVector PortLocation, FVector TargetLocation, bool bIsEffector)
{
    TargetLocation = GetOwner()->GetFTransform().ToLocalSpace(TargetLocation);
    auto LinkageMesh =  BasicShapesLibrary::GenerateCylinder(PortLocation, TargetLocation, LinkageRadius);
    auto LeftSphere = BasicShapesLibrary::GenerateSphere(LinkageRadius);
    LeftSphere->Translate(PortLocation);
    auto RightSphere = BasicShapesLibrary::GenerateSphere(LinkageRadius);
    RightSphere->Translate(TargetLocation);
    LinkageMesh = MeshBoolean::MeshUnion(LinkageMesh, LeftSphere);
    LinkageMesh = MeshBoolean::MeshUnion(LinkageMesh, RightSphere);
    return LinkageMesh;
}

double SpatialJointComponent::FindBestPortPair(ObjectPtr<SpatialJointComponent> NextJoint, int& BestJointPort, int& BestSocketPort)
{
    auto ThisJointPort = GetJointPortInMeshLocation();
	auto NextSocketPort = NextJoint->GetSocketPortInMeshLocation();
    FVector ThisJointPos = GetOwner()->GetTranslation();
    FVector NextJointPos = NextJoint->GetOwner()->GetTranslation();
	double BestLength = 100000.;
	// find a best solution
	for (int i = 0; i < ThisJointPort.size(); i++)
	{
		for (int j = 0; j < NextSocketPort.size(); j++)
		{
			FVector Delta = GetJointPortWorldLocation(i)
				- NextJoint->GetSocketPortWorldLocation(j);
			if (BestLength > Delta.norm())
			{
				BestLength = Delta.norm();
				BestJointPort = i;
				BestSocketPort = j;
			}
		}
	}
    if(NextJoint->IsRoot()) {
        FVector NextJointZ = NextJoint->GetOwner()->GetRotation() * FVector{0, 0, 1};
        for(int i = 0;i < NextSocketPort.size(); i ++) {
            FVector NextSocketPos = NextJoint->GetSocketPortWorldLocation(i);
            double t = MechEngine::Math::SegmentIntersectPlane(ThisJointPos, NextSocketPos, NextJointPos, NextJointZ);
            if(t > 1. || t < 0.) {
                BestSocketPort = i;
                break;
            }
        }
    }
    return BestLength;
}

ObjectPtr<StaticMesh> SpatialJointComponent::CreateJointMesh()
{
    if (!NextJointComponents.empty())
    {
	     TArray<int> BestPortIndexs;
	     // Gather all possible Joint ports
	     for( auto ChildJoint : NextJointComponents)
	     {
	         if(auto NextSpacialJoint = ChildJoint.lock())
	         {
	             int JointPortIndex; int SocketPortIndex;
	             FindBestPortPair(NextSpacialJoint, JointPortIndex, SocketPortIndex);
	             BestPortIndexs.push_back(JointPortIndex);
	         }
	     }
	     auto ThisMesh = GenerateJointMeshWithPort(BestPortIndexs);
	     for( auto ChildJoint : NextJointComponents)
	     {
	         if(auto NextSpacialJoint = ChildJoint.lock())
	         {
	             int JointPortIndex; int SocketPortIndex;
	             FindBestPortPair(NextSpacialJoint, JointPortIndex, SocketPortIndex);
	         	 bool bIsEffector = NextSpacialJoint->GetJoint()->NextJoints.empty();

	             auto LinkageToNext = GenerateLinkageTo(GetJointPortLocation()[JointPortIndex],
	             NextSpacialJoint->GetSocketPortWorldLocation(SocketPortIndex), bIsEffector);

	             // ADD Next Socket
	             auto NextSocketMesh = NextSpacialJoint->GenerateSocketMeshWithPort(SocketPortIndex);
	             Matrix4d ToLocal = (GetOwner()->GetTransform().inverse() * NextSpacialJoint->GetOwner()->GetTransform()).matrix();
	             NextSocketMesh->TransformMesh(ToLocal);

	             ThisMesh = MeshBoolean::MeshUnion(ThisMesh, LinkageToNext);
	             ThisMesh = MeshBoolean::MeshUnion(ThisMesh, NextSocketMesh);
	         }
	     }
	     return ThisMesh;
	 }

	if(JointPtr->IsRootJoint() && JointPtr->NextJoints.size() == 0) // Single Joint, Show all port, for debug
    {
         TArray<int> SocketIndexes(GetSocketPortInMeshLocation().size());
         std::iota(SocketIndexes.begin(), SocketIndexes.end(), 0);
         auto SocketMesh = GenerateSocketMeshWithPort(SocketIndexes);

         TArray<int> JointIndexs(GetJointPortInMeshLocation().size());
         std::iota(JointIndexs.begin(), JointIndexs.end(), 0);
         auto JointMesh = GenerateJointMeshWithPort(JointIndexs);

         return MeshBoolean::MeshUnion(SocketMesh, JointMesh);
    }
	return nullptr;
}

ObjectPtr<StaticMesh> SpatialJointR::GenerateSocketMesh(double OuterRadius, double InnerRadius, double SocketHeight, double JointHeight)
{
    // (0,0,0.5Thick) -> (0,0,-0.5Thick)
    auto ResultMesh = BasicShapesLibrary::GenerateCylinder(SocketHeight, OuterRadius);
    auto JointMesh = GenerateJointMesh(OuterRadius * 2, InnerRadius * 0.95, JointHeight * 1.05);
    return MeshBoolean::MeshMinus(ResultMesh, JointMesh);
}

ObjectPtr<StaticMesh> SpatialJointR::GenerateJointMesh(double OuterRadius, double InnerRadius, double Height)
{
    return BasicShapesLibrary::GenerateHollowCylinder(OuterRadius, InnerRadius, Height);
}

TArray<FVector> SpatialJointR::GetJointPortInMeshLocation()
{
    return {FVector(OuterRadius, 0, 0)};
}

TArray<FVector> SpatialJointR::GetSocketPortInMeshLocation()
{
    return {
        FVector(0, 0, SocketHeight * 0.5 * 0.95),
        FVector(0, 0, -SocketHeight * 0.5 * 0.95),
    };
}

ObjectPtr<StaticMesh> SpatialJointP::GenerateSocketMesh(double OuterSideLength, double InnerSideLength, double Length)
{
    auto Outer = BasicShapesLibrary::GenerateCuboid({Length, OuterSideLength, OuterSideLength});
    auto Inner = BasicShapesLibrary::GenerateCuboid({Length*2, InnerSideLength, InnerSideLength});
    Outer = MeshBoolean::MeshMinus(Outer, Inner);
    return Outer;
}

ObjectPtr<StaticMesh> SpatialJointP::GenerateJointMesh(double SideLength, double JointLeft, double JointRight)
{
    auto Cube =  BasicShapesLibrary::GenerateCuboid({JointRight + JointLeft, SideLength, SideLength});
    double DeltaX = (JointLeft + JointRight) * 0.5 - JointLeft;
    Cube->Translate({DeltaX, 0, 0});
    return Cube;
}

TArray<FVector> SpatialJointP::GetJointPortInMeshLocation()
{
    // Left and right
    return {
        FVector(-JointLeft, 0., 0.),
         FVector(JointRight, 0., 0.)
    };
}

TArray<FVector> SpatialJointP::GetSocketPortInMeshLocation()
{
    double HalfSideL = OuterSideLength * 0.5;
    return {
        FVector(0, 0, HalfSideL), // Z Up
        FVector(0, 0, -HalfSideL), // Z Down
        FVector(0, HalfSideL, 0), // Y Out
        FVector(0, -HalfSideL, 0), // Y In
    };
}

ObjectPtr<StaticMesh> SpatialJointC::GenerateSocketMesh(double OuterRadius, double InnerRadius, double Length)
{
    auto HollowCylinder = BasicShapesLibrary::GenerateHollowCylinder(OuterRadius, InnerRadius, Length);
    HollowCylinder->TransformMesh(MMath::RotationMatrix4FromEulerXYZ({0, M_PI * 0.5, 0}));
    return HollowCylinder;
}

ObjectPtr<StaticMesh> SpatialJointC::GenerateJointMesh(double Radius, double JointLeft, double JointRight)
{
    auto Result = BasicShapesLibrary::GenerateCylinder({-JointLeft, 0, 0}, {JointRight, 0, 0}, Radius, 100);
    return Result;
}


TArray<FVector> SpatialJointC::GetJointPortInMeshLocation()
{
    // Left and right
    return {
        FVector(-JointLeft, 0., 0.),
        FVector(JointRight, 0., 0.)
    }; 
}

TArray<FVector> SpatialJointC::GetSocketPortInMeshLocation()
{
    return {
        FVector(0, 0, OuterRadius), // Z Up
        FVector(0, 0, -OuterRadius), // Z Down
        FVector(0, OuterRadius, 0), // Y Out
        FVector(0, -OuterRadius, 0), // Y In
    };
}

ObjectPtr<StaticMesh> SpatialJointU::GenerateSocketMesh(double Diameter, double JointRadius, double Thickness, double CylinderRadius)
{
    auto Cube = BasicShapesLibrary::GenerateCuboid({Thickness, Diameter, Diameter});
    auto JointBall = BasicShapesLibrary::GenerateSphere(JointRadius);
    Cube = MeshBoolean::MeshMinus(Cube, JointBall);

    
    auto SupporterCylinder = BasicShapesLibrary::GenerateCylinder(Diameter, CylinderRadius);
    SupporterCylinder->RotateEuler({-0.5*M_PI, 0, 0});
    Cube = MeshBoolean::MeshUnion(Cube, SupporterCylinder);

    auto CylinderCut = BasicShapesLibrary::GenerateCylinder(JointRadius, CylinderRadius * 1.2);
    CylinderCut->RotateEuler({-0.5*M_PI, 0, 0});
    Cube = MeshBoolean::MeshMinus(Cube, CylinderCut);
    return Cube;
}

ObjectPtr<StaticMesh> SpatialJointU::GenerateJointMesh(double Radius)
{
    auto SphereMesh = BasicShapesLibrary::GenerateSphere(Radius);
    auto CutCylinder = BasicShapesLibrary::GenerateHollowCylinder(2*Radius, 0.5*Radius, 0.5*Radius);
    CutCylinder->RotateEuler({0, -0.5*M_PI, 0});
    SphereMesh = MeshBoolean::MeshMinus(SphereMesh, CutCylinder);
    return SphereMesh;
}

TArray<FVector> SpatialJointU::GetJointPortInMeshLocation()
{
    return { FVector(JointRadius, 0, 0) };
}

TArray<FVector> SpatialJointU::GetSocketPortInMeshLocation()
{
    double HalfDiameter = Diameter * 0.5;
    return {
        FVector(0, 0, HalfDiameter), // Z Up
        FVector(0, 0, -HalfDiameter), // Z Down
        FVector(0, HalfDiameter, 0), // Y Out
        FVector(0, -HalfDiameter, 0), // Y In
    };
}


ObjectPtr<StaticMesh> SpatialJointS::GenerateSocketMesh(double OuterRadius, double InnerRadius)
{
    auto OuterSphere = BasicShapesLibrary::GenerateSphere(OuterRadius);
    auto InnerSphere = BasicShapesLibrary::GenerateSphere(InnerRadius);
    auto CutCube = BasicShapesLibrary::GenerateCuboid({OuterRadius*2, OuterRadius*2, OuterRadius*2});
    CutCube->Translate({1.5 * OuterRadius, 0, 0});
    OuterSphere = MeshBoolean::MeshMinus(OuterSphere, CutCube);
    OuterSphere = MeshBoolean::MeshMinus(OuterSphere, InnerSphere);
    return OuterSphere;
}

ObjectPtr<StaticMesh> SpatialJointS::GenerateJointMesh(double Radius)
{
    return BasicShapesLibrary::GenerateSphere(Radius);
}

TArray<FVector> SpatialJointS::GetJointPortInMeshLocation()
{
    return {FVector(InnerRadius, 0, 0)};
}

TArray<FVector> SpatialJointS::GetSocketPortInMeshLocation()
{
    return {
        FVector(-OuterRadius, 0, 0), // Z Up
        FVector(0, 0, OuterRadius), // Z Up
        FVector(0, 0, -OuterRadius), // Z Down
        FVector(0, OuterRadius, 0), // Y Out
        FVector(0, -OuterRadius, 0), // Y In
    };
}