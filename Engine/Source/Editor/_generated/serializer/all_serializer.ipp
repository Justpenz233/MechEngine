#pragma once
#include "../Editor/_generated/serializer/PointLightComponent.serializer.gen.h"
#include "../Editor/_generated/serializer/RenderingComponent.serializer.gen.h"
#include "../Editor/_generated/serializer/PointLightActor.serializer.gen.h"
#include "../Editor/_generated/serializer/FTransform.serializer.gen.h"
#include "../Editor/_generated/serializer/TransformComponent.serializer.gen.h"
#include "../Editor/_generated/serializer/LightComponent.serializer.gen.h"
#include "../Editor/_generated/serializer/Actor.serializer.gen.h"
#include "../Editor/_generated/serializer/Object.serializer.gen.h"

template<>
Json Serializer::write(const RenderingComponent& instance){
    Json::object  ret_context;
    auto&&  json_context_0 = Serializer::write(*(ActorComponent*)&instance);
    assert(json_context_0.is_object());
    auto&& json_context_map_0 = json_context_0.object_items();
    ret_context.insert(json_context_map_0.begin() , json_context_map_0.end());
    ret_context.insert_or_assign("bCastShadow", Serializer::write(instance.bCastShadow));
    ret_context.insert_or_assign("bVisible", Serializer::write(instance.bVisible));
    return  Json(ret_context);
}
template<>
RenderingComponent& Serializer::read(const Json& json_context, RenderingComponent& instance){
    assert(json_context.is_object());
    Serializer::read(json_context,*(ActorComponent*)&instance);
    if(!json_context["bCastShadow"].is_null()){
        Serializer::read(json_context["bCastShadow"], instance.bCastShadow);
    }
    if(!json_context["bVisible"].is_null()){
        Serializer::read(json_context["bVisible"], instance.bVisible);
    }
    return instance;
}
template<>
Json Serializer::write(const FTransform& instance){
    Json::object  ret_context;
    
    ret_context.insert_or_assign("Translation", Serializer::write(instance.Translation));
    ret_context.insert_or_assign("Rotation", Serializer::write(instance.Rotation));
    ret_context.insert_or_assign("Scale", Serializer::write(instance.Scale));
    return  Json(ret_context);
}
template<>
FTransform& Serializer::read(const Json& json_context, FTransform& instance){
    assert(json_context.is_object());
    
    if(!json_context["Translation"].is_null()){
        Serializer::read(json_context["Translation"], instance.Translation);
    }
    if(!json_context["Rotation"].is_null()){
        Serializer::read(json_context["Rotation"], instance.Rotation);
    }
    if(!json_context["Scale"].is_null()){
        Serializer::read(json_context["Scale"], instance.Scale);
    }
    return instance;
}
template<>
Json Serializer::write(const TransformComponent& instance){
    Json::object  ret_context;
    auto&&  json_context_0 = Serializer::write(*(ActorComponent*)&instance);
    assert(json_context_0.is_object());
    auto&& json_context_map_0 = json_context_0.object_items();
    ret_context.insert(json_context_map_0.begin() , json_context_map_0.end());
    ret_context.insert_or_assign("Transform", Serializer::write(instance.Transform));
    return  Json(ret_context);
}
template<>
TransformComponent& Serializer::read(const Json& json_context, TransformComponent& instance){
    assert(json_context.is_object());
    Serializer::read(json_context,*(ActorComponent*)&instance);
    if(!json_context["Transform"].is_null()){
        Serializer::read(json_context["Transform"], instance.Transform);
    }
    return instance;
}
template<>
Json Serializer::write(const LightComponent& instance){
    Json::object  ret_context;
    auto&&  json_context_0 = Serializer::write(*(RenderingComponent*)&instance);
    assert(json_context_0.is_object());
    auto&& json_context_map_0 = json_context_0.object_items();
    ret_context.insert(json_context_map_0.begin() , json_context_map_0.end());
    ret_context.insert_or_assign("Intensity", Serializer::write(instance.Intensity));
    ret_context.insert_or_assign("LightColor", Serializer::write(instance.LightColor));
    ret_context.insert_or_assign("SamplesPerPixel", Serializer::write(instance.SamplesPerPixel));
    return  Json(ret_context);
}
template<>
LightComponent& Serializer::read(const Json& json_context, LightComponent& instance){
    assert(json_context.is_object());
    Serializer::read(json_context,*(RenderingComponent*)&instance);
    if(!json_context["Intensity"].is_null()){
        Serializer::read(json_context["Intensity"], instance.Intensity);
    }
    if(!json_context["LightColor"].is_null()){
        Serializer::read(json_context["LightColor"], instance.LightColor);
    }
    if(!json_context["SamplesPerPixel"].is_null()){
        Serializer::read(json_context["SamplesPerPixel"], instance.SamplesPerPixel);
    }
    return instance;
}
template<>
Json Serializer::write(const Object& instance){
    Json::object  ret_context;
    
    ret_context.insert_or_assign("ObjectName", Serializer::write(instance.ObjectName));
    return  Json(ret_context);
}
template<>
Object& Serializer::read(const Json& json_context, Object& instance){
    assert(json_context.is_object());
    
    if(!json_context["ObjectName"].is_null()){
        Serializer::read(json_context["ObjectName"], instance.ObjectName);
    }
    return instance;
}

