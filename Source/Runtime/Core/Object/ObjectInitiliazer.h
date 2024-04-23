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

template<class T>
concept DefaultConstructible = requires { T{}; } && requires {new T;};

#define CheckDefaultConstructible(class_name)\
	static_assert(DefaultConstructible<class_name>, "\n \t MCLASS must be default constructible, add "#class_name"() = default \n");

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