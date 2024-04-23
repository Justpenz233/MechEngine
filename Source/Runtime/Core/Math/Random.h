#pragma once
#include "Core/CoreMinimal.h"
#include <cmath>
#include <random>

namespace Random
{
    template<class T>
    T RandomInterval(T&& L, T&& R)
    {
        srand(std::time(0));
        std::random_device rd;
        std::mt19937 gen(rd());  //here you could set the seed, but std::random_device already does that
        std::uniform_real_distribution<T> dis(L, R);
        return dis(gen);
    }

    inline double Random()
    {
        return RandomInterval<double>(0, 1);
    }

    inline FVector RandomFVector(double L = 0., double R = 1.)
    {
        srand(std::time(0));
        std::random_device rd;
        std::mt19937 gen(rd());  //here you could set the seed, but std::random_device already does that
        std::uniform_real_distribution<double> dis(L, R);
        return FVector::NullaryExpr([&](){return dis(gen);} );
    }

    inline Vector2d RandomVector2d(double L = 0., double R = 1.)
    {
        srand(std::time(0));
        std::random_device rd;
        std::mt19937 gen(rd());  //here you could set the seed, but std::random_device already does that
        std::uniform_real_distribution<double> dis(L, R);
        return Vector2d::NullaryExpr([&](){return dis(gen);} );
    }

    inline FQuat RandomQuat()
    {
        const double u1 = RandomInterval<double>(0., 1.);
        const double u2 = RandomInterval<double>(0., 2. * M_PI);
        const double u3 = RandomInterval<double>(0., 2. * M_PI);
        const double a = sqrt(1. - u1),
        b = sqrt(u1);
        return FQuat (a * sin(u2), a * cos(u2), b * sin(u3), b * cos(u3));
    }
};