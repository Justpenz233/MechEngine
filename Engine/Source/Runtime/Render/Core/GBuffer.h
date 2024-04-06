//
// Created by MarvelLi on 2024/3/30.
//

#pragma once
#include "luisa/luisa-compute.h"
namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;

class GBuffer
{
	uint2 Resolution;

	Buffer<Image<Float>> Depth;
	Buffer<Image<Float3>> BaseColor;
	Buffer<Image<Float3>> Diffuse;
	Buffer<Image<Float3>> Specular;
	Buffer<Image<Float3>> Normal;
	Buffer<Image<Float>> Roughness;
	Buffer<Image<Float>> Metalness;
	Buffer<Image<Float>> Emissive;
	Buffer<Image<Float>> Opacity;
	Buffer<Image<Float>> AmbientOcclusion;


};


}