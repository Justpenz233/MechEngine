//
// Created by Li Siqi, CIH-I on 2023/6/26.
//

#include "Editor.h"
#include "Core/reflection_register.h"
#include "Misc/Config.h"
#include "Misc/Path.h"

// Override new and delete with mi_malloc and mi_free
// #include <mimalloc.h>

#include "EditorDefaultLayout.h"
#include "Render/RayTracing/RayTracingPipeline.h"
#include "GlobalSymbols.h"

Editor &Editor::Get() {
    static Editor Instance;
    return Instance;
}

void Editor::Init(const std::string& BinPath, const std::string& ProjectDir)
{
	//----- Init path -----
	Path::Init(BinPath, ProjectDir);

	//----- Init logger -----
	// Init logger after path init, because the log file path is depend on the path
	Logger::Get();

	//----- Init config file -----
	{
		// Load all the ini file in the engine config directory
		for(const auto& EngineConfigFile : Path::DirectoryIterator(Path::EngineConfigDir()))
		{
			if(EngineConfigFile.is_regular_file() && EngineConfigFile.path().extension() == ".ini")
			{
				GConfig.LoadFile(EngineConfigFile.path().string());
			}
		}
		// Load all the ini file in the project config directory
		for(const auto& ProjectConfigFile : Path::DirectoryIterator(Path::ProjectConfigDir()))
		{
			if(ProjectConfigFile.is_regular_file() && ProjectConfigFile.path().extension() == ".ini")
			{
				GConfig.LoadFile(ProjectConfigFile.path().string());
			}
		}
	}
	// ---------- Init Renderer ----------

	// Init Renderer
	PipelineType = static_cast<RenderPipelineType>(GConfig.Get<int>("Render", "RenderPipelineType"));
	auto WindowName = GConfig.Get<String>("Basic", "WindowName");
	auto Width = GConfig.Get<int>("Render", "ResolutionX");
	auto Height = GConfig.Get<int>("Render", "ResolutionY");

	if(PipelineType == RenderPipelineType::RAYTRACING)
	{
		Renderer = MakeUnique<RayTracingPipeline>(Width, Height, WindowName);
	}
	else {
		LOG_ERROR("Not support render pipeline type");
	}

	//--------- Reflection meta infomation register ----------
	Reflection::TypeMetaRegister::metaRegister();
}

void Editor::LoadWorld(TFunction<void(World&)>&& InitScript)
{
	if (CurrentWorld)
	{
		LOG_ERROR("World already loaded");
		return;
	}
	LOG_INFO("Load world map by script");

	CurrentWorld = MakeUnique<World>();
	CurrentWorld->InitScript = std::move(InitScript);
	GWorld = CurrentWorld.get();

	Renderer->Init();
	// Init World, setup game logic and read maps
	CurrentWorld->Init(Renderer->NewScene(CurrentWorld.get()), Renderer->GetViewport());

	LoadDefaultEditorLayout(CurrentWorld.get());

	LOG_INFO("Load world map done");
}

void Editor::Start()
{
	if(!CurrentWorld)
	{
		LOG_ERROR("No world loaded!");
		return;
	}

	auto NowFrame = std::chrono::high_resolution_clock::now();
	auto LastFrame = std::chrono::high_resolution_clock::now();
	LOG_INFO("Editor START");

	CurrentWorld->BeginPlay();

	while (!Renderer->ShouldClose())
	{
		NowFrame = std::chrono::high_resolution_clock::now();
		auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(NowFrame - LastFrame);
		LastFrame = NowFrame;

		CurrentWorld->Tick((double) delta.count() * (double) 1e-9);
		Renderer->RenderFrame();
		CalculateFPSTimings();
	}

	LOG_INFO("Closing editor");
	Renderer.reset();

	LOG_INFO("End play");
	CurrentWorld->EndPlay();
}