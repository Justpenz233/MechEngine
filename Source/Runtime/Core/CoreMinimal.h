//
// Created by MarvelLi on 2023/8/24.
//

#pragma once

#include "Log/Log.h"
#include "Log/LogFormatter.h"

#include "Object/Object.h"
class Object;
class Actor;
class World;
class MObjectInitiliazer;

template<class T, bool>
class _ObjectDefaultConstructor;


template<typename R, typename... Args>
class Delegate;

template<class T, typename R, typename... Args>
class ObjectDelegate;

template<typename TLambda, typename R, typename... Args>
class LambdaDelegate;

template<typename... Args>
class MultiCastDelegate;

using namespace MechEngine;

//---- Core classes ----
class Object;
class MObjectInitiliazer;

class Actor;
class ActorComponent;

//---- Math types ----
#include "Math/MathType.h"

//---- Container types ----
class Path;
class Config;

#include "ContainerTypes.h"


// ---- Pointer types ----
#include "PointerTypes.h"

#include "igl/parallel_for.h"
#define ParallelFor(...) \
	igl::parallel_for(__VA_ARGS__);

#include "Misc/Platform.h"