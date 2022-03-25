#pragma once

#ifndef IS_FXSERVER
#include <HostSharedData.h>
#include <CfxState.h>

#include <string_view>

namespace shmr
{
bool IsShMode()
{
	static bool isShModeSet = false; 
	static bool isShMode = false;

	if (!isShModeSet)
	{
		auto sharedData = CfxState::Get();
		std::wstring_view cli = (sharedData->initCommandLine[0]) ? sharedData->initCommandLine : GetCommandLineW();

		if (cli.find(L"-sh") != std::wstring_view::npos)
		{
			isShMode = true;
		}

	}
	return isShMode;
}
}
#endif
