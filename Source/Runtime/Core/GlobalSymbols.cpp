//
// Created by Mayn on 2024/7/21.
//

#include <chrono>
#include "GlobalSymbols.h"
#include <iostream>

ENGINE_API float GAverageFPS = 0.0f;
ENGINE_API float GAverageMS = 0.0f;

// From UE5 source
void CalculateFPSTimings()
{
	// Calculate the average frame time via continued averaging.
	static long long LastTime = 0.0;
	long long CurrentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	if (LastTime == 0.) [[unlikely]]
	{
		LastTime = CurrentTime;
		return;
	}
	float FrameTimeMS = (float)((CurrentTime - LastTime));
	LastTime = CurrentTime;

	// A 3/4, 1/4 split gets close to a simple 10 frame moving average
	GAverageMS = GAverageMS == 0.f ? FrameTimeMS : GAverageMS * 0.75f + FrameTimeMS * 0.25f;


	// Calculate average framerate.
	GAverageFPS = 1000.f / GAverageMS;
}