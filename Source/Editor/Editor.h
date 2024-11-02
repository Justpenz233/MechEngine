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

	[[nodiscard]] FORCEINLINE World* GetWorld() const;

	/**
	 * Set the max FPS for Engine
	 * @param InMaxFPS -1 means no limit
	 */
	FORCEINLINE void SetMaxFPS(float InMaxFPS);

	FORCEINLINE float GetMaxFPS() const;

	void LoadWorld(TFunction<void(class World&)>&& InitScript);

private:
	float MaxFPS = -1;

	UniquePtr<RenderPipeline> Renderer = nullptr;

	UniquePtr<World> CurrentWorld;

	Editor() = default;
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