//
// Created by Li Siqi on 2023/6/15.
//

#include "Core/CoreMinimal.h"
#include "World.h"
#include "Components/StaticCurveComponent.h"
#include "Actors/CameraActor.h"
#include "Components/CameraComponent.h"
#include "Game/Actor.h"
#include "Render/GPUSceneInterface.h"

World* GWorld = nullptr;

World::~World()
{

}

bool World::OnKeyPressed(int Key)
{
	if(KeyEvents.count(Key))
	{
		KeyEvents[Key].Broadcast();
        return true;
	}
    return false;
}

void World::MarkAsDirty() {
    bDirty = true;
}

void World::BeginPlay()
{
	LOG_INFO("GWorld begin play");

	//Init world
	if(BeginPlayScript)
    	BeginPlayScript(*this);

	// Check if contains at least one camera
	bool HasCamera = false;
	for (auto& Actor: Actors) {
		if (Actor->GetComponent<CameraComponent>()) {
			HasCamera = true;
			break;
		}
	}
	if (!HasCamera) {
		SpawnActor<CameraActor>("Camera")->SetTranslation({-5, 0, 0});
	}

	for(auto actor : Actors)
		actor->BeginPlay();

	if(SelectedActor.expired() && !Actors.empty())
		SelectActor(Actors[0]);
}

void World::Tick(double DeltaTime)
{
	for (auto Object: Actors) {
		if(!Object->HasBeginPlay) {
			Object->BeginPlay();
		}
	}

	for (auto Object: Actors) {
		Object->Tick(DeltaTime);
	}

	if(TickFunction) {
		TickFunction(DeltaTime, *this);
	}
}

void World::EndPlay()
{
	if(EndPlayScript)
		EndPlayScript(*this);
	
	for(auto actor : Actors)
		actor->EndPlay();

	while(!Actors.empty())
		DestroyActor(*Actors.begin());

	Actors.clear();
}

void World::DestroyActor(const ObjectPtr<Actor>& InActor)
{
	for (auto i = Actors.begin(); i != Actors.end(); i++)
	{
		if (*i == InActor)
		{
			Actors.erase(i);
			break;
		}
	}
	// here, suppose to auto run destructor to the actor
}

void World::SelectActor(const ObjectPtr<Actor>& InActor)
{
	if(!SelectedActor.expired())
		SelectedActor.lock()->SetSelected(false);
	SelectedActor = InActor;
	InActor->SetSelected(true);
	OnActorSelectedEvent.Broadcast(InActor.get());
}

void World::SetViewMode(const ViewMode& Mode) const
{
	GetScene()->ViewModeSet(Mode);
}

void World::DebugDrawPoint(const FVector& Point, const FVector& Color)
{

}

void World::DebugDrawLine(const FVector& Start, const FVector& End, const FVector& Color)
{

}

template <class T>
void World::BindKeyPressedEvent(int Key, ObjectPtr<T> Object, void(T::* FuncPtr)())
{
	if(KeyEvents.count(Key))
		KeyEvents[Key].AddMember(Object, FuncPtr);
	else
	{
		KeyPressedEvent Event;
		Event.AddMember(Object, FuncPtr);
		KeyEvents[Key] = Event;
	}
}

void World::Init(Rendering::GPUSceneInterface* InScene, class ViewportInterface* InViewport)
{
	Viewport = InViewport;
	GPUScene = InScene;

	LOG_INFO("Load default Editor layout");

	if(InitScript)
		InitScript(*this);
}