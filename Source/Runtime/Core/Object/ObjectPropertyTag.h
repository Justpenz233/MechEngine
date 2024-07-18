//
// Created by marvelli on 6/2/24.
//
#pragma once
#include "Misc/Platform.h"
#include "ContainerTypes.h"
#include "Containers/FString.h"


struct ENGINE_API ObjectPropertyTag
{
	virtual ~ObjectPropertyTag() = default;
};

/***
 * Create a drag slider widget for the property, in range [ValueMin, ValueMax] with step SlideStep
 */
struct ENGINE_API Slide_ : public ObjectPropertyTag
{
	explicit constexpr Slide_(float InValueMin = 0.f, float InValueMax = 1.f)
		: ValueMin(InValueMin), ValueMax(InValueMax){}

	[[nodiscard]] constexpr float GetMin() const { return ValueMin; }
	[[nodiscard]] constexpr float GetMax() const { return ValueMax; }

protected:
	float ValueMin;
	float ValueMax;
};

/***
 * Create a step widget(with button + and - ) for the property, in range [ValueMin, ValueMax] with step
 */
struct ENGINE_API Step_ : public ObjectPropertyTag
{
	explicit constexpr Step_(float InValueMin, float InValueMax, float InStep)
		: ValueMin(InValueMin), ValueMax(InValueMax), Step(InStep) {}

	explicit constexpr Step_(float InValueMin = 0.f, float InValueMax = 1.f)
			: ValueMin(InValueMin), ValueMax(InValueMax), Step((InValueMin + InValueMax) / 5.f ) {}


	[[nodiscard]] constexpr float GetMin() const { return ValueMin; }
	[[nodiscard]] constexpr float GetMax() const { return ValueMax; }
	[[nodiscard]] constexpr float GetStep() const { return Step; }

protected:
	float ValueMin;
	float ValueMax;
	float Step;
};

/***
 * Create a category for the property, the category should split by '|' as the delimiter, represent the category from root to leaf
 * Example: Category_("Rendering|Shadow")
 */
struct ENGINE_API Category_ : public ObjectPropertyTag
{
public:
	Category_() = delete;
	explicit Category_(const String& InCategory) : CategoryString(InCategory) {}
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
