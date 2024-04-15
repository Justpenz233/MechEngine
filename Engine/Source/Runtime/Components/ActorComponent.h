//
// Created by Siqi Li on 8/24/2023.
//

#pragma once
#include "Delegate.h"
#include "Core/CoreMinimal.h"
#include "Object/Object.h"

MCLASS()
class ActorComponent : public Object
{
	REFLECTION_BODY()
protected:
	class Actor* Owner;
	class World* World;
	friend class Actor;
	friend class World;

	bool Selected = false;

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

    // Called after this object is created
    virtual void Init() {};
	// Called when game start
	virtual void BeginPlay() {};
	virtual void TickComponent(double DeltaTime){};
	// Called when game end
	virtual void EndPlay() {};

	void SetSelected(bool InSelected);
	virtual void OnSelected() {}
	virtual void OnCancleSelected() {}

};

FORCEINLINE void ActorComponent::SetOwner(Actor* OwnerActor)
{
	Owner = OwnerActor;
}

FORCEINLINE class Actor* ActorComponent::GetOwner() const
{
	return Owner;
}

template <class T>
concept IsActorComponent = std::is_base_of_v<ActorComponent, T>;