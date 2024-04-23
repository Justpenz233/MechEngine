//
// Created by MarvelLi on 2023/12/14.
//

#pragma once
#include <spdlog/async_logger.h>

namespace MechEngine
{
    class Logger
    {
    public:
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
    	~Logger();
        static Logger& Get();

    	[[nodiscard]] std::shared_ptr<spdlog::async_logger>& GetDefaultLogger()
    	{
    		return AsyncLogger;
    	}

    private:
        Logger();

    	std::shared_ptr<spdlog::async_logger> AsyncLogger;


    };
}

