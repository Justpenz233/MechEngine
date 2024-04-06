//
// Created by MarvelLi on 2024/2/13.
//

#pragma once
#include "Math/Math.h"

namespace MechEngine::Math
{
template <class T>
struct TBox
{
	using Real = T;
	Vector3<T> Min;
	Vector3<T> Max;

	constexpr TBox()
		: Min(Vector3<T>::Constant(std::numeric_limits<T>::max())), Max(Vector3<T>::Constant(std::numeric_limits<T>::min()))
	{}

	template <typename Arg>
	TBox(const Vector3<Arg>& InMin, const Vector3<Arg>& InMax)
		: Min(InMin.template cast<T>())
		, Max(InMax.template cast<T>())
	{
	}

	template <typename FArg>
	explicit TBox(const TBox<FArg>& From)
		: TBox<T>(From.Min.template cast<T>(), From.Max.template cast<T>()) {}

	template <typename Arg>
	explicit TBox(const Vector3<Arg>& Size)
		: Min(-Size.template cast<T>() * 0.5f), Max(Size.template cast<T>() * 0.5f)
	{

	}

	explicit TBox(const std::vector<Vector3<T>>& Points)
	{
		for (auto& i : Points)
		{
			*this += i;
		}
	}

	explicit TBox(const Eigen::MatrixX3d &Points)
	{
		for (int i = 0; i < Points.rows(); i++)
		{
			*this += Points.row(i);
		}
	}
	/**
	 * Compares two boxes for equality.
	 *
	 * Returns true if both bounding boxes are invalid. Returns false if one of the bounding boxes is invalid.
	 *
	 * @return true if the boxes are equal, false otherwise.
	 */
	FORCEINLINE bool operator==(const TBox<T>& Other) const
	{
		return (Min == Other.Min) && (Max == Other.Max);
	}

	FORCEINLINE bool operator!=(const TBox<T>& Other) const
	{
		return !(*this == Other);
	}

	/**
	 * Adds to this bounding box to include a given point.
	 *
	 * @param Other the point to increase the bounding volume to.
	 * @return Reference to this bounding box after resizing to include the other point.
	 */
	FORCEINLINE TBox<T>& operator+=(const Vector3<T>& Other)
	{
		Min.x() = Math::Min(Min.x(), Other.x());
		Min.y() = Math::Min(Min.y(), Other.y());
		Min.z() = Math::Min(Min.z(), Other.z());

		Max.x() = Math::Max(Max.x(), Other.x());
		Max.y() = Math::Max(Max.y(), Other.y());
		Max.z() = Math::Max(Max.z(), Other.z());
		return *this;
	}

	/**
	 * Adds to this bounding box to include a new bounding volume.
	 *
	 * @param Other the bounding volume to increase the bounding volume to.
	 * @return Reference to this bounding volume after resizing to include the other bounding volume.
	 */
	FORCEINLINE TBox<T>& operator+=(const TBox<T>& Other)
	{
		Min.x() = Math::Min(Min.x(), Other.Min.x());
		Min.y() = Math::Min(Min.y(), Other.Min.y());
		Min.z() = Math::Min(Min.z(), Other.Min.z());

		Max.x() = Math::Max(Max.x(), Other.Max.x());
		Max.y() = Math::Max(Max.y(), Other.Max.y());
		Max.z() = Math::Max(Max.z(), Other.Max.z());
		return *this;
	}

	/**
	 * Gets the result of addition to this bounding volume.
	 *
	 * @param Other The other point to add to this.
	 * @return A new bounding volume.
	 */
	FORCEINLINE TBox<T> operator+(const Vector3<T>& Other) const
	{
		return TBox<T>(*this) += Other;
	}

	/**
	 * Compares two boxes for inequality.
	 *
	 * @return false if the boxes are equal, true otherwise.
	 */
	FORCEINLINE bool ContainsNaN() const
	{
		return Min.hasNaN() || Max.hasNaN();
	}

	/**
	 * Gets the center point of this box.
	 *
	 * @return The center point.
	 * @see GetCenterAndExtents, GetExtent, GetSize, GetVolume
	 */
	FORCEINLINE Vector3<T> GetCenter() const
	{
		return (Min + Max) * 0.5f;
	}

	/**
	 * Gets the size of this box.
	 *
	 * @return The box size.
	 * @see GetCenter, GetCenterAndExtents, GetExtent, GetVolume
	 */
	FORCEINLINE Vector3<T> GetSize() const
	{
		return (Max - Min);
	}

	/**
	 * Gets the volume of this box.
	 *
	 * @return The box volume.
	 * @see GetCenter, GetCenterAndExtents, GetExtent, GetSize
	 */
	FORCEINLINE T GetVolume() const
	{
		return (Max.x() - Min.x()) * (Max.y() - Min.y()) * (Max.z() - Min.z());
	}


	/**
	 * Get 8 vertices of the box
	 * @return 8 vertices of the box
	 */
	FORCEINLINE TArray<Vector3<T>> GetVertex() const
	{
		return {
			Vector3<T>(Min.x(), Min.y(), Min.z()),
			Vector3<T>(Max.x(), Min.y(), Min.z()),
			Vector3<T>(Max.x(), Max.y(), Min.z()),
			Vector3<T>(Min.x(), Max.y(), Min.z()),
			Vector3<T>(Min.x(), Min.y(), Max.z()),
			Vector3<T>(Max.x(), Min.y(), Max.z()),
			Vector3<T>(Max.x(), Max.y(), Max.z()),
			Vector3<T>(Min.x(), Max.y(), Max.z())
		};
	}

	FORCEINLINE TBox<T>& Translate(const Vector3<T>& Offset)
	{
		Min += Offset;
		Max += Offset;
		return *this;
	}

	FORCEINLINE TBox<T>& Scale(const Vector3<T>& InScale)
	{
		Vector3<T> NewSize = GetSize().cwiseProduct(InScale);
		Vector3<T> PreCenter = GetCenter();
		Min = PreCenter - NewSize * 0.5f;
		Max = PreCenter + NewSize * 0.5f;
		return *this;
	}

	/**
	 * Checks whether the given bounding box intersects this bounding box.
	 *
	 * @param Other The bounding box to intersect with.
	 * @return true if the boxes intersect, false otherwise.
	 *
	 * @note  This function assumes boxes have closed bounds, i.e. boxes with
	 *        coincident borders on any edge will overlap.
	 */
	FORCEINLINE bool Intersect(const TBox<T>& Other) const;

	/**
	 * Checks whether the given point is inside this bounding box.
	 * @param Point The point to check for inside the bounding volume.
	 * @return true if the point is inside the bounding volume, false otherwise.
	 */
	FORCEINLINE bool Contain(const Vector3<T>& Point) const;

	/**
	 * Returns the overlap TBox<T> of two box
	 *
	 * @param Other The bounding box to test overlap
	 * @return the overlap box. It can be 0 if they don't overlap
	 */
	 TBox<T> Overlap( const TBox<T>& Other ) const;
};
template <class T>
bool TBox<T>::Intersect(const TBox<T>& Other) const
{
	if ((Min.x() > Other.Max.x()) || (Other.Min.x() > Max.x()))
	{
		return false;
	}

	if ((Min.y() > Other.Max.y()) || (Other.Min.y() > Max.y()))
	{
		return false;
	}

	if ((Min.z() > Other.Max.z()) || (Other.Min.z() > Max.z()))
	{
		return false;
	}

	return true;
}

template <class T>
bool TBox<T>::Contain(const Vector3<T>& Point) const
{
	return Point.x() >= Min.x() && Point.x() <= Max.x() &&
		Point.y() >= Min.y() && Point.y() <= Max.y() &&
		Point.z() >= Min.z() && Point.z() <= Max.z();
}

template <class T>
TBox<T> TBox<T>::Overlap(const TBox<T>& Other) const
{
	if (Intersect(Other) == false)
	{
		static TBox<T> EmptyBox;
		return EmptyBox;
	}

	// otherwise they overlap
	// so find overlapping box
	Vector3<T> MinVector, MaxVector;

	MinVector.x() = Math::Max(Min.x(), Other.Min.x());
	MaxVector.x() = Math::Min(Max.x(), Other.Max.x());

	MinVector.y() = Math::Max(Min.y(), Other.Min.y());
	MaxVector.y() = Math::Min(Max.y(), Other.Max.y());

	MinVector.z() = Math::Max(Min.z(), Other.Min.z());
	MaxVector.z() = Math::Min(Max.z(), Other.Max.z());

	return TBox<T>(MinVector, MaxVector);
}
} // namespace MechEngine::Math