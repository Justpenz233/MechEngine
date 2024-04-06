#pragma once
#include "Components/CameraComponent.h"
#include "../Editor/_generated/serializer/RenderingComponent.serializer.gen.h"


template<>
Json Serializer::write(const CameraComponent& instance);
template<>
CameraComponent& Serializer::read(const Json& json_context, CameraComponent& instance);

