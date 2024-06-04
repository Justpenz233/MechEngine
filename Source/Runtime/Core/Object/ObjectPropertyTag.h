//
// Created by marvelli on 6/2/24.
//
#pragma once
#include "Misc/Platform.h"
#include "ContainerTypes.h"
#include "Containers/FString.h"


struct ENGINE_API ObjectPropertyTag {};

struct ENGINE_API Drag_ : public ObjectPropertyTag
{
	explicit constexpr Drag_(float InValueMin = 0.f, float InValueMax = 1.f, float InDragStep = 0.01f)
		: ValueMin(InValueMin), ValueMax(InValueMax), DragStep(InDragStep) {}

	[[nodiscard]] constexpr float GetMin() const { return ValueMin; }
	[[nodiscard]] constexpr float GetMax() const { return ValueMax; }
	[[nodiscard]] constexpr float GetDragStep() const { return DragStep; }

protected:
	float ValueMin;
	float ValueMax;
	float DragStep;
};


struct ENGINE_API Category_ : public ObjectPropertyTag
{
public:
	Category_() = delete;
	explicit constexpr Category_(const String& InCategory) : CategoryString(InCategory) {}
	/**
	 * Parse category by splitting the string with '|'
	 * @return Array of category from root to leaf
	 */
	[[nodiscard]] TArray<String> ParseCategory() const
	{
		return FString::SpiltBy(CategoryString, '|');
	}

protected:
	String CategoryString;
};
