//
// Created by MarvelLi on 2023/12/14.
//

#pragma once
#include <spdlog/async_logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Misc/Platform.h"

namespace MechEngine
{
    class ENGINE_API Logger
    {
    public:
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
    	~Logger();
        static Logger& Get();

    	[[nodiscard]] FORCEINLINE std::shared_ptr<spdlog::async_logger>& GetDefaultLogger()
    	{
    		return AsyncLogger;
    	}

    	[[nodiscard]] FORCEINLINE std::shared_ptr<spdlog::logger>& GetTempLogger()
	    {
    		return TempLogger;
    	}

    private:
        Logger();

    	std::shared_ptr<spdlog::async_logger> AsyncLogger;

    	std::shared_ptr<spdlog::logger> TempLogger;

    };
}

