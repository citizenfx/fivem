#include <StdInc.h>
#include <ServerTime.h>

std::chrono::milliseconds msec()
{
	auto curTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
	static auto startTime = curTime;

	return curTime - startTime;
}
