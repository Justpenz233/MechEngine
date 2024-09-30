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

    struct material_data
    {
        uint material_type = ~0u;
    	uint normal_type; // 0: triangle normal, 1: vertex normal, 2: corner normal
        uint show_wireframe = false;
    	float alpha;
        float3 base_color;
        float metalness;
        float specular;
        float roughness;
        float3 specular_tint;


        material_data() = default;
        material_data(uint Tag, const ::Material* InMaterial) :
        show_wireframe(InMaterial->bShowWireframe),
        material_type(Tag),
    	alpha(InMaterial->Alpha),
        base_color(ToLuisaVector(InMaterial->BaseColor)),
        specular_tint(ToLuisaVector(InMaterial->SpecularTint)),
        metalness(InMaterial->Metalness),
        specular(InMaterial->Specular),
        roughness(InMaterial->Roughness),
    	normal_type(static_cast<uint>(InMaterial->NormalType))
        {}
    };
}
LUISA_STRUCT(MechEngine::Rendering::material_data,
    material_type, normal_type, show_wireframe,
    alpha, base_color, metalness, specular, roughness, specular_tint){};
