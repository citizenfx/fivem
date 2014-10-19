#include "StdInc.h"
#include "MumbleClient.h"

int main()
{
	auto mumbleClient = CreateMumbleClient();

	mumbleClient->Initialize();

	//mumbleClient->ConnectAsync("191.236.32.95", 64738, va(L"loveliest%d", GetTickCount())).wait();
	mumbleClient->ConnectAsync("192.168.178.83", 64738, L"loveliest").wait();

	return 0;
}

void GlobalError(const char*, ...)
{
	// ...
}