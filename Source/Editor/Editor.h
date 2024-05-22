//
// Created by Li Siqi on 2023/6/26.
//

#pragma once

#include "CoreMinimal.h"
#include "Delegate.h"
#include "Game/World.h"
#include "Render/RendererInterface.h"

#define GEditor Editor::Get()

DECLARE_MULTICAST_DELEGATE(KeyPressedEvent);

/**
 * MechEngine Editor for multi world
 * First Init the editor, then load the world
 */
class ENGINE_API Editor
{
public:
    static Editor &Get();

	void Init(const std::string& BinPath, const std::string& ProjectDir);

	void Start();

	FORCEINLINE World* GetWorld() const;

	void LoadWorld(TFunction<void(class World&)>&& InitScript);

private:
	RenderPipelineType PipelineType;

	UniquePtr<RenderPipeline> Renderer = nullptr;

	UniquePtr<World> CurrentWorld;

	Editor() = default;
};

FORCEINLINE World* Editor::GetWorld() const
{
	return CurrentWorld.get();
}