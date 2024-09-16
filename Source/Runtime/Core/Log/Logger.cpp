//
// Created by MarvelLi on 2023/12/14.
// Edited by Rick Zhang on Sep 05, 2024
//

#include "Log.h"
#include <iomanip>
#include <sstream>
#include <csignal>
#include "Misc/Path.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "Logger.h"

namespace MechEngine
{
	Logger::~Logger()
	{
		AsyncLogger->info("Shut down the logger.");
		spdlog::shutdown();
	}

    Logger& Logger::Get()
    {
        static Logger logger;
        return logger;
    }

    auto GetLogFileName()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
    	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H-%M-%S");
    	std::string FileName = ss.str() + ".log";
    	return (Path::ProjectLogDir()/ FileName).make_preferred();
    }

	inline void CrashWithLog(int signal)
	{
		LOG_TRACE();
		std::exit(signal);
	}

    Logger::Logger()
    {
        size_t q_size = 8192;
        spdlog::init_thread_pool(q_size, 1);
		if(!Path::ProjectLogDir().Existing())
			create_directories(Path::ProjectLogDir());
        auto FileName = GetLogFileName();
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(FileName.string(), true);
        std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};
        AsyncLogger = std::make_shared<spdlog::async_logger>("Log", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::overrun_oldest);
		AsyncLogger->flush_on(spdlog::level::level_enum::info);
		AsyncLogger->set_pattern("[%H:%M:%S] [%n] [%^%l%$] %v");

		AsyncLogger->set_level(spdlog::level::debug); // Set the log level to debug

        AsyncLogger->info("Set default logger, {}", FileName.string());


		TempLogger = spdlog::stdout_color_mt("TempLogger");
		TempLogger->set_pattern("[%H:%M:%S] [%n] [%^%l%$] %v");

		std::signal(SIGSEGV, CrashWithLog);
		std::signal(SIGABRT, CrashWithLog);
		std::signal(SIGFPE, CrashWithLog);
		std::signal(SIGILL, CrashWithLog);
		std::signal(SIGBUS, CrashWithLog);
    }
}
