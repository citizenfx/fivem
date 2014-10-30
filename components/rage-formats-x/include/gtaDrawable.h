#include <stdint.h>

#include <rmcDrawable.h>
//#include <grcTexture.h>

#define RAGE_FORMATS_FILE gtaDrawable
#include <formats-header.h>

#ifdef RAGE_FORMATS_OK
#if defined(RAGE_FORMATS_GAME_NY)
#define RAGE_FORMATS_ny_gtaDrawable 1
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
	pgArray<CLightAttr> m_lightAttrs;

#ifdef RAGE_FORMATS_GAME_FIVE
	pgPtr<const char> m_name;
#endif

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		rmcDrawable::Resolve(blockMap);

		m_lightAttrs.Resolve(blockMap);

#ifdef RAGE_FORMATS_GAME_FIVE
		m_name.Resolve(blockMap);
#endif
	}
};

#endif

#include <formats-footer.h>