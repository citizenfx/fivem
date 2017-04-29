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
#ifdef RAGE_FORMATS_GAME_NY
	char m_pad[140];
#elif defined(RAGE_FORMATS_GAME_FIVE)
	char m_pad[152];
#endif
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

class fragPhysicsLOD : public pgBase
{
private:
	char m_pad[192];

	pgPtr<pgPtr<fragTypeChild>> m_children;

	uint8_t m_pad2[70];

	uint8_t m_childCount;

public:
	inline uint8_t GetNumChildren()
	{
		return m_childCount;
	}

	inline fragTypeChild* GetChild(int idx)
	{
		return *((*m_children)[idx]);
	}
};

class fragPhysicsLODGroup : public pgBase
{
private:
	pgPtr<fragPhysicsLOD> m_lods[3];

public:
	inline fragPhysicsLOD* GetLod(int idx)
	{
		return *(m_lods[idx]);
	}
};

class fragType : public pgBase
{
private:
#ifdef RAGE_FORMATS_GAME_NY
	char m_pad[172];
	pgPtr<rmcDrawable> m_drawable; // actually fragDrawable

	char m_pad2[28];
	pgPtr<pgPtr<fragTypeChild>> m_children;

	char m_pad3[283];
	uint8_t m_childCount;
#elif defined(RAGE_FORMATS_GAME_FIVE)
	char m_pad[32];
	pgPtr<rmcDrawable> m_primaryDrawable;

	pgPtr<pgPtr<rmcDrawable>> m_drawables;

	pgPtr<void> m_pad2;

	uint32_t m_drawableCount;

	char m_pad3[164];

	pgPtr<fragPhysicsLODGroup> m_lodGroup;
#endif

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
#ifdef RAGE_FORMATS_GAME_NY
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
#endif
	}

#ifdef RAGE_FORMATS_GAME_NY
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
#elif defined(RAGE_FORMATS_GAME_FIVE)
	inline rmcDrawable* GetPrimaryDrawable()
	{
		return (*m_primaryDrawable);
	}

	inline uint32_t GetNumDrawables()
	{
		return m_drawableCount;
	}

	inline rmcDrawable* GetDrawable(int i)
	{
		return *((*m_drawables)[i]);
	}

	inline fragPhysicsLODGroup* GetLodGroup()
	{
		return *m_lodGroup;
	}
#endif
};

#endif

#include <formats-footer.h>