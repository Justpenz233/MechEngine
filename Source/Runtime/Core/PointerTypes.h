//
// Created by MarvelLi on 2024/3/25.
//

#pragma once
#include <memory>

/************************************************************
 *
 * This file responsible for pointer forward declaration.
 *
 ************************************************************/


template <typename T>
using ObjectPtr = std::shared_ptr<T>;

template <typename T>
using SharedPtr = std::shared_ptr<T>;

template <typename T>
using WeakObjectPtr = std::weak_ptr<T>;

template <typename T>
using WeakPtr = std::weak_ptr<T>;

template <typename T>
using UniqueObjectPtr = std::unique_ptr<T>;

template <typename T>
using UniquePtr = std::unique_ptr<T>;

template <typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
ObjectPtr<T> MakeShared(Args&&... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

//---- Cast types ----

template<class T1, class T2>
ObjectPtr<T1> Cast(ObjectPtr<T2> Other)
{
	return std::dynamic_pointer_cast<T1>(Other);
}

template<class T1, class T2>
WeakObjectPtr<T1> Cast(WeakObjectPtr<T2> Other)
{
	if(!Other.expired())
		return std::dynamic_pointer_cast<T1>(Other.lock());
	else
		return {};
}

template<template<typename...> class Pointer, class T1, class T2>
Pointer<T1> Cast(Pointer<T2> Other)
{
	return std::dynamic_pointer_cast<T1>(Other);
}

template<class T1, class T2>
T1* Cast(T2* Other)
{
	return dynamic_cast<T1*>(Other);
}
