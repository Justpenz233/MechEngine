#pragma once
#include "Components/SCAFParametricMeshComponent.h"
#include "../Editor/_generated/serializer/ParametricMeshComponent.serializer.gen.h"


template<>
Json Serializer::write(const SCAFParametricMeshComponent& instance);
template<>
SCAFParametricMeshComponent& Serializer::read(const Json& json_context, SCAFParametricMeshComponent& instance);

