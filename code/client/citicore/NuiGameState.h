#pragma once

#ifndef IS_FXSERVER
#include <atomic>
#include <cstdint>

struct NuiGameStateData
{
	float pedPos[3]{};

	float camPos[3]{};

	float camRot[3]{};

	float camForward[3]{};
	float camUp[3]{};

	float camFov = 0.0f;
};

struct NuiGameState
{
	// seqlock: the writer makes this odd for the duration of a write, and it changes on
	// every write, so a reader that sees the same even value before and after copying
	// `data` knows it did not observe a half-written frame. 0 means 'never written'.
	std::atomic<uint32_t> sequence{ 0 };

	NuiGameStateData data;
};
#endif
