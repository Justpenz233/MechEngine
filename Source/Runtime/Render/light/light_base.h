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
	class GpuScene;

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

		light_base(GpuScene& in_scene) : scene(in_scene) {}

		/**
		 * Total emitted power.
		 * This makes it possible to sample lights according to their relative power
		 */
    	// virtual Float3 Phi(Expr<light_data> data) const = 0;


    	/**
    	* Sample the light incident radiance at pos x with shading normal
    	* @param data light data
    	* @param x the position in the lighting surface which receives the light
    	* @param u random number for sampling
    	 */
    	[[nodiscard]] virtual light_li_sample sample_li(Expr<light_data> data, const Float3& x, const Float2& u) const = 0;

    	/**
		* Evaluate the pdf of the light incident radiance at pos x with intersection in light source p_l
		* @param data light data
		* @param x the position in the lighting surface which receives the light
		* @param p_l the position in the light source where the light is emitted
		 */
    	[[nodiscard]] virtual Float pdf_li(Expr<light_data> data, const Float3& x, const Float3& p_l) const = 0;

    	[[nodiscard]] virtual std::pair<Float3, Float> l_i(Expr<light_data> data, const Float3& x, const Float3& p_l) const = 0;

    	/** real time l_i */
		[[nodiscard]] virtual Float3 l_i_rt(Expr<light_data> data, const Float3& x, const Float3& w_i, const Float3& w_o, const Float3& n) const
    	{return make_float3(0.f);}

    	/**
    	* Calculate an intersection point of the light source regardless of occlusion
    	* @param data light data
    	* @param x the position in the lighting surface which receives the light
    	* @param w_i the direction along which light arrives at the point
    	*/
    	// [[nodiscard]] virtual Float3 intersect(Expr<light_data> data, const Float3& x, const Float3& w_i) const = 0;

    protected:
    	const GpuScene& scene;
    };
}