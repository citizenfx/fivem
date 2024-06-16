#pragma once

#include <HostSharedData.h>
#include <CfxState.h>

namespace fx
{
namespace client
{
inline int GetPureLevel()
{
	static int pureLevel = -1;

	if (pureLevel != -1)
	{
		return pureLevel;
	}

	auto sharedData = CfxState::Get();
	std::wstring_view cli = (sharedData->initCommandLine[0]) ? sharedData->initCommandLine : GetCommandLineW();
	pureLevel = 0;

	size_t found = cli.find(L"pure_");
	if (found != std::wstring_view::npos)
	{
		pureLevel = _wtoi(&cli[found + 5]);
	}

	return pureLevel;
}
}
}
