//
// Created by MarvelLi on 2024/11/12.
//

#pragma once
#include "Render/sampler/light_sampler.h"

#include <luisa/luisa-compute.h>
namespace MechEngine::Rendering
{
using namespace luisa::compute;

struct reservoir_sample
{
	light_sample sample;
	float weight;
};

};

LUISA_STRUCT(Rendering::reservoir_sample, sample, weight) {};


namespace MechEngine::Rendering
{
struct ris_reservoir
{
	// static constexpr int reservoir_size = 1; // Now we only support 1 sample
	float sum_weight = 0.0f;
	reservoir_sample out;
};
}

LUISA_STRUCT(MechEngine::Rendering::ris_reservoir, sum_weight, out)
{
	// Add a sample to the reservoir and return the out sample
	luisa::compute::Var<Rendering::reservoir_sample>
	add_sample(
		luisa::compute::Expr<Rendering::reservoir_sample> in_sample,
		const luisa::compute::Float& u)
	{
		$if(in_sample.weight > 0.0f)
		{
			sum_weight += in_sample.weight;
			luisa::compute::Float p = in_sample.weight / sum_weight;
			$if(u < p)
			{
				out = in_sample;
			};
		};
		return out;
	}
};