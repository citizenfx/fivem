#pragma once

namespace rage
{
class GAMESPEC_EXPORT sysTimer
{
private:
	sysTimer() {}

public:
	static uint64_t getCount();

	static float getCountDivisor();
};
}