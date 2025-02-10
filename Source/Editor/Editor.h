//
// Created by Li Siqi on 2023/6/26.
//

#pragma once

#include "CoreMinimal.h"
#include "Delegate.h"
#include "Game/World.h"
#include "Render/PipeLine/RenderPipeline.h"

#define GEditor Editor::Get()

DECLARE_MULTICAST_DELEGATE(KeyPressedEvent);

enum WorldCommandType
{
	Load, Unload, Save
};

typedef std::tuple<WorldCommandType, TFunction<void(class World&)>> WorldCommand;

/**
 * MechEngine Editor for multi world
 * First Init the editor, then load the world
 */
class EDITOR_API Editor
{
public:
    static Editor &Get();

	void Init(const std::string& BinPath, const std::string& ProjectDir);

	void Start();

	/**
	 * Load a world by script
	 * @param InitScript The script to init the world
	 */
	void LoadWorld(const TFunction<void(class World&)>& InitScript);

	[[nodiscard]] FORCEINLINE World* GetWorld() const;

	/**
	 * Set the max FPS for Engine
	 * @param InMaxFPS -1 means no limit
	 */
	FORCEINLINE void SetMaxFPS(float InMaxFPS);

	FORCEINLINE float GetMaxFPS() const;

private:
	void Tick(double DeltaTime);

	void UnloadCurrentWorldImpl();
	void LoadWorldImpl(const TFunction<void(class World&)>& InitScript);
	// void SaveWorldImpl();

	float MaxFPS = -1;

	UniquePtr<RenderPipeline> Renderer = nullptr;

	UniquePtr<World> CurrentWorld;

	Editor() = default;

	std::queue<WorldCommand> WorldCommandQueue;
};

FORCEINLINE World* Editor::GetWorld() const
{
	return CurrentWorld.get();
}

FORCEINLINE void Editor::SetMaxFPS(float InMaxFPS)
{
	MaxFPS = InMaxFPS;
}

FORCEINLINE float Editor::GetMaxFPS() const
{
	return MaxFPS;
}