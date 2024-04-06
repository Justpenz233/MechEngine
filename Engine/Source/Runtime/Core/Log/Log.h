#pragma once
#include <spdlog/spdlog.h>
#include <Eigen/Eigen>
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

template <>
struct fmt::formatter<Eigen::Vector3d> : fmt::formatter<std::string>
{
	auto format(const Eigen::Vector3d& a, format_context& ctx) const {
        std::string Context = fmt::format("[{0}, {1}, {2}]", a[0], a[1], a[2]);
        return fmt::formatter<std::string>::format(Context, ctx);
  }
};
template <>
struct fmt::formatter<Eigen::Vector2d> : fmt::formatter<std::string>
{
    auto format(const Eigen::Vector2d& a, format_context& ctx) const {
        std::string Context = fmt::format("[{0}, {1}]", a[0], a[1]);
        return fmt::formatter<std::string>::format(Context, ctx);
    }
};
template <>
struct fmt::formatter<Eigen::Quaterniond> : fmt::formatter<std::string>
{
	auto format(const Eigen::Quaterniond& a, format_context& ctx) const {
        std::string Context = fmt::format("({0}, {1}, {2}, {3})", a.x(), a.y(), a.z(), a.w());
        return fmt::formatter<std::string>::format(Context, ctx);
  }
};

template <>
struct fmt::formatter<Eigen::MatrixXd> : fmt::formatter<std::string>
{
	auto format(const Eigen::MatrixXd& a, format_context& ctx) const {
        std::ostringstream stream;
        stream << a;
        std::string Context = fmt::format("\n{}", stream.str());
        return fmt::formatter<std::string>::format(Context, ctx);
  }
};

template <>
struct fmt::formatter<Eigen::Matrix4d> : fmt::formatter<std::string>
{
	auto format(const Eigen::Matrix4d& a, format_context& ctx) const {
		std::ostringstream stream;
		stream << a;
		std::string Context = fmt::format("\n{}", stream.str());
		return fmt::formatter<std::string>::format(Context, ctx);
	}
};

template <>
struct fmt::formatter<Eigen::VectorXd> : fmt::formatter<std::string>
{
	auto format(const Eigen::VectorXd& a, format_context& ctx) const {
        std::ostringstream stream;
        stream << a;
        std::string Context = fmt::format("{}", stream.str());
        std::replace(Context.begin(), Context.end(), '\n', ' ');
        return fmt::formatter<std::string>::format(Context, ctx);
  }
};

template <>
struct fmt::formatter<Eigen::RowVectorXd> : fmt::formatter<std::string>
{
    auto format(const Eigen::RowVectorXd& a, format_context& ctx) const {
        std::ostringstream stream;
        stream << a;
        std::string Context = fmt::format("{}", stream.str());
        return fmt::formatter<std::string>::format(Context, ctx);
    }
};

// Format for std::vector, with limited number of 20 elements
// if the vector has more than 20 elements, the last element will be "..."
template <typename T>
struct fmt::formatter<std::vector<T>> : fmt::formatter<std::string> {
	// if the vector has more than 20 elements, the last element will be "..."
	auto format(const std::vector<T>& a, format_context& ctx) {
		std::string Context = "[";
		int Count = 0;
		int EndIndex = a.size() > 20 ? 20 : a.size() - 1;
		for (auto& i : a)
		{
			Context += fmt::format("{}", i);
			if(Count != EndIndex)
				Context += ", ";
			else if (EndIndex < a.size() - 1)
			{
				Context += "...";
				break;
			}
			Count ++;
		}
		Context += "]";
		return fmt::formatter<std::string>::format(Context, ctx);
	}
};