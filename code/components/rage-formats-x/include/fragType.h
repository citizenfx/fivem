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

struct evtSet
{

};

struct fragTypeGroup
{

};

struct phArchetype
{

};

struct fragType_obj1
{
	pgArray<uint32_t> _f0;
	uint32_t   _f8;
	pgPtr<void>   _fC;
	pgPtr<evtSet>   m_pEventSet;   // pgPtr<rage::evtSet> (same as fragType::m_pCollisionEvents)
	struct {
		uint32_t a;
		uint32_t b;
	} _f14[20];
	uint32_t   _fB4;
	float   _fB8;
	uint32_t   _fBC;
	uint32_t   _fC0;
	uint32_t   _fC4;  // unused
	uint8_t    _fC8;
	uint8_t    _fC9;
	uint8_t    _fCA;
	uint8_t    _fCB;
};

class fragType : public pgBase
{
private:
#ifdef RAGE_FORMATS_GAME_NY
	float m_maxInertiaValueNormalized;
	float m_maxInertiaValue;
	Vector4 m_centerRadius; // center/radius
	Vector3 m_centerOfMass;
	Vector3 m_centerOfMass2;
	Vector3 m_unbrokenCGOffset;
	Vector3 m_dampingLinearC;
	Vector3 m_dampingLinearV;
	Vector3 m_dampingLinearV2;
	Vector3 m_dampingAngularC;
	Vector3 m_dampingAngularV;
	Vector3 m_dampingAngularV2;
	pgPtr<char> m_name;
	pgPtr<rmcDrawable> m_drawable; // actually fragDrawable
	pgPtr<void> m_unk0; // 0?
	pgPtr<void> m_unk1; // 0?
	uint32_t m_unkInt; // 0?
	uint32_t m_unkInt2;
	pgPtr<void> m_child;
	pgPtr<pgPtr<char>> m_groupNames;
	pgPtr<pgPtr<fragTypeGroup>> m_groups;
	pgPtr<pgPtr<fragTypeChild>> m_children;
	pgObjectArray<void> m_unkArray;
	pgPtr<void> m_unkPtr;
	pgPtr<phArchetype> m_archetype;
	pgPtr<phArchetype> m_archetype2;
	pgPtr<phBound> m_bound;
	pgPtr<Vector3> m_childInertia;
	pgPtr<Vector3> m_childInertiaDamaged;
	pgPtr<Matrix3x4> m_matrices;
	pgPtr<uint8_t> m_selfCollisionIndices1;
	pgPtr<uint8_t> m_selfCollisionIndices2;
	uint32_t m_modelIndex;
	pgPtr<evtSet> m_collisionEvents;
	fragType_obj1 m_obj1;
	pgPtr<void> m_frame; // actually crFrame
	pgPtr<void> m_f1DC; // actually fragType_obj2
	pgPtr<void> m_f1E0; // actually fragType_obj2
	pgPtr<void> m_f1E4;
	uint32_t m_estimatedCacheSize;
	uint32_t m_estimatedArticulatedCacheSize;
	uint8_t m_selfCollisionCount;
	bool m_selfCollisionCountAllocated;
	uint8_t m_groupCount;
	uint8_t m_childCount;
	uint8_t m_fragTypeGroupCount;
	uint8_t m_damageRegions;
	uint8_t m_childCount2;
	uint8_t m_flags;
	uint8_t m_entityClass;
	uint8_t m_becomeRope;
	uint8_t m_artAssetId;
	uint8_t m_bAttachBottomEnd;
	uint32_t m_unkInt3;
	float m_minMoveForce;

	// gtaFragType follows
#elif defined(RAGE_FORMATS_GAME_FIVE)
	char m_pad[16]; // +16
	Vector4 m_centerRadius; // +32
	pgPtr<rmcDrawable> m_primaryDrawable; // +48
	pgPtr<pgPtr<rmcDrawable>> m_drawables; // +56
	pgPtr<pgPtr<char>> m_drawableNames; // +64
	uint32_t m_drawableCount; // +72
	uint32_t m_damagedDrawableIdx; // +76
	pgPtr<fragTypeChild> m_child; // +80
	pgPtr<char> m_name; // +88
	pgObjectArray<void> m_environmentCloth;
	pgObjectArray<void> m_characterCloth;
	uint64_t m_f80;
	pgPtr<evtSet> m_collisionEventSet;
	uint64_t m_collisionEventPlayer;
	uint8_t m_pad2[16];
	pgPtr<void> m_matrixSet;
	uint64_t m_estimatedCacheSize;
	uint64_t m_estimatedArticulatedCacheSize;
	uint8_t m_entityClass;
	uint8_t m_artAssetId;
	uint8_t m_bAttachBottomEnd;
	uint16_t m_flags;
	uint32_t m_clientClassId;
	float m_unbrokeElasticity;
	float m_gravityFactor;
	float m_buoyancyFactor;
	uint8_t m_glassAttachmentBone;
	uint8_t m_glassPaneModelInfosCount;
	pgPtr<void> m_glassPaneModelInfos;
	uint64_t m_pad3;
	pgPtr<fragPhysicsLODGroup> m_lodGroup;
	pgPtr<rmcDrawable> m_clothDrawable; // actually: fragDrawable
	uint64_t m_pad4;
	uint64_t m_pad5;
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