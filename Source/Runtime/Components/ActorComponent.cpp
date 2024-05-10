//
// Created by Siqi Li on 8/24/2023.
//
#include "Game/Actor.h"
#include "ActorComponent.h"


std::string ActorComponent::GetOwnerName() const
{
	return Owner->GetName();
}