#pragma once

#ifndef GTA_STREAMING_EXPORT
#ifdef COMPILING_GTA_STREAMING_FIVE
#define GTA_STREAMING_EXPORT DLL_EXPORT
#else
#define GTA_STREAMING_EXPORT DLL_IMPORT
#endif
#endif

namespace sf
{
	int GTA_STREAMING_EXPORT RegisterFontIndex(const std::string& fontName);

	void GTA_STREAMING_EXPORT RegisterFontLib(const std::string& swfName);

	int GTA_STREAMING_EXPORT AddMinimapOverlay(const std::string& swfName);

	void GTA_STREAMING_EXPORT RemoveMinimapOverlay(int swfId);

	bool GTA_STREAMING_EXPORT HasMinimapLoaded(int swfId);

	bool GTA_STREAMING_EXPORT CallMinimapOverlay(int minimap, const std::string& functionName);

	void GTA_STREAMING_EXPORT SetMinimapOverlayDisplay(int minimap, float x, float y, float xScale, float yScale, float alpha);
}
