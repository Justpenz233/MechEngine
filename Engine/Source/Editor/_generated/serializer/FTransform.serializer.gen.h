#pragma once
#include "Core/Math/FTransform.h"


template<>
Json Serializer::write(const FTransform& instance);
template<>
FTransform& Serializer::read(const Json& json_context, FTransform& instance);

