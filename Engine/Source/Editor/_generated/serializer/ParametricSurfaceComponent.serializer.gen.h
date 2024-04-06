#pragma once
#include "Components/ParametricSurfaceComponent.h"
#include "../Editor/_generated/serializer/ParametricMeshComponent.serializer.gen.h"


template<>
Json Serializer::write(const ParametricSurfaceComponent& instance);
template<>
ParametricSurfaceComponent& Serializer::read(const Json& json_context, ParametricSurfaceComponent& instance);

