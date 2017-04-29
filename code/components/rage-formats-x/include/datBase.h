/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <rageVectors.h>

#define RAGE_FORMATS_FILE datBase
#include <formats-header.h>

#ifdef RAGE_FORMATS_OK

using Vector3 = rage::Vector3;
using Vector4 = rage::Vector4;

#if defined(RAGE_FORMATS_GAME_NY)
#define RAGE_FORMATS_ny_datBase 1
#elif defined(RAGE_FORMATS_GAME_FIVE)
#define RAGE_FORMATS_five_datBase 1
#endif

#define SwapShortRead(x) (x)
#define SwapShortWrite(x) (x)

#define SwapLongRead(x) (x)
#define SwapLongWrite(x) (x)

#define SwapLongLongRead(x) (x)
#define SwapLongLongWrite(x) (x)

#define SwapSingleRead(x) (x)
#define SwapSingleWrite(x) (x)

class BlockMap;

#ifdef RAGE_NATIVE_ARCHITECTURE
#undef RAGE_NATIVE_ARCHITECTURE
#endif

#if (defined(GTA_FIVE) && defined(RAGE_FORMATS_GAME_FIVE)) || (defined(GTA_NY) && defined(RAGE_FORMATS_GAME_NY))
#define RAGE_NATIVE_ARCHITECTURE 1
#else
#define RAGE_NATIVE_ARCHITECTURE 0
#endif

class FORMATS_EXPORT datBase
{
public:
#if (defined(GTA_FIVE) && defined(RAGE_FORMATS_GAME_FIVE)) || (defined(GTA_NY) && defined(RAGE_FORMATS_GAME_NY))
	virtual ~datBase() {}
#elif defined(RAGE_FORMATS_GAME_FIVE)
	uint64_t m_vt;
#elif defined(RAGE_FORMATS_GAME_NY)
	uint32_t m_vt;
#endif

	void* operator new(size_t size, bool isPhysical);

	void* operator new(size_t size, BlockMap* blockMap, bool isPhysical);

	void* operator new(size_t size);
};
#endif

#include <formats-footer.h>