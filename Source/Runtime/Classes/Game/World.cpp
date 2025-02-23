//
// Created by Li Siqi on 2023/6/15.
//

#include "Core/CoreMinimal.h"
#include "World.h"
#include "ImguiPlus.h"
#include "Components/CurveComponent.h"
#include "Components/LightComponent.h"
#include "Components/CameraComponent.h"
#include "Actors/CameraActor.h"
#include "Game/Actor.h"
#include "Render/GpuSceneInterface.h"
#include "TimerManager.h"
#include "Components/LinesComponent.h"
#include "Components/PointLightComponent.h"
#include "Render/PipeLine/GpuScene.h"

#include <spdlog/fmt/fmt.h>

ENGINE_API World* GWorld = nullptr;

World::World()
{
	TimerManager = MakeUnique<class TimerManager>();
	DebugDrawComponent = MakeUnique<LinesComponent>();
	DebugDrawComponent->World = this;
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
		Camera->AddComponent<PointLightComponent>();
	}

	for(auto actor : Actors)
		actor->BeginPlay();
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
	if (EndPlayScript)
		EndPlayScript(*this);

	for (auto actor : Actors)
		actor->EndPlay();

	for (const auto& actor : Actors)
		actor->Destroy();

	Actors.clear();
}

void World::DestroyActor(Actor* ToDestroyActor)
{
	if(ToDestroyActor->IsA<CameraActor>())
	{
		LOG_WARNING("Trying to destroy the camera actor, this is not allowed");
		// Should support in future when multiple camera supported
		return;
	}
	ToDestroyActor->Destroy();

	for (auto i = Actors.begin(); i != Actors.end(); ++ i)
	{
		if (i->get() == ToDestroyActor)
		{
			Actors.erase(i);
			break;
		}
	}
}

void World::SelectActor(const ObjectPtr<Actor>& InActor)
{
	if (!SelectedActor.expired())
		SelectedActor.lock()->SetSelected(false);
	SelectedActor = InActor;
	InActor->SetSelected(true);
	OnActorSelectedEvent.Broadcast(InActor.get());
}

void World::SelectActorByInstanceId(uint InstanceId)
{
	for (auto& Actor: Actors) {
		for(auto MeshComponent : Actor->GetAllComponentsOfClass<StaticMeshComponent>())
		{
			if(MeshComponent->GetInstanceID() == InstanceId)
			{
				SelectActor(Actor);
				return;
			}
		}
	}
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

void World::DebugDrawPoint(const FVector& WorldPosition, double Radius, const FColor& Color, double LifeTime)
{
	DebugDrawComponent->AddPoint(WorldPosition, Radius, Color, LifeTime);
}

void World::DebugDrawLine(const FVector& WorldStart, const FVector& WorldEnd, const FColor& Color, double Thickness, double LifeTime)
{
	DebugDrawComponent->AddLine(WorldStart, WorldEnd, Color, Thickness, LifeTime);
}
void World::DebugDrawCube(const FVector& Center, const FVector& Size, const FColor& Color, double Thickness, double LifeTime)
{
	DebugDrawComponent->AddCube(Center, Size, {}, Color, Thickness, LifeTime);
}

void World::DebugDrawBox(const FBox& Box, const FTransform& Transform, const FColor& Color, double Thickness, double LifeTime)
{
	DebugDrawComponent->AddCube(Box.GetCenter(), Box.GetSize(), Transform, Color, Thickness, LifeTime);
}

void World::ExportSceneToObj(const Path& FolderPath, bool bExportGlobal)
{
	if (FolderPath.Existing() && FolderPath.IsDirectory())
	{
		for (auto& Actor : Actors)
		{
			if (auto MeshComponent = Actor->GetComponent<StaticMeshComponent>())
			{
				if (auto Mesh = MeshComponent->GetMeshData())
				{
					if (!bExportGlobal)
						Mesh->SaveOBJ(FolderPath / (Actor->GetName() + ".obj"));
					else
					{
						auto	   Transform = Actor->GetTransformMatrix();
						StaticMesh CopyMesh = *Mesh;
						CopyMesh.TransformMesh(Transform);
						CopyMesh.SaveOBJ(FolderPath / (Actor->GetName() + ".obj"));
					}
				}
			}
		}
		ImGui::NotifySuccess(fmt::format("Export scene successfully, path: {} ", FolderPath.string()), "Export success", 1e4);
	}
	else
	{
		ImGui::NotifyError(fmt::format("Export scene to obj failed, folder path {} is invalid", FolderPath.string()), "Export failed", 1e4);
	}
}

RayCastHit World::RayCastQuery(const uint PixelX, const uint PixelY) const
{
	return Cast<Rendering::GpuScene>(GetScene())->RayCastQuery(PixelX, PixelY);
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

void World::Init(Rendering::GpuSceneInterface* InScene, class ViewportInterface* InViewport)
{
	Viewport = InViewport;
	GPUScene = InScene;

	LOG_INFO("Load default Editor layout");

	if(InitScript)
		InitScript(*this);
}