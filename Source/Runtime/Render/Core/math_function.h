//
// Created by MarvelLi on 2024/4/30.
//

#pragma once

#include <luisa/luisa-compute.h>
#include "Misc/Platform.h"

namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;

	template<typename T>
	FORCEINLINE void swap(T& a, T& b) noexcept
	{
		auto temp = a;
		a = b;
		b = temp;
	}

	template<typename T>
	FORCEINLINE Var<T> square(const Var<T>& x)
	{
		return x * x;
	}

    /**
     * Calulate the distance from a point to a segment in 2D space.
     * @param p point in 2D space
     * @param x start point of the segment
     * @param y end point of the segment
     * @return the distance from the point to the segment
     */
    FORCEINLINE Float distance_to_segment(const Float2& p, const Float2& x, const Float2& y)
    {
        auto l = y - x;
        auto t = dot(p - x, l) / dot(l, l);
        auto dis = length(p - (x + t * l));
        $if (t < 0.f) {dis = length(p - x);};
        $if (t > 1.f) {dis = length(p - y);};
        return dis;
    }

    /**
     * Given the viewport coordinate, test if the pixel is in the segments x -> y with given thickness.
     * @param p pixel in viewport space
     * @param x start point of the line in viewport space
     * @param y end point of the line in viewport space
     * @param thickness thickness of the line in pixel
     */
    FORCEINLINE Bool on_the_lines(const Float2& p, const Float2& x, const Float2& y, const Float& thickness)
    {
        return distance_to_segment(p, x, y) <= thickness;
    }

	/**
	 * Given the viewport coordinate, and world space triangle, calculate the distance from the pixel to the triangle.
	* @param view current view
	* @param p pixel in viewport space
	* @param v0 vertex 0 of the triangle in screen space
	* @param v1 vertex 1 of the triangle in screen space
	* @param v2 vertex 2 of the triangle in screen space
	* @return the distance from the pixel to the triangle
	 */
	FORCEINLINE Float distance_to_triangle(const Float2& p, const Float2& v0, const Float2& v1, const Float2& v2)
	{
		return min(distance_to_segment(p, v0, v1),
			min(distance_to_segment(p, v1, v2),
				distance_to_segment(p, v2, v0)));
	}

	/**
	 * Test if a point is inside a box.
	 * @param p given point
	 * @param min min point of the box
	 * @param max max point of the box
	 * @return true if the point is inside the box
	 */
	FORCEINLINE Bool point_inside_box(const Float3& p, const Float3& min, const Float3& max)
	{
		return all(p >= min) & all(p <= max);
	}

	FORCEINLINE Bool point_inside_box(const Float2& p, const Float2& min, const Float2& max)
	{
		return all(p >= min) & all(p <= max);
	}

	/**
	 * Test if a segment totally outside box.
	 * @param p0 start point of the segment
	 * @param p1 end point of the segment
	 * @param min min point of the box
	 * @param max max point of the box
	 * @return true if the segment is totally outside the box
	 */
	FORCEINLINE Bool segment_outside_box(const Float2& p0, const Float2& p1, const Float2& min, const Float2& max)
	{
		return (p0.x < min.x & p1.x < min.x) | (p0.x > max.x & p1.x > max.x) |
				(p0.y < min.y & p1.y < min.y) | (p0.y > max.y & p1.y > max.y);
	}
	FORCEINLINE Bool segment_outside_box(const Float3& p0, const Float3& p1, const Float3& min, const Float3& max)
	{
		return (p0.x < min.x & p1.x < min.x) | (p0.x > max.x & p1.x > max.x) |
			(p0.y < min.y & p1.y < min.y) | (p0.y > max.y & p1.y > max.y) |
			(p0.z < min.z & p1.z < min.z) | (p0.z > max.z & p1.z > max.z);
	}

	/**
	 * Test if a segment totally inside box.
	 * @param p0 start point of the segment
	 * @param p1 end point of the segment
	 * @param min min point of the box
	 * @param max max point of the box
	 * @return true if the segment is totally inside the box
	 */
	FORCEINLINE Bool segment_inside_box(const Float2& p0, const Float2& p1, const Float2& min, const Float2& max)
	{
		return all(p0 >= min) & all(p0 <= max) & all(p1 >= min) & all(p1 <= max);
	}

	FORCEINLINE Bool segment_inside_box(const Float3& p0, const Float3& p1, const Float3& min, const Float3& max)
	{
		return all(p0 >= min) & all(p0 <= max) & all(p1 >= min) & all(p1 <= max);
	}

	/**
	 * Clip a 3d segment within a box.
	 * Should test if the segment is outside/inside the box first.
	 * @param p0 point of the segment
	 * @param p1 point of the segment
	 * @param min min point of the box
	 * @param max max point of the box
	 * @return the clipped segment
	 */
	FORCEINLINE std::pair<Float3, Float3> clip_segment_within_box_3D(
		const Float3& p0, const Float3& p1, const Float3& min, const Float3& max)
	{
		Float2 x = make_float2(p0.x, p1.x);
		Float2 y = make_float2(p0.y, p1.y);
		Float2 z = make_float2(p0.z, p1.z);

		Float a = x[1] - x[0];
		Float b = y[1] - y[0];
		Float c = z[1] - z[0];
		$for(i, 0, 2)
		{
			$if(x[i] < min.x)
			{
				y[i] = b / a * (min.x - x[0]) + y[0];
				z[i] = c / a * (min.x - x[0]) + z[0];
				x[i] = min.x;
			}
			$elif(x[i] > max.x)
			{
				y[i] = b / a * (max.x - x[0]) + y[0];
				z[i] = c / a * (max.x - x[0]) + z[0];
				x[i] = max.x;
			};
			$if(y[i] < min.y)
			{
				x[i] = a / b * (min.y - y[0]) + x[0];
				z[i] = c / b * (min.y - y[0]) + z[0];
				y[i] = min.y;
			}
			$elif(y[i] > max.y)
			{
				x[i] = a / b * (max.y - y[0]) + x[0];
				z[i] = c / b * (max.y - y[0]) + z[0];
				y[i] = max.y;
			};
			$if(z[i] < min.z)
			{
				x[i] = a / c * (min.z - z[0]) + x[0];
				y[i] = b / c * (min.z - z[0]) + y[0];
				z[i] = min.z;
			}
			$elif(z[i] > max.z)
			{
				x[i] = a / c * (max.z - z[0]) + x[0];
				y[i] = b / c * (max.z - z[0]) + y[0];
				z[i] = max.z;
			};
		};
		return std::make_pair(make_float3(x[0], y[0], z[0]), make_float3(x[1], y[1], z[1]));
	}

	/**
	 * Calculate the orthogonal basis of a given normal. Assume the normal is normalized, and is z axis.
	 * @see https://raytracing.github.io/books/RayTracingTheRestOfYourLife.html#orthonormalbases
	 * @param n given normal
	 * @return the orthogonal basis
	 */
	FORCEINLINE std::array<Float3, 3> orthogonal_basis(const Float3& n)
	{
		auto sgn = sign(n.z);
		auto a = -1.f / (sgn + n.z);
		auto b = n.x * n.y * a;
		auto s = make_float3(1.f + sgn * square(n.x) * a, sgn * b, -sgn * n.x);
		auto t = make_float3(b, sgn + square(n.y) * a, -n.y);
		return {normalize(s), normalize(t), n};
	}
	/**
	 * Calculate the orthogonal basis of a given normal and a point mapping to normal space. Assume the normal is normalized, and is z axis.
	 * @param n given normal
	 * @param p point mapping to normal space
	 * @return new point in the orthogonal basis
	 */
	FORCEINLINE Float3 orthogonal_basis(const Float3& n, const Float3& p)
	{
		auto basis = orthogonal_basis(n);
		return p.x * basis[0] + p.y * basis[1] + p.z * basis[2];
	}

}
