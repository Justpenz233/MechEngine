//
// Created by Li Siqi on 2023/6/15.
//

#pragma once

#include "Core/CoreMinimal.h"
#include "Delegate.h"
#include "Object/Object.h"
#include "Actor.h"
#include "Render/ViewportInterface.h"
#include "Render/Core/ViewMode.h"

class TimerManager;
namespace MechEngine::Rendering
{
	class GpuSceneInterface;
}

DECLARE_MULTICAST_DELEGATE(KeyPressedEvent);
DECLARE_MULTICAST_DELEGATE_PARMS(OnActorSelectedDelegate, class Actor*);


/// This class will manage Animation and Tick and Signal
class ENGINE_API World : public Object
{
public:

    World();

	~World();

	/**
	 * Initialize the world with a viewport and a render pipeline, will create a new GPU scene for this world
	 * @param InViewport Viewport of currenty window, used for handle input and add UI widgets
	 * @param InScene GPU Scene for this world, used for create a new GPU scene for this world. For component to upload GPU data
	 */
	void Init(Rendering::GpuSceneInterface* InScene, class ViewportInterface* InViewport);
    
    TFunction<void(double, World&)> TickFunction;
    TFunction<void(World&)> BeginPlayScript;
    TFunction<void(World&)> EndPlayScript;
    TFunction<void(World&)> InitScript;
    
    void MarkAsDirty();

	virtual void BeginPlay();
	virtual void Tick(double DeltaTime);
	virtual void EndPlay();

	/**
	 * Spawn an actor in the world
	 * @tparam T Actor type
	 * @tparam Args Constructor arguments for the actor
	 * @param ActorName Name of the actor, shoule be unique
	 * @param args Constructor arguments for the actor
	 * @return The actor pointer
	 */
	template <class T, typename... Args>
	ObjectPtr<T> SpawnActor(const String& ActorName, Args&&... args);

	/**
	 * Add an already constructed actor to the world
	 * @tparam T Actor type
	 * @param InActor Actor to be added
	 * @return The actor pointer
	 */
	template <class T>
	ObjectPtr<T> AddActor(ObjectPtr<T> InActor);

	/**
	 * Destroy and remove actor in the world
	 * @param ToDestroyActor Actor to be destroyed
	 */
	void DestroyActor(Actor* ToDestroyActor);

    TArray<ObjectPtr<class Actor>> GetAllActors() { return Actors; }

	void SelectActor(const ObjectPtr<class Actor>& InActor);

	class CameraActor* GetCurrentCamera() const;

    FORCEINLINE Actor* GetSelectedActor() const;
    FORCEINLINE Rendering::GpuSceneInterface* GetScene() const { return GPUScene; }
	FORCEINLINE class ViewportInterface* GetViewport() const { return Viewport; }

	template<class T>
	void BindKeyPressedEvent(int Key, ObjectPtr<T> Object, void(T::* FuncPtr)());

    /**
     * Add a widget to the world, the widget will be rendered in the viewport
     * @tparam T Widget type, whoule be derived from UIWidget
     * @tparam Args Constructor arguments for the widget
     * @param args Constructor arguments for the widget
     * @return The widget pointer
     */
    template<class T, class... Args>
	T* AddWidget(Args&&... args);

	/************************************************************************
	 *						Rendering functions
	 ************************************************************************/
	void SetViewMode(const ViewMode& Mode) const;

	void DebugDrawPoint(const FVector& WorldPosition, double Radius, const FColor& Color, double LifeTime = -1.);

	void DebugDrawLine(const FVector& WorldStart, const FVector& WorldEnd, const FColor& Color, double Thickness = 2., double LifeTime = -1.);

	void DebugDrawCube(const FVector& Center, const FVector& Size, const FColor& Color, double Thickness = 2., double LifeTime = -1.);

	void DebugDrawBox(const FBox& Box, const FTransform& Transform, const FColor& Color, double Thickness = 2., double LifeTime = -1.);
	/**
	 * Get the timer manager of the world, used for control the timer in the world
	 * The timer will be ticked by the world
	 * Attention: The timer manager will not be more precise than the world tick, so the timer may not be accurate
	 * @return Timer manager of the world
	 */
	FORCEINLINE TimerManager* GetTimerManager();

	/**
	 * Export the scene to a obj file, the obj file will be saved in the folder
	 * @param FolderPath Folder path to save the obj file
	 * @param bExportGlobal Export to global space if true, otherwise export to local space
	 */
	void ExportSceneToObj(const Path& FolderPath, bool bExportGlobal = true);

	struct RayCastHit RayCastQuery(uint PixelX, uint PixelY) const;


private:
    Rendering::GpuSceneInterface* GPUScene = nullptr;
	ViewportInterface* Viewport = nullptr;
	bool bDirty = true;

	TArray<ObjectPtr<Actor>> Actors;
	WeakObjectPtr<Actor> SelectedActor;

	TArray<ObjectPtr<class UIWidget>> Widgets;

	TMap<int, KeyPressedEvent> KeyEvents;
	OnActorSelectedDelegate OnActorSelectedEvent;
	bool OnKeyPressed(int Key);

	TArray<ObjectPtr<class CurveComponent>> DebugCurves;

	UniquePtr<TimerManager> TimerManager;

	UniquePtr<class LinesComponent> DebugDrawComponent;

	friend class Editor;
};


template <class T, typename... Args>
ObjectPtr<T> World::SpawnActor(const String& ActorName, Args&&... args)
{
	static_assert(std::derived_from<T, Actor>, "ERROR SpawnActor: class T is not subclass of Actor");
	auto NewActor = NewObject<T>(std::forward<Args>(args)...);
	NewActor->SetName(ActorName);
	NewActor->World = this;
	Actors.push_back(NewActor);
	for(auto Component : NewActor->GetAllComponents())
	{
		Component->SetOwner(NewActor.get());
		Component->World = this;
	}
	// Init all components and then actor
	NewActor->Init();
	return NewActor;
}

template <class T>
ObjectPtr<T> World::AddActor(ObjectPtr<T> InActor)
{
	InActor->World = this;
	Actors.push_back(InActor);
	for(auto Component : InActor->GetAllComponents())
	{
		Component->SetOwner(InActor.get());
		Component->World = this;
	}
	return InActor;
}

template<class T, class... Args>
T* World::AddWidget(Args&&... args)
{
	static_assert(std::is_base_of_v<UIWidget, T>, "T must be derived from UIWidget");
	static_assert(std::is_constructible_v<T, Args...>, "T must be constructible with Args");
	volatile auto RawMemory = std::malloc(sizeof(T)); // Force non reorder
	static_cast<T*>(RawMemory)->World = this;
	auto Widget = SharedPtr<T>(new(RawMemory) T(std::forward<Args>(args)...));
	Viewport->AddWidget(Cast<UIWidget>(Widget));
	Widgets.emplace_back(Widget);
	return static_cast<T*>(Widgets.back().get());
}

FORCEINLINE Actor* World::GetSelectedActor() const
{
	if(SelectedActor.expired())
	{
		return nullptr;
	}
	return SelectedActor.lock().get();
}

FORCEINLINE TimerManager* World::GetTimerManager()
{
	return TimerManager.get();
}

extern ENGINE_API World* GWorld;