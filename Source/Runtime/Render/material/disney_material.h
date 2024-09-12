//
// Created by MarvelLi on 2024/4/26.
//

#pragma once

#include "shader_base.h"
#include "shading_function.h"

namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;
    /**
     * Disney's physically based shading model
     * @see https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf
     */
    class disney_material : public shader_base
    {
        using shader_base::shader_base;

        static FORCEINLINE Float sqr(const Float& x) { return x*x; }

        static Float SchlickFresnel(const Float& u)
        {
            Float m = clamp(1.f-u, 0.f, 1.f);
            Float m2 = m*m;
            return m2*m2*m;
        }

        static FORCEINLINE Float GTR1(const Float& NdotH, const Float& a)
        {
            auto a2 = a*a;
            auto t = 1.f + (a2-1.f)*NdotH*NdotH;
            return select(1.f / pi, (a2-1.f) / (pi*log(a2)*t), a >= 1.f);
        }

        static FORCEINLINE Float GTR2(const Float& NdotH, const Float& a)
        {
            Float a2 = a*a;
            Float t = 1.f + (a2-1.f)*NdotH*NdotH;
            return a2 / (pi * t*t);
        }

        static FORCEINLINE Float GTR2_aniso(const Float& NdotH, const Float& HdotX, const Float& HdotY, const Float& ax, const Float& ay)
        {
            return 1.f / (pi * ax*ay * sqr( sqr(HdotX/ax) + sqr(HdotY/ay) + NdotH*NdotH ));
        }

        static FORCEINLINE Float smithG_GGX(const Float& NdotV, const Float& alphaG)
        {
            Float a = alphaG*alphaG;
            Float b = NdotV*NdotV;
            return 1.f / (NdotV + sqrt(a + b - a*b));
        }

        static Float smithG_GGX_aniso(const Float& NdotV, const Float& VdotX, const Float& VdotY, const Float& ax, const Float& ay)
        {
            return 1.f / (NdotV + sqrt( sqr(VdotX*ax) + sqr(VdotY*ay) + sqr(NdotV) ));
        }

        static Float3 mon2lin(const Float3& x)
        {
            return pow(x, 2.2f);
        }

    public:
        /**
         * disney's physically based shading model
         * @see https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf
         * @see https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
         */
        [[nodiscard]]
        virtual Float3 evaluate(const material_parameters& material_data, const ray_intersection& intersection, const Float3& w_o, const Float3& w_i) const override
        {
            auto N = material_data.normal;

            auto NdotL = dot(N, w_i);
            auto NdotV = dot(N, w_o);

            Float3 H = normalize(w_i + w_o);
            Float NdotH = dot(N, H);
            Float LdotH = dot(w_i, H);
            
            Float3 Cdlin = mon2lin(material_data.base_color);
            Float Cdlum = .3f * Cdlin.x + .6f *Cdlin.y + .1f *Cdlin.z; // luminance approx.

            Float3 Ctint = select(Float(1), Cdlin/Cdlum, Cdlum > 0); // normalize lum. to isolate hue+sat
            Float3 Cspec0 = lerp(material_data.specular *.08f * lerp(make_float3(1.f), Ctint, material_data.specular_tint),
                Cdlin, material_data.metalness);
            // Float3 Csheen = lerp(make_float3(1.f), Ctint, sheenTint);

            // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
            // and lerp in diffuse retro-reflection based on roughness
            Float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
            Float Fd90 = 0.5f + 2.f * LdotH*LdotH * material_data.roughness;
            Float Fd = lerp(1.0f, Fd90, FL) * lerp(1.0f, Fd90, FV);


            // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
            // 1.25 scale is used to (roughly) preserve albedo
            // Fss90 used to "flatten" retroreflection based on roughness
            // Float Fss90 = LdotH*LdotH * material_data.roughness;
            // Float Fss = lerp(1.0f, Fss90, FL) * lerp(1.0f, Fss90, FV);
            // Float ss = 1.25f * (Fss * (1.f / (NdotL + NdotV) - .5f) + .5f);

            // specular
            Float aspect = 1.f;
            // Float ax = max(.001f, sqr(material_data.roughness)/aspect);
            // Float ay = max(.001f, sqr(material_data.roughness)*aspect);
            Float a = max(.001f, sqr(material_data.roughness));
            Float Ds = GTR2(NdotH, a);
            // Float Ds = GTR2_aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);


            Float FH = SchlickFresnel(LdotH);
            Float3 Fs = lerp(Cspec0, make_float3(1.f), FH);
            Float Gs;
            // Gs  = smithG_GGX_aniso(NdotL, dot(L, X), dot(L, Y), ax, ay);
            // Gs *= smithG_GGX_aniso(NdotV, dot(V, X), dot(V, Y), ax, ay);
            Gs  = smithG_GGX(NdotL, a);
            Gs *= smithG_GGX(NdotV, a);

            return (1.f/pi * Fd * Cdlin) * (1.f - material_data.metalness) + Gs*Fs*Ds;
        }
    };
}
