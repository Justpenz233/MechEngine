#pragma once
#include "Misc/Platform.h"
#include "MathType.h"

class StaticMesh;
namespace MechEngine::Math
{
template <class T>
constexpr FORCEINLINE double DegToRad(T Deg)
{
	return Deg * M_PI / 180.;
}

constexpr FORCEINLINE float DegToRad(float Deg)
{
	return Deg * static_cast<float>(M_PI) / 180.f;
}

template <class T>
constexpr FORCEINLINE double RadToDeg(T Rad)
{
	return Rad * 180. * M_1_PI;
}

template <class T>
constexpr FORCEINLINE Vector3<T> DegToRad(const Vector3<T>& DegVector)
{
	return {DegToRad(DegVector.x()), DegToRad(DegVector.y()), DegToRad(DegVector.z())};
}

template <class T>
constexpr FORCEINLINE Vector3<T> RadToDeg(const Vector3<T>& RadVector)
{
	return {RadToDeg(RadVector.x()), RadToDeg(RadVector.y()), RadToDeg(RadVector.z())};
}

template <class T>
constexpr FORCEINLINE T Frac(T x)
{
	return x - std::floor(x);
}

/** Returns higher value in a generic way */
template <class T>
constexpr FORCEINLINE T Max(const T A, const T B)
{
	return (B < A) ? A : B;
}

template <class Arg1, class Arg2>
constexpr FORCEINLINE auto Max(Arg1 A, Arg2 B) -> decltype(A * B)
{
	using ArgType = decltype(A * B);
	return Max(static_cast<ArgType>(A), static_cast<ArgType>(B));
}

/** Returns lower value in a generic way */
template <class T>
constexpr FORCEINLINE T Min(const T A, const T B)
{
	return (A < B) ? A : B;
}

/** Mixing float and double types with any other type results in same type as "X * Y"  */
template <class Arg1, class Arg2>
constexpr FORCEINLINE auto Min(Arg1 A, Arg2 B) -> decltype(A * B)
{
	using ArgType = decltype(A * B);
	return Min(static_cast<ArgType>(A) , static_cast<ArgType>(B));
}

/** Returns 1, 0, or -1 depending on relation of A to 0 */
template< class T >
static constexpr FORCEINLINE T Sign( const T A )
{
	return (A > static_cast<T>(0)) ? static_cast<T>(1) : ((A < static_cast<T>(0)) ? static_cast<T>(-1) : static_cast<T>(0));
}

template <class T1, class T2>
constexpr FORCEINLINE T1 Lerp(const T1 A, const T1 B, const T2 T)
{
	return A * (static_cast<T2>(1.) - T) + B * T;
}

template <class T>
constexpr FORCEINLINE T Pow2(const T A)
{
	return A * A;
}

template <class T1, class T2>
FVector MakeVector3d(Eigen::Vector2<T1> XY, T2 Z = 0.)
{
	return {XY[0], XY[1], Z};
}

template <class T1, class T2>
FVector MakeVector3d(T1 X, Eigen::Vector2<T2> YZ)
{
	return {X, YZ[0], YZ[1]};
}

template <class T1, class T2>
Vector4d MakeVector4d(Eigen::Vector3<T1> XYZ, T2 W = 0.)
{
	return {XYZ[0], XYZ[1], XYZ[2], W};
}

}