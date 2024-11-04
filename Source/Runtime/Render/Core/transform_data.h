//
// Created by MarvelLi on 2024/5/12.
//

#pragma once
#include "luisa/luisa-compute.h"

namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;

struct transform_data
{
	luisa::float4x4 transform_matrix{};
	luisa::float4x4 inverse_transform_matrix{};

	luisa::float4x4 last_transform_matrix{}; // Last frame's transform matrix, used for motion vector calculation

	luisa::float4 rotation_quaternion{};
	luisa::float3 scale{};
};
}

LUISA_STRUCT(MechEngine::Rendering::transform_data, transform_matrix, inverse_transform_matrix, last_transform_matrix, rotation_quaternion, scale)
{
	[[nodiscard]] luisa::compute::Float3 get_location() const noexcept
	{
		return transform_matrix[3].xyz();
	}

	[[nodiscard]] luisa::compute::Float3 get_scale() const noexcept
	{
		return scale;
	}

	[[nodiscard]] luisa::compute::Float4x4 get_matrix() const noexcept
	{
		return transform_matrix;
	}

	[[nodiscard]] luisa::compute::Float4x4 get_inverse_matrix() const noexcept
	{
		return inverse_transform_matrix;
	}


};
