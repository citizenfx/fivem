#pragma once

#ifndef GTA_CORE_EXPORT
#ifdef COMPILING_GTA_CORE_FIVE
#define GTA_CORE_EXPORT DLL_EXPORT
#else
#define GTA_CORE_EXPORT DLL_IMPORT
#endif
#endif

namespace sf
{
	int GTA_CORE_EXPORT RegisterFontIndex(const std::string& fontName);

	void GTA_CORE_EXPORT RegisterFontLib(const std::string& swfName);
}