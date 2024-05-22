//
// Created by MarvelLi on 2024/2/17.
//

#pragma once
#include "ini.h"
#include "Core/CoreMinimal.h"


namespace MechEngine
{

class ENGINE_API Config
{
private:
	TMap<String, inih::INIReader> ConfigFiles;

public:
	Config() = default;
	explicit Config(const String& ConfigFilePath);

	// Load the config file from the given file path
	void LoadFile(const String& InFilename);

	// Unload the config file from the given file path
	void UnloadFile(const String& Filename);

	template <typename T>
	T Get(const String& Section, const String& Key);

	template <typename T>
	TArray<T> GetArray( const String& Section, const String& Key);
};

template <typename T>
T Config::Get(const String& Section, const String& Key)
{
	for(const auto& [FileName, ConfigFile] : ConfigFiles)
	{
		if(ConfigFile.ContainKey(Section, Key))
			return ConfigFile.template Get<T>(Section, Key);
	}
	LOG_ERROR("Section {} , Key {} not found in config", Section, Key);
	return {};
}

template <typename T>
TArray<T> Config::GetArray(const String& Section, const String& Key)
{
	for(const auto& [FileName, ConfigFile] : ConfigFiles)
	{
		if(ConfigFile.ContainKey(Section, Key))
			return ConfigFile.template GetVector<T>(Section, Key);
	}
	LOG_ERROR("Section {} , Key {} not found in config", Section, Key);
	return {};
}

}

// Global config file, by dafault at least contains config files from Path::EngineConfigDir() and Path::ProjectConfigDir()
extern ENGINE_API MechEngine::Config GConfig;