//
// Created by MarvelLi on 2024/2/17.
//

#include "Config.h"
#include "Path.h"

ENGINE_API MechEngine::Config GConfig;

namespace MechEngine
{

Config::Config(const String& ConfigFilePath)
{
	LoadFile(ConfigFilePath);
}

void Config::LoadFile(const String& InFilename)
{
	String FileName = Path(InFilename).FileNameWithoutExtension().string();
	if (ConfigFiles.find(FileName) != ConfigFiles.end())
	{
		LOG_ERROR("Config file {} already loaded", InFilename.c_str());
		return;
	}
	mINI::INIFile file(Path(InFilename).string());
	mINI::INIStructure ini;
	if(file.read(ini))
		ConfigFiles.insert({ FileName, { std::move(file), std::move(ini) } });
	else
		LOG_ERROR("Failed to load config file: {}", InFilename.c_str());
}

void Config::UnloadFile(const String& Filename)
{
	auto File = ConfigFiles.find(Path(Filename).FileNameWithoutExtension().string());
	if( File != ConfigFiles.end())
		ConfigFiles.erase(File);
}

} // namespace MechEngine