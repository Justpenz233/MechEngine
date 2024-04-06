#pragma once
#include "Components/StaticMeshComponent.h"
#include "../Editor/_generated/serializer/RenderingComponent.serializer.gen.h"


template<>
Json Serializer::write(const StaticMeshComponent& instance);
template<>
StaticMeshComponent& Serializer::read(const Json& json_context, StaticMeshComponent& instance);

