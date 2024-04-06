#pragma once
#include "Components/StaticCurveComponent.h"
#include "../Editor/_generated/serializer/StaticMeshComponent.serializer.gen.h"


template<>
Json Serializer::write(const StaticCurveComponent& instance);
template<>
StaticCurveComponent& Serializer::read(const Json& json_context, StaticCurveComponent& instance);

