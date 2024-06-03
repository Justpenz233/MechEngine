//
// Created by marvelli on 6/2/24.
//
#pragma once
#include "Misc/Platform.h"
#include "ContainerTypes.h"
#include "Containers/FString.h"

namespace PropertyTag
{
struct ENGINE_API ObjectPropertyTag {};

struct ENGINE_API FloatDragTag : public ObjectPropertyTag
{
	explicit FloatDragTag(float InValueMin = 0.f, float InValueMax = 1.f, float InDragStep = 0.01f)
		: ValueMin(InValueMin), ValueMax(InValueMax), DragStep(InDragStep) {}

	[[nodiscard]] float GetMin() const { return ValueMin; }
	[[nodiscard]] float GetMax() const { return ValueMax; }
	[[nodiscard]] float GetDragStep() const { return DragStep; }

protected:
	float ValueMin;
	float ValueMax;
	float DragStep;
};


struct ENGINE_API CategoryTag : public ObjectPropertyTag
{

public:
	CategoryTag() = delete;
	explicit CategoryTag(const String& InCategory) : Category(InCategory) {}
	/**
	 * Parse category by splitting the string with '|'
	 * @return Array of category from root to leaf
	 */
	[[nodiscard]] TArray<String> ParseCategory() const
	{
		return FString::SpiltBy(Category, '|');
	}

protected:
	String Category;
};
}
