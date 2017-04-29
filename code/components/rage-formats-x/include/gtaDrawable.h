/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <stdint.h>

#include <rmcDrawable.h>
//#include <grcTexture.h>

#define RAGE_FORMATS_FILE gtaDrawable
#include <formats-header.h>

#ifdef RAGE_FORMATS_OK
#if defined(RAGE_FORMATS_GAME_NY)
#define RAGE_FORMATS_ny_gtaDrawable 1
#endif

#if defined(RAGE_FORMATS_GAME_FIVE)
#define RAGE_FORMATS_five_gtaDrawable 1
#endif

class CLightAttr : public datBase
{
private:
	// undefined for now
	uint8_t m_pad[0x68];

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{

	}
};

class gtaDrawable : public rmcDrawable
{
private:
#ifdef RAGE_FORMATS_GAME_FIVE
	pgPtr<const char> m_name;
#endif

	pgArray<CLightAttr> m_lightAttrs;

#ifdef RAGE_FORMATS_GAME_FIVE
	pgPtr<void> m_unk1;
	pgPtr<void> m_unk2;
#endif

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		rmcDrawable::Resolve(blockMap);

		m_lightAttrs.Resolve(blockMap);

#ifdef RAGE_FORMATS_GAME_FIVE
		m_name.Resolve(blockMap);

		m_unk1.Resolve(blockMap);
		m_unk2.Resolve(blockMap);
#endif
	}

#ifdef RAGE_FORMATS_GAME_FIVE
	inline void SetName(const char* name)
	{
		m_name = pgStreamManager::StringDup(name);
	}
#endif
};

#endif

#include <formats-footer.h>