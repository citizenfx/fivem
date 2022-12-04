#undef GetSystemInfo
#include <windows.h>

extern "C" void WINAPI GetSystemInfoFake(LPSYSTEM_INFO info)
{
	GetSystemInfo(info);

	info->dwNumberOfProcessors = min(info->dwNumberOfProcessors, 2);
}
