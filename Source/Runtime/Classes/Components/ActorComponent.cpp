//
// Created by Siqi Li on 8/24/2023.
//
#include "Game/Actor.h"
#include "ActorComponent.h"

void ActorComponent::SetOwner(Actor* OwnerActor)
{
	Owner = OwnerActor;
}

std::string ActorComponent::GetOwnerName() const
{
	return Owner->GetName();
}