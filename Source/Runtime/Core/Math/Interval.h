//
// Created by Mavel Li on 8/9/23.
//
#pragma once
#include <cmath>
namespace MMath
{
	enum IntervalType
	{
		CloseClose,
		OpenClose,
		CloseOpen,
		OpenOpen
	};

	template <class T>
	class Interval
	{
	public:
		T Left;
		T Right;

		IntervalType Type = CloseClose;
		Interval() { Left = 0.; Right = 0.; }
		Interval(const T& _Left, const T& _Right) : Left(_Left), Right(_Right) {  }
		T Middle() const { return (Left + Right) / (T)2; }
		T Length() const { return std::abs(Right - Left); }
		void swap() { std::swap(Left, Right); }
	};

} // namespace MMath