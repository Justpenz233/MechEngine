#pragma once
#include "Animation/IKJoint.h"
#include "Animation/Joints.h"
#include "Core/CoreMinimal.h"
#include "Math/FTransform.h"
#include "Game/Actor.h"
#include "Mesh/BasicShapesLibrary.h"
#include "Mesh/StaticMesh.h"
#include "Animation/JointComponent.h"

class SpatialJoint : public IKJoint
{
public:
    char Type;
    SpatialJoint(char InType, const FTransform& InInitTransform = FTransform::Identity(), bool InIsRoot = false);
};
/**
 * @brief This an abstract class, shoule be derived and complete generate mesh.
 */
class SpatialJointComponent : public IKJointComponent
{
private:
	TArray<WeakObjectPtr<SpatialJointComponent>> NextJointComponents;
public:
    // Stretch out linkage to connect others
    double PortLength = 0.1;
    double LinkageRadius = 0.01;
    double FabricateEngery = 0.01;

    SpatialJointComponent(EDOF3D InRoationDOF, EDOF3D InTranslationDOF, const FTransform& InInitTransform, bool InIsRoot = false)
	: IKJointComponent( InRoationDOF,  InTranslationDOF, InInitTransform, InIsRoot) { }

    bool IsRoot() const;

    virtual ObjectPtr<StaticMesh> GenerateJointMesh() = 0;
    virtual ObjectPtr<StaticMesh> GenerateSocketMesh() = 0;

    // Return the sockets where this joint mesh can connect to next mesh, in world space
    virtual TArray<FVector> GetJointPortInMeshLocation() { return {FVector::Zero()}; };
    virtual TArray<FVector> GetSocketPortInMeshLocation() { return {FVector::Zero()}; };

	virtual TArray<FVector> GetJointPortLocation();
	virtual TArray<FVector> GetSocketPortLocation();

    FVector GetJointPortWorldLocation(int Index) { return GetOwner()->GetTransform() * GetJointPortLocation()[Index]; }
    FVector GetSocketPortWorldLocation(int Index) { return GetOwner()->GetTransform() * GetSocketPortLocation()[Index]; }

    double FindBestPortPair(ObjectPtr<SpatialJointComponent> NextJoint, int& BestJointPort, int& BestSocketPort);

    ObjectPtr<StaticMesh> GenerateJointMeshWithPort(int PortIndex);
    ObjectPtr<StaticMesh> GenerateSocketMeshWithPort(int PortIndex);
    
    ObjectPtr<StaticMesh> GenerateJointMeshWithPort(TArray<int> PortIndexs);
    ObjectPtr<StaticMesh> GenerateSocketMeshWithPort(TArray<int> PortIndexs);
    /**
     * @brief Generate linkage mesh from local PortLocation to world Target location
     * 
     * @param PortLocation Local space
     * @param TargetLocation World space
     * @return ObjectPtr<StaticMesh> Linkage mesh
     */
    virtual ObjectPtr<StaticMesh> GenerateLinkageTo(FVector PortLocation, FVector TargetLocation);

	/**
	 * Create and assemble the joint mesh, will be called when BeginPlay
	 * @return The joint mesh
	 */
	virtual ObjectPtr<StaticMesh> CreateJointMesh() override;

	void AddNextJoint(ObjectPtr<SpatialJointComponent> NextJoint) { NextJointComponents.push_back(NextJoint); }
};


// All the joint initially extend to +x, and head to +z
// Please initialize the actor as initialization of the corresponding joint
class SpatialJointR : public SpatialJointComponent
{
public:
    double OuterRadius = 0.025;
    double InnerRadius = 0.0125;

    double SocketHeight = 0.04;
    double JointHeight = 0.025;

    SpatialJointR(const FTransform& InInitTransform, bool InIsRoot = false)
     : SpatialJointComponent(EDOF3D::FreeZ, EDOF3D::FreeNone, InInitTransform, InIsRoot)
    { }
    static ObjectPtr<StaticMesh> GenerateSocketMesh(double OuterRadius, double InnerRadius, double SocketHeight, double JointHeight);
    static ObjectPtr<StaticMesh> GenerateJointMesh(double OuterRadius, double InnerRadius, double Height);
    virtual ObjectPtr<StaticMesh> GenerateSocketMesh() override {
        return GenerateSocketMesh(OuterRadius, InnerRadius, SocketHeight, JointHeight);
    };
    virtual ObjectPtr<StaticMesh> GenerateJointMesh() override {
        return GenerateJointMesh(OuterRadius, InnerRadius, JointHeight);
    };
    virtual TArray<FVector> GetJointPortInMeshLocation() override;
    virtual TArray<FVector> GetSocketPortInMeshLocation() override;


};

class SpatialJointP : public SpatialJointComponent
{
public:
    // in Z
    double OuterSideLength = 0.075;
    double InnerSideLength = 0.05;
    // In x
    double Length = 0.04;

    double JointLeft = 0.2;
    double JointRight = 0.2;

    SpatialJointP(const FTransform& InInitTransform, bool InIsRoot = false)
     : SpatialJointComponent(EDOF3D::FreeNone, EDOF3D::FreeX, InInitTransform, InIsRoot)
    { }
    static ObjectPtr<StaticMesh> GenerateSocketMesh(double OuterSideLength, double InnerSideLength, double Length);
    static ObjectPtr<StaticMesh> GenerateJointMesh(double SideLength, double JointLeft, double JointRight);

    virtual ObjectPtr<StaticMesh> GenerateSocketMesh() override {
        return GenerateSocketMesh(OuterSideLength, InnerSideLength, Length);
    };
    virtual ObjectPtr<StaticMesh> GenerateJointMesh() override {
        return GenerateJointMesh(InnerSideLength, JointLeft, JointRight);
    };

    virtual TArray<FVector> GetJointPortInMeshLocation() override;
    virtual TArray<FVector> GetSocketPortInMeshLocation() override;
};

class SpatialJointC : public SpatialJointComponent
{
public:
    double OuterRadius = 0.04;
    double InnerRadius = 0.025;
    double SocketLength = 0.05;

    double JointLeft = 0.2;
    double JointRight = 0.2;

    SpatialJointC(const FTransform& InInitTransform, bool InIsRoot = false)
     : SpatialJointComponent(EDOF3D::FreeX, EDOF3D::FreeX, InInitTransform, InIsRoot)
    { }
    static ObjectPtr<StaticMesh> GenerateSocketMesh(double OuterRadius, double InnerRadius, double Length);
    static ObjectPtr<StaticMesh> GenerateJointMesh(double Radius, double JointLeft, double JointRight);

    virtual ObjectPtr<StaticMesh> GenerateSocketMesh() override {
        return GenerateSocketMesh(OuterRadius, InnerRadius, SocketLength);
    };
    virtual ObjectPtr<StaticMesh> GenerateJointMesh() override {
        return GenerateJointMesh(InnerRadius, JointLeft, JointRight);
    };

    virtual TArray<FVector> GetJointPortInMeshLocation() override;
    virtual TArray<FVector> GetSocketPortInMeshLocation() override;
};

class SpatialJointU : public SpatialJointComponent
{
public:
    double Diameter = 0.125;
    double SocketThickness = 0.05;
    double JointRadius = 0.05;
    double SupporterRadius = 0.01;

    SpatialJointU(const FTransform& InInitTransform, bool InIsRoot = false)
     : SpatialJointComponent(EDOF3D::FreeXY, EDOF3D::FreeNone, InInitTransform, InIsRoot)
    { }
    // Diameter in Y,Z  Thickness in Z. Cylinder raidus setup for supporter
    static ObjectPtr<StaticMesh> GenerateSocketMesh(double Diameter, double JointRadius, double Thickness, double CylinderRadius);
    static ObjectPtr<StaticMesh> GenerateJointMesh(double Radius);

    virtual ObjectPtr<StaticMesh> GenerateSocketMesh() override {
        return GenerateSocketMesh(Diameter, JointRadius, SocketThickness, SupporterRadius);
    };
    virtual ObjectPtr<StaticMesh> GenerateJointMesh() override {
        return GenerateJointMesh(JointRadius);
    };

    virtual TArray<FVector> GetJointPortInMeshLocation() override;
    virtual TArray<FVector> GetSocketPortInMeshLocation() override;
};

class SpatialJointS : public SpatialJointComponent
{
public:
    double OuterRadius = 0.05;
    double InnerRadius = 0.04;
    SpatialJointS(const FTransform& InInitTransform, bool InIsRoot = false)
     : SpatialJointComponent(EDOF3D::FreeXYZ, EDOF3D::FreeNone, InInitTransform, InIsRoot)
    { }
    // Will cut 0.5 Radius, result in [-60,60] deg
    static ObjectPtr<StaticMesh> GenerateSocketMesh(double OuterRadius, double InnerRadius);
    static ObjectPtr<StaticMesh> GenerateJointMesh(double Radius);

    virtual ObjectPtr<StaticMesh> GenerateSocketMesh() override {
        return GenerateSocketMesh(OuterRadius, InnerRadius);
    };
    virtual ObjectPtr<StaticMesh> GenerateJointMesh() override {
        return GenerateJointMesh(InnerRadius);
    };

    virtual TArray<FVector> GetJointPortInMeshLocation() override;
    virtual TArray<FVector> GetSocketPortInMeshLocation() override;
};

class EffectorJoint : public SpatialJointComponent
{
public:
    double Radius = 0.02;
    EffectorJoint(const FTransform& InInitTransform, bool InIsRoot = false)
	: SpatialJointComponent(FreeNone, FreeNone, InInitTransform, InIsRoot){ }

    virtual ObjectPtr<StaticMesh> GenerateSocketMesh() override {
        return BasicShapesLibrary::GenerateSphere(Radius);
    };
    virtual ObjectPtr<StaticMesh> GenerateJointMesh() override {
        return BasicShapesLibrary::GenerateSphere(Radius);
    };

    virtual TArray<FVector> GetJointPortInMeshLocation() override {return {FVector::Zero()};};
    virtual TArray<FVector> GetSocketPortInMeshLocation() override  {return {FVector::Zero()};};

};

class SpatialJointActor : public Actor
{
protected:
    ObjectPtr<SpatialJointComponent> JointComponent;

public:
    SpatialJointActor(char Type, const FTransform& InitTransform = FTransform::Identity(), bool IsRoot = false) : Actor(InitTransform)
    {
        if (Type == 'R') JointComponent = AddComponent<SpatialJointR>(InitTransform, IsRoot);
        else if (Type == 'P') JointComponent = AddComponent<SpatialJointP>(InitTransform, IsRoot);
        else if (Type == 'C') JointComponent = AddComponent<SpatialJointC>(InitTransform, IsRoot);
        else if (Type == 'U') JointComponent = AddComponent<SpatialJointU>(InitTransform, IsRoot);
        else if (Type == 'S') JointComponent = AddComponent<SpatialJointS>(InitTransform, IsRoot);
        else if (Type == 'E') JointComponent = AddComponent<EffectorJoint>(InitTransform, IsRoot);
		else ASSERTMSG(false, "Type {0} is not a valid Spatial joint", Type);
    }

    ObjectPtr<SpatialJointComponent> GetJointComponent() { return JointComponent; }
    void AddNextJoint(const ObjectPtr<Joint>& NextJoint)
    {
        JointComponent->GetJoint()->AddNextJoint(NextJoint);
    }
    void AddNextJoint(const ObjectPtr<SpatialJointComponent>& NextJoint)
    {
        JointComponent->GetJoint()->AddNextJoint(NextJoint->GetJoint());
        JointComponent->AddNextJoint(NextJoint);
    }
    void AddNextJoint(const ObjectPtr<SpatialJointActor>& NextJoint)
    {
        JointComponent->GetJoint()->AddNextJoint(NextJoint->GetJointComponent()->GetJoint());
        JointComponent->AddNextJoint(NextJoint->GetJointComponent());
    }

};