//
// Created by Li Siqi on 2023/6/15.
//

#include "Core/CoreMinimal.h"
#include "World.h"
#include "ImguiPlus.h"
#include "Components/StaticCurveComponent.h"
#include "Components/LightComponent.h"
#include "Components/CameraComponent.h"
#include "Actors/CameraActor.h"
#include "Game/Actor.h"
#include "Render/GPUSceneInterface.h"
#include "TimerManager.h"
#include "Components/ConstPointLightComponent.h"

World* GWorld = nullptr;

World::World()
{
	TimerManager = MakeUnique<class TimerManager>();
}
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
	CameraActor* Camera = nullptr;
	for (auto& Actor: Actors) {
		if (Actor->GetComponent<CameraComponent>()) {
			HasCamera = true;
			Camera = Cast<CameraActor>(Actor).get();
			break;
		}
	}
	if (!HasCamera) {
		Camera = SpawnActor<CameraActor>("Camera").get();
		Camera->SetTranslation({-5, 0, 0});
	}

	// Add a const light along with camera if no light in the scene
	bool HasLight = false;
	for (auto& Actor: Actors) {
		if (Actor->GetComponent<LightComponent>()) {
			HasLight = true;
			break;
		}
	}
	if (!HasLight) {
		Camera->AddComponent<ConstPointLightComponent>();
	}

	for(auto actor : Actors)
		actor->BeginPlay();

	if(SelectedActor.expired() && !Actors.empty())
		SelectActor(Actors[0]);
}

void World::Tick(double DeltaTime)
{
	for (auto& Object: Actors) {
		if(!Object->HasBeginPlay) {
			Object->BeginPlay();
		}
	}

	TimerManager->Tick(DeltaTime);

	for (auto& Object: Actors) {
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

CameraActor* World::GetCurrentCamera() const
{
	for (auto& Actor: Actors) {
		if (auto Component = Actor->GetComponent<CameraComponent>()) {
			return Cast<CameraActor>(Component->GetOwner());
		}
	}
	return nullptr;
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

void World::ExportSceneToObj(const Path& FolderPath)
{
	if(FolderPath.Existing() && FolderPath.IsDirectory())
	{
		for (auto& Actor : Actors)
		{
			if(auto MeshComponent = Actor->GetComponent<StaticMeshComponent>())
			{
				if(auto Mesh = MeshComponent->GetMeshData())
				{
					auto Transform = Actor->GetTransformMatrix();
					StaticMesh CopyMesh = *Mesh;
					CopyMesh.TransformMesh(Transform);
					CopyMesh.SaveOBJ(FolderPath / (Actor->GetName() + ".obj"));
				}
			}
		}
		ImGui::NotifySuccess(std::format("Export scene successfully, path: {} ", FolderPath.string()), "Export success", 1e4);
	}
	else
	{
		ImGui::NotifyError(std::format("Export scene to obj failed, folder path {} is invalid", FolderPath.string()), "Export failed", 1e4);
	}
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