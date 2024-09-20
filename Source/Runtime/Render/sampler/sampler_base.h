//
// Created by Mayn on 2024/9/20.
//

#pragma once
#include <luisa/dsl/syntax.h>
namespace MechEngine::Rendering
{

using luisa::compute::Float;
using luisa::compute::Float2;
using luisa::compute::uint;
using luisa::compute::Expr;
using luisa::uint2;

class sampler_base
{
public:
	virtual ~sampler_base() = default;
	// interfaces
	// virtual void reset(CommandBuffer& command_buffer, uint2 resolution, uint state_count, uint spp) noexcept = 0;
	virtual void init(Expr<uint2> pixel, Expr<uint> sample_index) noexcept = 0;
	// virtual void save_state(Expr<uint> state_id) noexcept = 0;
	// virtual void load_state(Expr<uint> state_id) noexcept = 0;
	/**
	 * Generate a random float in [0, 1]
	 */
	[[nodiscard]] virtual Float generate_1d() noexcept = 0;

	/**
	 * Generate a random float2 in [0, 1]
	 */
	[[nodiscard]] virtual Float2 generate_2d() noexcept = 0;

	/**
	 * Get the seed value of the sampler
	 */
	[[nodiscard]] auto get_seed() const noexcept { return seed; };

private:
	// Hardcoded seed value
	uint seed = 19990718u;
};
};