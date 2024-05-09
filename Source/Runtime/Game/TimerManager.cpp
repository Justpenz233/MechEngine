//
// Created by MarvelLi on 2024/5/8.
//

#include "TimerManager.h"
#include "PointerTypes.h"
#include "Log/Log.h"
#include "Math/Math.h"

void TimerManager::Tick(double DeltaTime)
{
	InternalTime += DeltaTime;

	TArray<SharedPtr<TimerData>> ToRemove;
	for (const auto& Timer : Timers)
	{
		if (InternalTime >= Timer->ExpireTime)
			ToRemove.emplace_back(Timer);
	}
	for (const auto& Timer : ToRemove)
	{
		Timers.erase(Timer);
	}
	ToRemove.clear();

	for (const auto& Timer : Timers)
	{
		if (InternalTime < Timer->ExpireTime)
		{
			const int CallCount = Max(int(DeltaTime / Timer->Rate), 1);
			for (int i = 0; i < CallCount; i++)
				Timer->Callback();
		}
	}
}

void TimerManager::AddTimer(double Duration, TFunction<void(void)>&& Callback, double Rate)
{
	auto Timer = MakeShared<TimerData>();
	Timer->Rate = Rate;
	Timer->ExpireTime = InternalTime + Duration;
	Timer->Callback = Callback;
	Timers.emplace(Timer);
}