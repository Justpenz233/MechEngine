//
// Created by MarvelLi on 2024/5/8.
//

#pragma once
#include "ContainerTypes.h"
#include "PointerTypes.h"
#include "Misc/Platform.h"

enum class ETimerStatus : uint8
{
	Active,
	Paused
};

struct TimerData
{
	// /** If true, this timer will loop indefinitely.  Otherwise, it will be destroyed when it expires. */
	// uint8 bLoop : 0;

	// /** Timer Status */
	// ETimerStatus Status = ETimerStatus::Active;

	/** Time between set and fire, or repeat frequency if looping. */
	double Rate;

	/**
	 * Time (on the FTimerManager's clock) that this timer should expire and fire its delegate.
	 * Note when a timer is paused, we re-base ExpireTime to be relative to 0 instead of the running clock,
	 * meaning ExpireTime contains the remaining time until fire.
	 */
	double ExpireTime;

	/** Holds the TFunction callback to call. */
	TFunction<void(void)> Callback;
};

class TimerManager
{
public:
	void Tick(double DeltaTime);

	void AddTimer(double Duration, TFunction<void(void)>&& Callback, double Rate = 0.0166);

protected:
	/** An internally consistent clock, independent of World.  Advances during ticking. */
	double InternalTime = 0.;

	TSet<SharedPtr<TimerData>> Timers;
};
