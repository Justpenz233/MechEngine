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

        luisa::float3 intensity;

        // light color in linear space
        luisa::float3 light_color;

    	// size of the light source, maybe radius for point light, size for area light
    	luisa::float2 size;
    };
}

LUISA_STRUCT(MechEngine::Rendering::light_data, light_type, instance_id, intensity, light_color, size)
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
	class RayTracingScene;

	struct light_li_sample
	{
		Float3 l_i; // the amount of radiance leaving the light toward the receiving point, does not include the effect of extinction due to participating media or occlusion
		Float3 w_i; // the direction along which light arrives at the point
		Float3 p_l; // position of where the light ray emitted
		Float pdf; // probability density of this sample
	};

    /**
     * A common light data structure for all lights.
     * Now support point light, spotlight, directional light, area light
     */
    class light_base
    {
    public:
		virtual ~light_base() = default;

		light_base(RayTracingScene& in_scene) : scene(in_scene) {}

		/**
		 * Total emitted power.
		 * This makes it possible to sample lights according to their relative power
		 */
    	// virtual Float3 Phi(Expr<light_data> data) const = 0;


    	/**
    	* Sample the light incident radiance at pos x with shading normal
    	* @param data light data
    	* @param x the position in the lighting surface which receives the light
    	* @param normal the shading normal at x
    	* @param u random number for sampling
    	 */
    	[[nodiscard]] virtual light_li_sample sample_li(Expr<light_data> data, const Float3& x, const Float3& normal, const Float2& u) const = 0;


    	/**
    	* Evaluate the pdf of the light incident radiance at pos x with shading normal
    	* Used for MIS, when already sampled a ray direction to the light and there is no occlusion in between
    	* @param data light data
    	* @param x the position in the lighting surface which receives the light
    	* @param normal the shading normal at x
    	* @param w_i the direction
    	 */
    	// [[nodiscard]] virtual Float pdf_li(Expr<light_data> data, const Float3& x, const Float3& normal, const Float3& w_i) const = 0;


    	/**
    	* If a ray happens to intersect an area light source, it is necessary to find the radiance that is emitted back along the ray
    	* takes local information about the intersection point and the outgoing direction.
    	* This method should never be called for any light that does not have geometry associated with it.
    	* @param data light data
    	* @param p_l the position of the light where the ray hit
    	* @param n_l the normal of the light where the ray hit
    	* @param w_o the direction of the ray
    	 */
    	[[nodiscard]] virtual Float3 l(Expr<light_data> data, const Float3& p_l, const Float3& n_l, const Float3& w_o) const
    	{
    		return make_float3(0.f);
    	}

		/**
		 * Enables infinite area lights to contribute radiance to rays that do not hit any geometry in the scene
		 * This method should only be called for lights without geometry associated with them.
		 * @param data light data
		 * @param ray the ray that did not hit any geometry
		 */
		[[nodiscard]] virtual Float3 l_e(Expr<light_data> data, const Ray& ray) const
    	{
    		return make_float3(0.f);
    	}

    protected:
    	const RayTracingScene& scene;
    };
}