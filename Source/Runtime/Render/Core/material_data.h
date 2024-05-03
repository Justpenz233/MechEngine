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
        uint show_wireframe = false;
        float3 base_color;
        float metalness;
        float roughness;
        float3 specular_tint;


        bool bUseTriangleNormal = false;
        bool bUseVertexNormal = true;

        materialData() = default;
        materialData(uint Tag, ::Material* InMaterial) :
        show_wireframe(InMaterial->bShowWireframe),
        material_type(Tag),
        base_color(ToLuisaVector(InMaterial->BaseColor)),
        specular_tint(ToLuisaVector(InMaterial->SpecularTint)),
        metalness(InMaterial->Metalness),
        roughness(InMaterial->Roughness),
        bUseTriangleNormal(InMaterial->NormalType == FaceNormal),
        bUseVertexNormal(InMaterial->NormalType == VertexNormal)
        {}
    };
}
LUISA_STRUCT(MechEngine::Rendering::materialData,
    material_type, show_wireframe,
    base_color, metalness, roughness, specular_tint, bUseTriangleNormal, bUseVertexNormal){};
