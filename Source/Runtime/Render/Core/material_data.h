//
// Created by MarvelLi on 2024/4/29.
//
#pragma once
#include <luisa/luisa-compute.h>
#include "Render/Core/TypeConvertion.h"
#include "Materials/Material.h"

namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;

    struct materialData
    {
        uint material_type = ~0u;
    	uint normal_type; // 0: triangle normal, 1: vertex normal, 2: corner normal
        uint show_wireframe = false;
        float3 base_color;
        float metalness;
        float specular;
        float roughness;
        float3 specular_tint;


        materialData() = default;
        materialData(uint Tag, ::Material* InMaterial) :
        show_wireframe(InMaterial->bShowWireframe),
        material_type(Tag),
        base_color(ToLuisaVector(InMaterial->BaseColor)),
        specular_tint(ToLuisaVector(InMaterial->SpecularTint)),
        metalness(InMaterial->Metalness),
        specular(InMaterial->Specular),
        roughness(InMaterial->Roughness),
    	normal_type(static_cast<uint>(InMaterial->NormalType))
        {}
    };
}
LUISA_STRUCT(MechEngine::Rendering::materialData,
    material_type, normal_type, show_wireframe,
    base_color, metalness, specular, roughness, specular_tint){};
