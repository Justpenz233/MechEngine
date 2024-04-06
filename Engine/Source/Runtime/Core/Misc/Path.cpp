//
// Created by MarvelLi on 2023/12/14.
//

#include "Path.h"

Path Path::_ExePath;
Path Path::_ProjectRootPath;

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
	return _ExePath.parent_path().parent_path().parent_path();
}
Path Path::EngineDir()
{
	return RootDir() / "Engine";
}

Path Path::ProjectDir()
{
	return _ProjectRootPath;
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
void Path::Init(const std::string& BinPath, const std::string& ProjectDir)
{
    _ExePath = BinPath;
	_ProjectRootPath = ProjectDir;
}

