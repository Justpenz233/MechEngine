#pragma once
#include "Components/LightComponent.h"
#include "../Editor/_generated/serializer/RenderingComponent.serializer.gen.h"


template<>
Json Serializer::write(const LightComponent& instance);
template<>
LightComponent& Serializer::read(const Json& json_context, LightComponent& instance);

