#pragma once

#ifdef _WIN32
#include <strsafe.h>

struct UpdaterUIState
{
	char title[1024];
	char subtitle[1024];

	bool waitForExpiration = false;
	bool finalized = true;
	double progress = -1.0;

	inline UpdaterUIState()
	{
		memset(title, 0, sizeof(title));
		memset(subtitle, 0, sizeof(subtitle));
	}

	inline void SetText(int place, const std::string& str)
	{
		StringCbCopyA(place == 0 ? title : subtitle, sizeof(title), str.c_str());
	}

	inline void SetProgress(double percent)
	{
		progress = percent;
	}

	inline void Open()
	{
		finalized = false;
	}

	inline void Close()
	{
		finalized = true;
		waitForExpiration = false;
	}

	inline void OpenWhenExpired()
	{
		waitForExpiration = true;
		finalized = false;
	}
};
#endif
