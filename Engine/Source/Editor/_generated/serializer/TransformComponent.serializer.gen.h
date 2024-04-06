#pragma once
#include "Components/TransformComponent.h"


template<>
Json Serializer::write(const TransformComponent& instance);
template<>
TransformComponent& Serializer::read(const Json& json_context, TransformComponent& instance);

