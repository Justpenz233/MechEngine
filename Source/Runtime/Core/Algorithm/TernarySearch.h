//
// Created by MarvelLi on 2024/1/15.
//

#pragma once

// TernarySearch find a local minimum of a function, the function should be unimodal
namespace MechEngine::Algorithm
{
	inline double TernarySearch(std::function<double(const double&)>&& func, double Min, double Max, double Eps = 1e-6)
	{
		double L = Min, R = Max;
		int MaxIter = 1000;
		while (R - L > Eps && MaxIter--)
		{
			double L1 = L + (R - L) / 3;
			double L2 = R - (R - L) / 3;
			if (func(L1) < func(L2))
				R = L2;
			else
				L = L1;
		}
		if(MaxIter == 0)
			LOG_WARNING("TernarySearch reach max iteration");
		return (L + R) / 2;
	}
}