#pragma once

#ifdef COMPILING_GTA_STREAMING_FIVE
#define PED_DECORATION_SORT_EXPORT DLL_EXPORT
#else
#define PED_DECORATION_SORT_EXPORT DLL_IMPORT
#endif

namespace fx
{
PED_DECORATION_SORT_EXPORT uint32_t GetOverlaySerializerGeneration();
}
