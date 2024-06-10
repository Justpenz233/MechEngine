#pragma once
#include "Reflection/json.h"
#include "Reflection/reflection/reflection.h"

#include <cassert>

template<typename...>
inline constexpr bool always_false = false;

class Serializer
{
public:
    template<typename T>
    static Json writePointer(T* instance)
    {
        return Json::object {{"$typeName", Json {"*"}}, {"$context", Serializer::write(*instance)}};
    }

    template<typename T>
    static T*& readPointer(const Json& json_context, T*& instance)
    {
        assert(instance == nullptr);
        std::string type_name = json_context["$typeName"].string_value();
        assert(!type_name.empty());
        if ('*' == type_name[0])
        {
            instance = new T;
            read(json_context["$context"], *instance);
        }
        else
        {
            instance = static_cast<T*>(
                Reflection::TypeMeta::newFromNameAndJson(type_name, json_context["$context"]).m_instance);
        }
        return instance;
    }

    template<typename T>
    static Json write(const Reflection::ReflectionPtr<T>& instance)
    {
        T*          instance_ptr = static_cast<T*>(instance.operator->());
        std::string type_name    = instance.getTypeName();
        return Json::object {{"$typeName", Json(type_name)},
                              {"$context", Reflection::TypeMeta::writeByName(type_name, instance_ptr)}};
    }

    template<typename T>
    static T*& read(const Json& json_context, Reflection::ReflectionPtr<T>& instance)
    {
        std::string type_name = json_context["$typeName"].string_value();
        instance.setTypeName(type_name);
        return readPointer(json_context, instance.getPtrReference());
    }

    template<typename T>
    static Json write(const T& instance)
    {

        if constexpr (std::is_pointer<T>::value)
        {
            return writePointer((T)instance);
        }
        else
        {
            // static_assert(always_false<T>, "Serializer::write<T> has not been implemented yet!");
            return Json();
        }
    }

    template<typename T>
    static T& read(const Json& json_context, T& instance)
    {
        // if constexpr (std::is_pointer<T>::value)
        // {
        //     return readPointer(json_context, instance);
        // }
        // else
        {
            // static_assert(always_false<T>, "Serializer::read<T> has not been implemented yet!");
            return instance;
        }
    }
};

template<>
inline Json Serializer::write(const char& instance)
{
	return {instance};
}
template<>
inline char& Serializer::read(const Json& json_context, char& instance)
{
	assert(json_context.is_number());
	return instance = json_context.number_value();
}

template<>
inline Json Serializer::write(const int& instance)
{
	return {instance};
}
template<>
inline int& Serializer::read(const Json& json_context, int& instance)
{
	assert(json_context.is_number());
	return instance = static_cast<int>(json_context.number_value());
}

template<>
inline Json Serializer::write(const unsigned int& instance)
{
	return {static_cast<int>(instance)};
}
template<>
inline unsigned int& Serializer::read(const Json& json_context, unsigned int& instance)
{
	assert(json_context.is_number());
	return instance = static_cast<unsigned int>(json_context.number_value());
}

template<>
inline Json Serializer::write(const float& instance)
{
	return {instance};
}
template<>
inline float& Serializer::read(const Json& json_context, float& instance)
{
	assert(json_context.is_number());
	return instance = static_cast<float>(json_context.number_value());
}

template<>
inline Json Serializer::write(const double& instance)
{
	return {instance};
}
template<>
inline double& Serializer::read(const Json& json_context, double& instance)
{
	assert(json_context.is_number());
	return instance = static_cast<float>(json_context.number_value());
}

template<>
inline Json Serializer::write(const bool& instance)
{
	return {instance};
}
template<>
inline bool& Serializer::read(const Json& json_context, bool& instance)
{
	assert(json_context.is_bool());
	return instance = json_context.bool_value();
}

template<>
inline Json Serializer::write(const std::string& instance)
{
	return {instance};
}
template<>
inline std::string& Serializer::read(const Json& json_context, std::string& instance)
{
	assert(json_context.is_string());
	return instance = json_context.string_value();
}