//
// Created by MarvelLi on 2024/9/24.
//

#pragma once

namespace MechEngine::Rendering
{
struct shape
{
	uint mesh_id = ~0u;
    uint light_id = ~0u;
};
};


LUISA_STRUCT(MechEngine::Rendering::shape, mesh_id, light_id)
{
    [[nodiscard]] auto is_light() const noexcept { return light_id != ~0u; }
	[[nodiscard]] auto is_surface() const noexcept { return mesh_id != ~0u;  }
};