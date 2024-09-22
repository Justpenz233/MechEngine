//
// Created by Mayn on 2024/9/20.
//

#include "independent_sampler.h"
#include "Render/Core/random.h"
#include "luisa/dsl/sugar.h"

namespace MechEngine::Rendering
{

void independent_sampler::init(Expr<uint2> pixel, Expr<uint> sample_index) noexcept
{
	state.emplace(xxhash32(make_uint4(pixel, get_seed(), sample_index)));
}
Float independent_sampler::generate_1d() noexcept
{
	return lcg(*state);
}

Float2 independent_sampler::generate_2d() noexcept
{
	return Float2{ generate_1d(), generate_1d() };
}
}