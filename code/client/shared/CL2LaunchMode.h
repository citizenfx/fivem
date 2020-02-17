#pragma once

inline bool IsCL2()
{
#ifndef IS_FXSERVER
	if (wcsstr(GetCommandLine(), L"cl2") != nullptr)
	{
		return true;
	}
#endif

	return false;
}
