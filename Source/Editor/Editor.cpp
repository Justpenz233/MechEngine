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
#include "Render/PipeLine/RenderPipeline.h"
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
		for (const auto& EngineConfigFile : Path::DirectoryIterator(Path::EngineConfigDir()))
		{
			if (EngineConfigFile.is_regular_file() && EngineConfigFile.path().extension() == ".ini")
			{
				GConfig.LoadFile(EngineConfigFile.path().string());
			}
		}
		// Load all the ini file in the project config directory
		for (const auto& ProjectConfigFile : Path::DirectoryIterator(Path::ProjectConfigDir()))
		{
			if (ProjectConfigFile.is_regular_file() && ProjectConfigFile.path().extension() == ".ini")
			{
				GConfig.LoadFile(ProjectConfigFile.path().string());
			}
		}
	}
	// ---------- Init Renderer ----------
	auto WindowName = GConfig.Get<String>("Basic", "WindowName");
	auto Width = GConfig.Get<int>("Render", "ResolutionX");
	auto Height = GConfig.Get<int>("Render", "ResolutionY");
	MaxFPS = GConfig.Get<float>("Render", "MaxFPS");
	Renderer = MakeUnique<RenderPipeline>(Width, Height, WindowName);
	//--------- Reflection meta infomation register ----------
	Reflection::TypeMetaRegister::metaRegister();
}

void Editor::LoadWorld(const TFunction<void(class World&)>& InitScript)
{
	WorldCommandQueue.emplace( Unload, nullptr );
	WorldCommandQueue.emplace( Load, InitScript );
}

void Editor::UnloadCurrentWorldImpl()
{
	if(!CurrentWorld) return;
	LOG_INFO("Unloading map");
	CurrentWorld->EndPlay();
	CurrentWorld.reset();
}

void Editor::LoadWorldImpl(const TFunction<void(class World&)>& InitScript)
{
	LOG_INFO("Load world map by script");

	CurrentWorld = MakeUnique<World>();
	CurrentWorld->InitScript = InitScript;
	GWorld = CurrentWorld.get();

	Renderer->Init();
	// Init World, setup game logic and read maps
	CurrentWorld->Init(Renderer->NewScene(CurrentWorld.get()), Renderer->GetViewport());

	LoadDefaultEditorLayout(CurrentWorld.get());

	CurrentWorld->BeginPlay();
	LOG_INFO("Load world map done");
}

void Editor::Tick(double DeltaTime)
{
	// First handle the world command
	if(CurrentWorld)
	{
		CurrentWorld->Tick(DeltaTime);
		Renderer->RenderFrame();
	}
	while(!WorldCommandQueue.empty())
	{
		auto& Command = WorldCommandQueue.front();
		switch(std::get<0>(Command))
		{
		case Load:
			LoadWorldImpl(std::get<1>(Command));
			break;
		case Unload:
			UnloadCurrentWorldImpl();
			break;
		case Save:
			break;
		}
		WorldCommandQueue.pop();
	}
}

void Editor::Start()
{

	auto NowFrame = std::chrono::high_resolution_clock::now();
	auto LastFrame = std::chrono::high_resolution_clock::now();
	LOG_INFO("Editor START");

	while (!Renderer->ShouldClose())
	{
		NowFrame = std::chrono::high_resolution_clock::now();

		// Sleep for a while if the frame rate is too high
		if (
			MaxFPS > 0 && NowFrame - LastFrame < std::chrono::nanoseconds(int(1.0 / MaxFPS * 1e9)))
		{
			std::this_thread::sleep_for(std::chrono::nanoseconds(int(1.0 / MaxFPS * 1e9)) - (NowFrame - LastFrame));
			NowFrame = std::chrono::high_resolution_clock::now();
		}

		auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(NowFrame - LastFrame);
		LastFrame = NowFrame;
		Tick(static_cast<double>(delta.count()) * (double)1e-9);
		CalculateFPSTimings();
	}

	LOG_INFO("Closing editor");
	Renderer.reset();

	LOG_INFO("End play");
	CurrentWorld->EndPlay();
}
