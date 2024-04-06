#pragma once
#include "Components/RenderingComponent.h"


template<>
Json Serializer::write(const RenderingComponent& instance);
template<>
RenderingComponent& Serializer::read(const Json& json_context, RenderingComponent& instance);

