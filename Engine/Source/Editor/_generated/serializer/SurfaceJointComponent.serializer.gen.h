#pragma once
#include "../../../DevelopableMechanism/Source/SpacialMechanism/SurfaceJointComponent.h"
#include "../Editor/_generated/serializer/JointComponent.serializer.gen.h"


template<>
Json Serializer::write(const SurfaceJointComponent& instance);
template<>
SurfaceJointComponent& Serializer::read(const Json& json_context, SurfaceJointComponent& instance);

