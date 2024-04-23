//
// Created by MarvelLi on 2024/3/27.
//

#pragma once

#include <Eigen/Eigen>
#include <luisa/luisa-compute.h>

namespace MechEngine::Rendering
{
	template<class T>
	inline luisa::float4x4 ToLuisaMatrix(const Eigen::Matrix4<T>& InMatrix)
	{
		luisa::float4x4 Result;
		for (int i = 0; i < 4; i++)
			Result[i] = luisa::float4(InMatrix(0, i), InMatrix(1, i), InMatrix(2, i), InMatrix(3, i));
		return Result;
	}

	template <int N, class T>
	luisa::Vector<float, N> ToLuisaVector(const Eigen::Matrix<T, N, 1>& InVector)
	{
		luisa::Vector<float, N> Result;
		for (int i = 0; i < N; i++) Result[i] = InVector(i);
		return Result;
	}
}