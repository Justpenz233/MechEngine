#pragma once

#include "Core/CoreMinimal.h"
#include "Math/FTransform.h"
#include "Object/Object.h"
#include "Components/ActorComponent.h"
#include "Components/TransformComponent.h"

DECLARE_MULTICAST_DELEGATE(FOnTransformUpdate);
// Actor is an actor in the world, which contains a transform data and also hierarchy information
class TransformComponent;
class World;

MCLASS(Actor)
class ENGINE_API Actor : public Object
{
	REFLECTION_BODY(Actor)
public:
	Actor(const FVector& InitLocation = FVector::Zero(), const FQuat& InitRotation = FQuat::Identity(), const FVector& InitScale = FVector::Ones());
	Actor(const FTransform& InitTransfrom);

    //After Constructor and before BeginPlay
    virtual void Init();

	virtual void PostEdit(Reflection::FieldAccessor& Field) override;

	std::function<void()> BeginPlayFunction;
	virtual void BeginPlay();

	std::function<void(double, Actor*)> TickFunction;
    virtual void Tick(double DeltaTime);

	std::function<void()> EndPlayFunction;
	virtual void EndPlay();

	FORCEINLINE class World* GetWorld() const;

	FORCEINLINE TransformComponent* GetTransformComponent() const;

	FORCEINLINE Eigen::Affine3d GetTransform() const;
	FORCEINLINE FTransform GetFTransform() const;
    FORCEINLINE FVector GetTranslation() const;
	FORCEINLINE FVector GetLocation() const;
	// Result in X,Y,Z , rotation order is x,y,z
    FORCEINLINE FVector GetRotationEuler() const;
	FORCEINLINE FQuat GetRotation() const;
    FORCEINLINE FVector GetScale() const;
	FORCEINLINE Eigen::Matrix4d GetTransformMatrix() const;
	FORCEINLINE Actor* SetTransform(const Eigen::Affine3d& InTransform);
	FORCEINLINE Actor* SetTransform(const FTransform& InTransform);
	FORCEINLINE Actor* SetTranslation(const FVector& Translation);
	FORCEINLINE Actor* AddTranslationGlobal(const FVector& DeltaTranslation);
	FORCEINLINE Actor* AddTranslationLocal(const FVector& DeltaTranslation);

	// Make sure set Rotation before Scale
	FORCEINLINE Actor* SetRotation(const FVector& InRotation);
	FORCEINLINE Actor* SetRotation(const FQuat& InRotation);
	FORCEINLINE Actor* AddRotationLocal(const FVector& DeltaRotation);
	FORCEINLINE Actor* AddRotationLocal(const FQuat& DeltaRotation);
	FORCEINLINE Actor* AddRotationGlobal(const FVector& DeltaRotation);
	FORCEINLINE Actor* AddRotationGlobal(const FQuat& DeltaRotation);
	// Make sure set Scale after rotation
	FORCEINLINE Actor* SetScale(const FVector& InScale);
	FORCEINLINE Actor* AddScale(const FVector& DeltaScale);

	void AddChild (ObjectPtr<Actor> InChild);

	WeakObjectPtr<Actor> GetRootActor();

	FORCEINLINE ObjectPtr<Actor> GetThis();

	FORCEINLINE TArray<ObjectPtr<ActorComponent>> GetAllComponents();

	template <class T, typename... Args> requires IsActorComponent<T>
	ObjectPtr<T> AddComponent(Args&&... args);

	template <class T> requires IsActorComponent<T>
	ObjectPtr<T> GetComponent();

	template <class T>
	TArray<ObjectPtr<T>> GetAllComponentsOfClass();

	template <class T>
	TArray<ObjectPtr<T>> GetComponentsFromChild();

	FORCEINLINE FOnTransformUpdate& GetTransformUpdateDelegate();


protected:
	bool HasBeginPlay = false;
	/**
	 * This is a pointer to the world that this actor belongs to
	 * This filed is set by the world when the actor is added or spawned to the world
	 */
	class World* World = nullptr;

	TArray<ObjectPtr<ActorComponent>> Components;

	// RootComponent store the transform of the actor
	TransformComponent* Transform;

	TArray<WeakObjectPtr<Actor>> Children;
	WeakObjectPtr<Actor> Parent;

	FOnTransformUpdate TransformUpdateDelegate;

	friend class World;
};

FORCEINLINE World* Actor::GetWorld() const
{
	return World;
}

FORCEINLINE TransformComponent* Actor::GetTransformComponent() const
{
	return Transform;
}

FORCEINLINE ObjectPtr<Actor> Actor::GetThis()
{
	return Cast<Actor>(Object::GetThis());
}

FORCEINLINE Eigen::Affine3d Actor::GetTransform() const
{
    return Eigen::Affine3d(GetTransformMatrix());
}

FORCEINLINE FTransform Actor::GetFTransform() const
{
	return Transform->GetFTransform();
}

FORCEINLINE Eigen::Matrix4d Actor::GetTransformMatrix() const
{
	return Transform->GetTransformMatrix();
}

FORCEINLINE FVector Actor::GetTranslation() const
{
    return Transform->GetLocation();
}

FORCEINLINE FVector Actor::GetLocation() const
{
	return Transform->GetLocation();
}

FORCEINLINE FVector Actor::GetRotationEuler() const
{
	return Transform->GetRotationEuler();
}

FORCEINLINE FQuat Actor::GetRotation() const
{
	return Transform->GetRotation();
}

FORCEINLINE FVector Actor::GetScale() const
{
	return Transform->GetScale();
}

FORCEINLINE Actor* Actor::SetTransform(const Eigen::Affine3d& InTransform)
{
	Transform->SetTransform(InTransform);
	return this;
}

FORCEINLINE Actor* Actor::SetTransform(const FTransform& InTransform)
{
	Transform->SetTransform(InTransform);
	return this;
}

FORCEINLINE Actor* Actor::SetTranslation(const FVector& Translation)
{
	Transform->SetTranslation(Translation);
	return this;
}
FORCEINLINE Actor* Actor::AddTranslationGlobal(const FVector &DeltaTranslation)
{
	Transform->AddTranslationGlobal(DeltaTranslation);
	return this;
}

FORCEINLINE Actor* Actor::AddTranslationLocal(const FVector &DeltaTranslation)
{
	Transform->AddTranslationLocal(DeltaTranslation);
	return this;
}

FORCEINLINE Actor* Actor::SetRotation(const FVector& InRotation)
{
	Transform->SetRotation(InRotation);
	return this;
}

FORCEINLINE Actor* Actor::SetRotation(const FQuat &InRotation)
{
	Transform->SetRotation(InRotation);
	return this;
}

FORCEINLINE Actor* Actor::AddRotationLocal(const FVector& DeltaRotation)
{
	Transform->AddRotationLocal(DeltaRotation);
	return this;
}

FORCEINLINE Actor* Actor::AddRotationLocal(const FQuat &DeltaRotation)
{
	Transform->AddRotationLocal(DeltaRotation);
	return this;
}

FORCEINLINE Actor* Actor::AddRotationGlobal(const FVector& DeltaRotation)
{
	Transform->AddRotationGlobal(DeltaRotation);
	return this;
}

FORCEINLINE Actor* Actor::AddRotationGlobal(const FQuat &DeltaRotation)
{
	Transform->AddRotationGlobal(DeltaRotation);
	return this;
}

FORCEINLINE Actor* Actor::SetScale(const FVector& InScale)
{
	Transform->SetScale(InScale);
	return this;
}

FORCEINLINE Actor* Actor::AddScale(const FVector &DeltaScale)
{
	Transform->AddScale(DeltaScale);
	return this;
}

FORCEINLINE TArray<ObjectPtr<ActorComponent>> Actor::GetAllComponents()
{
	return Components;
}

FORCEINLINE FOnTransformUpdate& Actor::GetTransformUpdateDelegate()
{
	return TransformUpdateDelegate;
}

template <class T, typename... Args> requires IsActorComponent<T>
	ObjectPtr<T> Actor::AddComponent(Args&&... args)
{
	auto NewComponent = NewObject<T>(std::forward<Args>(args)...);
	NewComponent->SetOwner(this);
	NewComponent->World = World;
	Components.push_back(NewComponent);
	return NewComponent;
}

template <class T> requires IsActorComponent<T>
ObjectPtr<T> Actor::GetComponent()
{
	for(auto i : Components)
	{
		if(auto Component = Cast<T>(i))
		{
			return Component;
		}
	}
	return nullptr;
}

template <class T>
TArray<ObjectPtr<T>> Actor::GetAllComponentsOfClass()
{
	TArray<ObjectPtr<T>> Result;
	for(auto i : Components)
	{
		if(auto Component = Cast<T>(i))
		{
			Result.push_back(Component);
		}
	}
	return Result;
}

template <class T>
TArray<ObjectPtr<T>> Actor::GetComponentsFromChild()
{
	TArray<ObjectPtr<T>> Result;

	auto ComponentFromSelf = GetAllComponentsOfClass<T>();
	Result.insert(Result.end(), ComponentFromSelf.begin(), ComponentFromSelf.end());

	for(auto i : Children)
	{
		if(!i.expired())
		{
			auto ComponentFromChild = i.lock()->GetComponentsFromChild<T>();
			Result.insert(Result.end(), ComponentFromChild.begin(), ComponentFromChild.end());
		}
	}

	return Result;
}