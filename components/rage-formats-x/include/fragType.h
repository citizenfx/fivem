/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <stdint.h>

#include <rmcDrawable.h>

#define RAGE_FORMATS_FILE fragType
#include <formats-header.h>

#ifdef RAGE_FORMATS_OK
#if defined(RAGE_FORMATS_GAME_NY)
#define RAGE_FORMATS_ny_fragType 1
#endif

class fragTypeChild : public datBase
{
private:
	char m_pad[140];
	pgPtr<rmcDrawable> m_drawable;
	pgPtr<rmcDrawable> m_drawable2;

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_drawable.Resolve(blockMap);
		m_drawable2.Resolve(blockMap);

		if (!m_drawable.IsNull())
		{
			m_drawable->Resolve(blockMap);
		}

		if (!m_drawable2.IsNull())
		{
			m_drawable2->Resolve(blockMap);
		}
	}

	inline rmcDrawable* GetDrawable()
	{
		return *m_drawable;
	}
	
	inline rmcDrawable* GetDrawable2()
	{
		return *m_drawable2;
	}
};

class fragType : public pgBase
{
private:
	char m_pad[172];
	pgPtr<rmcDrawable> m_drawable; // actually fragDrawable

	char m_pad2[28];
	pgPtr<pgPtr<fragTypeChild>> m_children;

	char m_pad3[283];
	uint8_t m_childCount;

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_drawable.Resolve(blockMap);

		if (!m_drawable.IsNull())
		{
			m_drawable->Resolve(blockMap);
		}

		m_children.Resolve(blockMap);

		for (int i = 0; i < m_childCount; i++)
		{
			(*m_children)[i].Resolve(blockMap);
			(*m_children)[i]->Resolve(blockMap);
		}
	}

	inline rmcDrawable* GetDrawable()
	{
		return (*m_drawable);
	}

	inline uint8_t GetNumChildren()
	{
		return m_childCount;
	}

	inline fragTypeChild* GetChild(int idx)
	{
		return *((*m_children)[idx]);
	}
};

#endif

#include <formats-footer.h>