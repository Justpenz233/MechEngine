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
	TMap<String, std::pair<mINI::INIFile, mINI::INIStructure>> ConfigFiles;

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


protected:
	template <typename T>
	T Converter(const std::string& s) const;
	[[nodiscard]] const bool BoolConverter(std::string s) const;
};

template <typename T>
T Config::Get(const String& Section, const String& Key)
{
	for(const auto& [FileName, InI] : ConfigFiles)
	{
		if(InI.second.has(Section) && InI.second.get(Section).has(Key))
		{
			auto value = InI.second.get(Section).get(Key);
			if constexpr (std::is_same<T, std::string>()) {
				return value;
			} else if constexpr (std::is_same<T, bool>()) {
				return BoolConverter(value);
			} else {
				return Converter<T>(value);
			};
		}
	}
	LOG_ERROR("Section {} , Key {} not found in config", Section, Key);
	return {};
}

template <typename T>
TArray<T> Config::GetArray(const String& Section, const String& Key)
{
	std::string value = Get<std::string>(Section, Key);

	std::istringstream out{value};
	const std::vector<std::string> strs{std::istream_iterator<std::string>{out},
		std::istream_iterator<std::string>()};

	std::vector<T> vs{};
	for (const std::string& s : strs) {
		vs.emplace_back(Converter<T>(s));
	}
	return vs;
}

template <typename T>
inline T Config::Converter(const std::string& s) const {
	try {
		T v{};
		std::istringstream _{s};
		_.exceptions(std::ios::failbit);
		_ >> v;
		return v;
	} catch (std::exception& e) {
		throw std::runtime_error("cannot parse value '" + s + "' to type<T>.");
	};
}

inline const bool Config::BoolConverter(std::string s) const {
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	static const std::unordered_map<std::string, bool> s2b{
		{"1", true},  {"true", true},   {"yes", true}, {"on", true},
		{"0", false}, {"false", false}, {"no", false}, {"off", false},
	};
	auto const value = s2b.find(s);
	if (value == s2b.end()) {
		throw std::runtime_error("'" + s + "' is not a valid boolean value.");
	}
	return value->second;
}


}

// Global config file, by dafault at least contains config files from Path::EngineConfigDir() and Path::ProjectConfigDir()
extern ENGINE_API MechEngine::Config GConfig;