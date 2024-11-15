//
// Created by MarvelLi on 2024/11/12.
//
#pragma	once
#include <luisa/luisa-compute.h>
namespace MechEngine::Rendering
{

using namespace luisa::compute;
struct light_sample
{
	uint index; // index of the light
	float3 l_i; // the amount of radiance leaving the light toward the receiving point, does not include the effect of extinction due to participating media or occlusion
	float3 w_i; // the direction along which light arrives at the point
	float3 p_l; // position of where the light ray emitted
	float pdf; // probability density of this sample
};
};

LUISA_STRUCT(Rendering::light_sample, index, l_i, w_i, p_l, pdf) {};