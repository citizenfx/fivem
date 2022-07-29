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

	if (cli.find(L"pure_1") != std::string_view::npos)
	{
		pureLevel = 1;
	}
	else if (cli.find(L"pure_2") != std::string_view::npos)
	{
		pureLevel = 2;
	}

	return pureLevel;
}
}
}
