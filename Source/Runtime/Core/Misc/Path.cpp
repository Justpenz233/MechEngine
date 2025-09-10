//
// Created by MarvelLi on 2023/12/14.
//

#include "Path.h"

Path Path::_ExePath;

#ifndef ENGINE_DIR
#define ENGINE_DIR ""
#endif

#ifndef PROJECT_DIR
#define PROJECT_DIR ""
#endif

Path Path::EngineDir()
{
	return ENGINE_DIR;
}

Path Path::ProjectDir()
{
	return PROJECT_DIR;
}

Path Path::BinPath()
{
	return _ExePath;
}

Path Path::ExePath()
{
	return _ExePath;
}

Path Path::ExecutePath()
{
	return _ExePath;
}

Path Path::EngineContentDir()
{
	return EngineDir() / "Content";
}
Path Path::EngineConfigDir()
{
	return EngineDir() / "Config";
}

Path Path::EnginLogDir()
{
	return EngineDir() / "Log";
}

Path Path::RootDir()
{
	return ProjectDir().parent_path();
}

Path Path::ProjectContentDir()
{
	return ProjectDir() / "Content";
}

Path Path::ProjectConfigDir()
{
	return ProjectDir() / "Config";
}

Path Path::ProjectLogDir()
{
	return ProjectDir() / "Log";
}

Path Path::FileNameWithoutExtension() const
{
	return stem();
}
bool Path::CreateDirectory(const Path& InPath)
{
	std::error_code ErrorCode;
	std::filesystem::create_directory(InPath, ErrorCode);
	return !ErrorCode;
}
void Path::Init(const std::string& BinPath)
{
	if (path(BinPath).is_absolute())
		_ExePath = BinPath;
	else
		_ExePath = std::filesystem::absolute(BinPath);
}