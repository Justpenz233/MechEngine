//
// Created by MarvelLi on 2024/4/15.
//
#pragma once
#include <Eigen/Eigen>
#include <spdlog/spdlog.h>
#include "ContainerTypes.h"

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