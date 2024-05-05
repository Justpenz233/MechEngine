#pragma once
#include "Log/Logger.h"

namespace MechEngine {
    template <class... Args>
    void LOG_INFO(Args&&... args)
    {
        MechEngine::Logger::Get().GetDefaultLogger()->info(std::forward<Args>(args)...);
    }

    template <class... Args>
    void LOG_ERROR(Args&&... args)
    {
        MechEngine::Logger::Get().GetDefaultLogger()->error(std::forward<Args>(args)...);
    }

	// Log error if the condition value is true
	template <class... Args>
	bool LOG_ERROR_IF(bool Value, Args&&... args)
    {
    	if(Value)
    		LOG_ERROR(std::forward<Args>(args)...);
    	return Value;
    }

    template <class... Args>
    void LOG_WARNING(Args&&... args)
    {
        MechEngine::Logger::Get().GetDefaultLogger()->warn(std::forward<Args>(args)...);
    }

	// Log warning if the condition value is true
	template <class... Args>
	bool LOG_WARNING_IF(bool Value, Args&&... args)
    {
	    if(Value)
	    	LOG_WARNING(std::forward<Args>(args)...);
    	return Value;
    }

    template <class... Args>
    void LOG_DEBUG(Args&&... args)
    {
        MechEngine::Logger::Get().GetDefaultLogger()->debug(std::forward<Args>(args)...);
    }

    template <class... Args>
    void LOG_CRITICAL(Args&&... args)
    {
        MechEngine::Logger::Get().GetDefaultLogger()->critical(std::forward<Args>(args)...);
    }

    template <class... Args>
    void LOG_TEMP(Args&&... args)
    {
        MechEngine::Logger::Get().GetTempLogger()->info(std::forward<Args>(args)...);
    }


    #define ASSERTMSG(Expr, ...) \
        __M_AssertMSG(#Expr, Expr, __FILE__, __LINE__, __VA_ARGS__)

    #define ASSERT(Expr) \
        __M_Assert(#Expr, Expr, __FILE__, __LINE__)

    template<typename... Args>
	void __M_AssertMSG(const char* expr_str, bool expr, const char* file, int line, Args... args)
    {
        if (!expr)
        {
            LOG_CRITICAL(args...);
            LOG_CRITICAL("Assert failed, Expected:\t {0}", expr_str);
            LOG_CRITICAL("At Source:\t {0}, line {1} \n", file, line);
            abort();
        }
    }

	inline void __M_Assert(const char* expr_str, bool expr, const char* file, int line)
    {
        if (!expr)
        {
            LOG_CRITICAL("Assert failed: Expected:\t {0}\n Source:\t\t {1}, line {2} \n", expr_str, file, line);
            abort();
        }
    }

    template <class T>
    std::string ToString(const T& A)
    {
        static_assert(sizeof(T) == -1, "ToString() not implemented");
		return {};
    }
};
