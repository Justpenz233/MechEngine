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
	ConfigFiles.insert({ FileName, inih::INIReader(InFilename) });
	if (int ErrorCode = ConfigFiles[FileName].ParseError(); ErrorCode != 0)
	{
		LOG_ERROR("Failed to load config file: {}, error code: {}", InFilename.c_str(), ErrorCode);
	}
}

void Config::UnloadFile(const String& Filename)
{
	auto File = ConfigFiles.find(Path(Filename).FileNameWithoutExtension().string());
	if( File != ConfigFiles.end())
		ConfigFiles.erase(File);
}

} // namespace MechEngine