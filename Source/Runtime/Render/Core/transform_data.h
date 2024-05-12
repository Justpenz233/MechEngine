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
	luisa::float4x4 transformMatrix{};
	luisa::float4 rotationQuaternion{};
	luisa::float3 scale{};
};
}

LUISA_STRUCT(MechEngine::Rendering::transform_data, transformMatrix, rotationQuaternion, scale)
{
	[[nodiscard]] luisa::compute::Float3 get_location() const noexcept
	{
		return transformMatrix[3].xyz();
	}

	[[nodiscard]] luisa::compute::Float3 get_scale() const noexcept
	{
		return scale;
	}

	[[nodiscard]] luisa::compute::Float4x4 get_matrix() const noexcept
	{
		return transformMatrix;
	}

};
