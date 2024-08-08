//
// Created by Siqi Li on 8/24/2023.
//

#pragma once
#include "Delegate.h"
#include "Core/CoreMinimal.h"
#include "Object/Object.h"

MCLASS()
class ENGINE_API ActorComponent : public Object
{
	REFLECTION_BODY()
public:
	bool ShouldTick = true;

	/** Get the Actor that 'Owns' this component */
	FORCEINLINE class Actor* GetOwner() const;

	/** Templated version of GetOwner(), will return nullptr if cast fails */
	template <class T>
	T* GetOwner() const
	{
		return Cast<T>(GetOwner());
	}

	FORCEINLINE void SetOwner(Actor*);
	String GetOwnerName() const;

	FORCEINLINE World* GetWorld() const;

	// Called after this object is created
    virtual void Init() {};
	// Called when game start
	virtual void BeginPlay() {};
	virtual void TickComponent(double DeltaTime){};
	// Called when game end
	virtual void EndPlay() {};

protected:
	class Actor* Owner = nullptr;
	class World* World = nullptr;
	friend class Actor;
	friend class World;
};

FORCEINLINE void ActorComponent::SetOwner(Actor* OwnerActor)
{
	Owner = OwnerActor;
}

FORCEINLINE class Actor* ActorComponent::GetOwner() const
{
	return Owner;
}

FORCEINLINE World* ActorComponent::GetWorld() const
{
	return World;
}
template <class T>
concept IsActorComponent = std::is_base_of_v<ActorComponent, T>;