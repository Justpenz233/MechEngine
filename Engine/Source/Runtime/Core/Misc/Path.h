//
// Created by MarvelLi on 2023/12/14.
//

#pragma once
#include "CoreMinimal.h"
#include <filesystem>

class Path : public std::filesystem::path
{
public:
	// Use constructor from Path
	template <typename... T>
	Path(T&&... Args) : std::filesystem::path(std::forward<T>(Args)...) {}

	Path() = default;
	~Path() = default;

	static Path BinPath();
	static Path ExePath();
    static Path ExecutePath();

	// Returns the root directory of the engine directory tree. Parent of the Engine directory.
    static Path RootDir();

	// Path of the engine directory which contains the engine content, config, source, and other directories.
	static Path EngineDir();

	// Path of user project root directory, which contains the project content, config, source, and other directories.
	static Path ProjectDir();

	static Path ProjectContentDir();
	static Path ProjectConfigDir();
	static Path ProjectLogDir();

	static Path EngineContentDir();
	static Path EngineConfigDir();
	static Path EnginLogDir();

	FORCEINLINE static Path ContentDir();

	Path FileNameWithoutExtension() const;

	FORCEINLINE static bool IsDirectory(const Path& InPath);
	FORCEINLINE bool IsDirectory() const;

	FORCEINLINE static bool Existing(const Path& InPath);

	FORCEINLINE bool Existing() const;

	template <class T>
	static auto DirectoryIterator(const T& InPath)
	{
		return std::filesystem::directory_iterator(InPath);
	}

private:
    static Path _ExePath;
	static Path _ProjectRootPath;
    static void Init(const std::string& BinPath, const std::string& ProjectDir);
    friend class Editor;
};

FORCEINLINE Path Path::ContentDir()
{
	return ProjectContentDir();
}

FORCEINLINE bool Path::IsDirectory() const
{
	return Path::IsDirectory(*this);
}

FORCEINLINE bool Path::IsDirectory(const Path& InPath)
{
	return std::filesystem::is_directory(InPath);
}

FORCEINLINE bool Path::Existing(const Path& InPath)
{
	return std::filesystem::exists(InPath);
}

FORCEINLINE bool Path::Existing() const
{
	return std::filesystem::exists(*this);
}