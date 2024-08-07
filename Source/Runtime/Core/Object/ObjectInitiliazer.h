//
// Created by MarvelLi on 2024/1/18.
//

#pragma once
#include <type_traits>
#include <concepts>
class MObjectInitiliazer
{
    // ....
public:
    static MObjectInitiliazer& Get();
};

// The concept have very different behaviours in different platforms, so one condition is enough
template<class T>
concept DefaultConstructible = requires { T{}; } || requires {new T;} || std::is_default_constructible_v<T>;

// if not in MSVC
#ifndef _MSC_VER // The following concept is not supported in MSVC so we skip it for now
#define CheckDefaultConstructible(class_name)\
	static_assert(DefaultConstructible<class_name>, "\n \t MCLASS must be default constructible, add "#class_name"() = default \n");
#else
	#define CheckDefaultConstructible(class_name)\
	static_assert(true, "\n \t MCLASS must be default constructible, add "#class_name"() = default \n");
#endif
template<class T, bool>
class _ObjectDefaultConstructor
{
public:
    static T* _InternalConstructor(const MObjectInitiliazer& Initiliazer)
    {
        return new T();
    }
};

template<class T>
class _ObjectDefaultConstructor<T, false>
{
public:
    static T* _InternalConstructor(const MObjectInitiliazer& Initiliazer)
    {
        return new T(Initiliazer);
    }
};