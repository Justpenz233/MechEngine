//
// Created by MarvelLi on 2024/3/25.
//

#pragma once

/************************************************************
 *
 * This file responsible for container types forward declaration.
 *
 ************************************************************/


#include <string>
using String = std::string;

#include <vector>
template <typename T>
using TArray = std::vector<T>;

#include <map>
template <typename K, typename V>
using TMap = std::map<K, V>;

template <typename K, typename V>
using TMultiMap = std::multimap<K, V>;

#include <unordered_map>
template <typename K, typename V>
using THashMap = std::unordered_map<K, V>;

#include <set>
template <typename T>
using TSet = std::set<T>;

#include <unordered_set>
template <typename T>
using THashSet = std::unordered_set<T>;

#include <functional>
template <typename T>
using TFunction = std::function<T>;
