//
// Created by Mayn on 2024/9/20.
//

#pragma once
#include "sampler_base.h"

namespace MechEngine::Rendering
{
using luisa::compute::UInt;

// A sampler that generates random numbers independently
class independent_sampler : public sampler_base
{
protected:
	luisa::optional<UInt> state;

public:
	using sampler_base::sampler_base;
	virtual ~independent_sampler() = default;

	virtual void init(Expr<uint2> pixel, Expr<uint> sample_index) noexcept override;
	virtual Float generate_1d() noexcept override;
	virtual Float2 generate_2d() noexcept override;
};

}
