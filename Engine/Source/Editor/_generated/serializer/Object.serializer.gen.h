#pragma once
#include "Core/Object/Object.h"


template<>
Json Serializer::write(const Object& instance);
template<>
Object& Serializer::read(const Json& json_context, Object& instance);

