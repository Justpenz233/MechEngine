//
// Created by Siqi Li on 8/24/2023.
//
#include "Game/Actor.h"
#include "ActorComponent.h"


std::string ActorComponent::GetOwnerName() const
{
	return Owner->GetName();
}

void ActorComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	Object::PostEdit(Field);
	PostEditDelegate.Broadcast(Field);
}

void ActorComponent::SetSelected(bool InSelected)
{
	bool bPreSelected = Selected;
	Selected = InSelected;
	if(!bPreSelected && InSelected) OnSelected();
	if(bPreSelected && !InSelected) OnCancleSelected();
}