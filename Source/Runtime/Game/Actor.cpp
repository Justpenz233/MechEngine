#include "Actor.h"
#include "Core/CoreMinimal.h"
#include "Math/FTransform.h"
#include "Components/TransformComponent.h"

Actor::Actor(const FVector& InitLocation, const FQuat& InitRotation, const FVector& InitScale)
{
	Transform = AddComponent<TransformComponent>().get();
	SetTranslation(InitLocation);
	SetRotation(InitRotation);
	SetScale(InitScale);
}

Actor::Actor(const FTransform& InTransform)
{
	Transform = AddComponent<TransformComponent>().get();
	SetTransform(InTransform);
}

void Actor::Init()
{
	for (auto Component : GetAllComponents())
		Component->Init();
}

void Actor::PostEdit(Reflection::FieldAccessor& Field)
{
	Object::PostEdit(Field);
}

void Actor::BeginPlay()
{
	if (HasBeginPlay)
	{
		LOG_WARNING("Actor {0} has already begin play", GetName());
		return;
	}
	HasBeginPlay = true;
	if(BeginPlayFunction) BeginPlayFunction();
	for(auto Component: Components)
	{
		if(Component)
		{
			Component->BeginPlay();
		}
	}
}

void Actor::Tick(double DeltaTime)
{
	if(TickFunction) TickFunction(DeltaTime, this);
	for(auto& Component: Components)
	{
		if(Component)
		{
			Component->TickComponent(DeltaTime);
		}
	}
}

void Actor::EndPlay()
{
	if(EndPlayFunction) EndPlayFunction();
	for(auto Component: Components)
	{
		if(Component)
		{
			Component->EndPlay();
		}
	}
}

void Actor::AddChild(ObjectPtr<Actor> InChild)
{
	if(InChild)
	{
		Children.push_back(InChild);
		InChild->Parent = Cast<Actor>(weak_from_this());
	}
	else{
		// LOG_ERROR();
	}
}

WeakObjectPtr<Actor> Actor::GetRootActor()
{
	if (Parent.expired())
		return Cast<Actor>(weak_from_this());
	auto Root = Parent;
	while (!Root.lock()->Parent.expired())
	{
		Root = Root.lock()->Parent;
	}
	return Root;
}