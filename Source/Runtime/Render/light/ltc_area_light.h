//
// Created by MarvelLi on 2024/11/6.
//

#pragma once
#include <luisa/luisa-compute.h>

/**
* This file contains implementation of the Floattime area light integration by using Light Transfer Coefficient(LTC).
* @see https://stgit.dcs.gla.ac.uk/2644603d/roll-a-ball-2644603d/-/blob/main/Library/PackageCache/com.unity.render-pipelines.core@14.0.11/ShaderLibrary/AreaLighting.hlsl?ref_type=heads
* @see https://blog.selfshadow.com/sandbox/ltc.html
* @see https://advances.Floattimerendering.com/s2016/s2016_ltc_rnd.pdf
*/

namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;

Float3 ComputeEdgeFactor(Float3 V1, Float3 V2)
{
	Float  V1oV2 = dot(V1, V2);
	Float3 V1xV2 = cross(V1, V2);
	#if 0
	return normalize(V1xV2) * acos(V1oV2));
	#else
	// Approximate: { y = rsqrt(1.0 - V1oV2 * V1oV2) * acos(V1oV2) } on [0, 1].
	// Fit: HornerForm[MiniMaxApproximation[ArcCos[x]/Sqrt[1 - x^2], {x, {0, 1 - $MachineEpsilon}, 6, 0}][[2, 1]]].
	// Maximum relative error: 2.6855360216340534 * 10^-6. Intensities up to 1000 are artifact-free.
	Float x = abs(V1oV2);
	Float y = 1.5707921083647782f + x * (-0.9995697178013095f + x * (0.778026455830408f + x * (-0.6173111361273548f + x * (0.4202724111150622f + x * (-0.19452783598217288f + x * 0.04232040013661036f)))));

	$if (V1oV2 < 0)
	{
		// Undo range reduction.
		const auto epsilon = def(1e-5f);
		y = pi * rsqrt(max(epsilon, saturate(1 - V1oV2 * V1oV2))) - y;
	};

	return V1xV2 * y;
	#endif
}
Float IntegrateEdge(const Float3& V1, const Float3& V2)
{
	// 'V1' and 'V2' are represented in a coordinate system with N = (0, 0, 1).
	return ComputeEdgeFactor(V1, V2).z;
}

// This function does not check whether light's contribution is 0.
Float3 PolygonFormFactor(std::array<Float3, 4> L)
{
	L[0] = normalize(L[0]);
	L[1] = normalize(L[1]);
	L[2] = normalize(L[2]);
	L[3] = normalize(L[3]);

	Float3 F  = ComputeEdgeFactor(L[0], L[1]);
	F += ComputeEdgeFactor(L[1], L[2]);
	F += ComputeEdgeFactor(L[2], L[3]);
	F += ComputeEdgeFactor(L[3], L[0]);

	return inv_pi * 0.5f * F;
}
Float PolygonIrradianceFromVectorFormFactor(Float3 F)
{
	auto l = length(F);
	return max(0.f, (l * l + F.z) / (l + 1.f));
}

Float PolygonIrradiance(const std::array<Float3, 4>& L)
{
    Float3 F = PolygonFormFactor(L);
    return PolygonIrradianceFromVectorFormFactor(F);
}

Float3 LTC_Evaluate(
	Float3 N, Float3 V, Float3 P, Float3x3 Minv, std::array<Float3, 4> L)
{
	// construct orthonormal basis around N
	Float3 T1, T2;
	T1 = normalize(V - N*dot(V, N));
	T2 = cross(N, T1);

	// rotate area light in (T1, T2, N) basis
	Minv = Minv * transpose(Float3x3(T1, T2, N));
	for(int i = 0;i < 4;i ++)
		L[i] = Minv * (L[i] - P);

	// sum = twoSided ? abs(sum) : max(0.0, sum);
	auto sum = max(PolygonIrradiance(L), 0.f);

	return make_float3(sum, sum, sum);
}


};