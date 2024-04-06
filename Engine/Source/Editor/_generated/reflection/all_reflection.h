#pragma once
#include "Reflection/reflection/reflection.h"
#include "../_generated/serializer/all_serializer.h"
#include "../Editor/_generated/reflection/Actor.reflection.gen.h"
#include "../Editor/_generated/reflection/CameraActor.reflection.gen.h"
#include "../Editor/_generated/reflection/CameraComponent.reflection.gen.h"
#include "../Editor/_generated/reflection/CurveActor.reflection.gen.h"
#include "../Editor/_generated/reflection/FTransform.reflection.gen.h"
#include "../Editor/_generated/reflection/JointComponent.reflection.gen.h"
#include "../Editor/_generated/reflection/LightComponent.reflection.gen.h"
#include "../Editor/_generated/reflection/Object.reflection.gen.h"
#include "../Editor/_generated/reflection/ParametricMeshActor.reflection.gen.h"
#include "../Editor/_generated/reflection/ParametricMeshComponent.reflection.gen.h"
#include "../Editor/_generated/reflection/ParametricSurfaceComponent.reflection.gen.h"
#include "../Editor/_generated/reflection/PointLightActor.reflection.gen.h"
#include "../Editor/_generated/reflection/PointLightComponent.reflection.gen.h"
#include "../Editor/_generated/reflection/RenderingComponent.reflection.gen.h"
#include "../Editor/_generated/reflection/SCAFParametricMeshComponent.reflection.gen.h"
#include "../Editor/_generated/reflection/StaticCurveComponent.reflection.gen.h"
#include "../Editor/_generated/reflection/StaticMeshActor.reflection.gen.h"
#include "../Editor/_generated/reflection/StaticMeshComponent.reflection.gen.h"
#include "../Editor/_generated/reflection/SurfaceJoint.reflection.gen.h"
#include "../Editor/_generated/reflection/SurfaceJointComponent.reflection.gen.h"
#include "../Editor/_generated/reflection/TargetPointsActor.reflection.gen.h"
#include "../Editor/_generated/reflection/TransformComponent.reflection.gen.h"

namespace Reflection{
    void TypeMetaRegister::metaRegister(){
        TypeWrappersRegister::Actor();
        TypeWrappersRegister::CameraActor();
        TypeWrappersRegister::CameraComponent();
        TypeWrappersRegister::CurveActor();
        TypeWrappersRegister::FTransform();
        TypeWrappersRegister::JointComponent();
        TypeWrappersRegister::LightComponent();
        TypeWrappersRegister::Object();
        TypeWrappersRegister::ParametricMeshActor();
        TypeWrappersRegister::ParametricMeshComponent();
        TypeWrappersRegister::ParametricSurfaceComponent();
        TypeWrappersRegister::PointLightActor();
        TypeWrappersRegister::PointLightComponent();
        TypeWrappersRegister::RenderingComponent();
        TypeWrappersRegister::SCAFParametricMeshComponent();
        TypeWrappersRegister::StaticCurveComponent();
        TypeWrappersRegister::StaticMeshActor();
        TypeWrappersRegister::StaticMeshComponent();
        TypeWrappersRegister::SurfaceJoint();
        TypeWrappersRegister::SurfaceJointComponent();
        TypeWrappersRegister::TargetPointsActor();
        TypeWrappersRegister::TransformComponent();
    }
}

