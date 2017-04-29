#include "StdInc.h"
#include "sysTimer.h"

namespace rage
{
uint64_t WRAPPER sysTimer::getCount() { EAXJMP(0x5AFB70); }

float sysTimer::getCountDivisor()
{
	return *reinterpret_cast<float*>(0x18A85D8);
}
}