//
// Created by MarvelLi on 2024/4/25.
//

#pragma once
#include "luisa/luisa-compute.h"

namespace MechEngine::Rendering
{
    struct light_data
    {
        //----- Base data for all the lights -------//
        luisa::uint light_type = ~0u;

    	uint instance_id = ~0u;

        float intensity = 1.f;

        // light color in linear space
        luisa::float3 light_color;

        //radius for area light(in disk) and point light
        float radius{};
    };
}

LUISA_STRUCT(MechEngine::Rendering::light_data, light_type, instance_id, intensity, light_color, radius)
{
	luisa::compute::Bool valid()
	{
		return light_type != ~0u & instance_id != ~0u;
	}
};


namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;
	struct ray_intersection;

    /**
     * A common light data structure for all lights.
     * Now support point light, spotlight, directional light, area light
     */
    class light_base
    {
    public:
		virtual ~light_base() = default;

		light_base() = default;

        /**
         * Evaluate the light irradiance at hit_pos in the direction of w_i
         * @param data light data used to evaluate the light
         * @param distance the distance between the light and hit_pos
         * @param w_i the direction to evaluate the light, same as render equation
         * @return the light irradiance at hit_pos in the direction of w_i
         */
        [[nodiscard]] virtual Float3 l_i(Expr<light_data> data, const Float& distance, const Float3& w_i) const = 0;

    	/**
    	* Evaluate the light emission with a direct hit at the light
    	 */
    	[[nodiscard]] virtual Float3 evaluate_hit(Expr<light_data> data, const ray_intersection& intersection, const Float3& w_i) const
    	{ return data.light_color; }

    	// [[nodiscard]] virtual Float3 pdf(Expr<light_data> data, const Float3& distance, const Float3& w_i) = 0;

    };
}