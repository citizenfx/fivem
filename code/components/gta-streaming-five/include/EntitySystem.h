#pragma once

#include <atArray.h>

#include <directxmath.h>

#ifdef COMPILING_GTA_STREAMING_FIVE
#define STREAMING_EXPORT DLL_EXPORT
#else
#define STREAMING_EXPORT DLL_IMPORT
#endif

using Vector3 = DirectX::XMFLOAT3;
using Matrix4x4 = DirectX::XMFLOAT4X4;

namespace rage
{
	class STREAMING_EXPORT fwRefAwareBase
	{
	public:
		~fwRefAwareBase() = default;

	public:
		void AddKnownRef(void** ref) const;

		void RemoveKnownRef(void** ref) const;
	};
}

template<typename TSubClass>
class fwFactoryBase
{
public:
	virtual ~fwFactoryBase() = 0;

	virtual TSubClass* Get(uint32_t hash) = 0;

	virtual void m3() = 0;
	virtual void m4() = 0;

	virtual void* GetOrCreate(uint32_t hash, uint32_t numEntries) = 0;

	virtual void Remove(uint32_t hash) = 0;

	virtual void ForAllOfHash(uint32_t hash, void(*cb)(TSubClass*)) = 0;
};

class STREAMING_EXPORT fwArchetypeDef
{
public:
	virtual ~fwArchetypeDef();

	virtual int64_t GetTypeIdentifier();

	float lodDist;
	uint32_t flags; // 0x10000 = alphaclip
	uint32_t specialAttribute; // lower 5 bits == 31 -> use alpha clip, get masked to 31 in InitializeFromArchetypeDef
	uint32_t pad;
	void* pad2;
	float bbMin[4];
	float bbMax[4];
	float bsCentre[4];
	float bsRadius;
	float hdTextureDist;
	uint32_t name;
	uint32_t textureDictionary;
	uint32_t clipDictionary;
	uint32_t drawableDictionary;
	uint32_t physicsDictionary;
	uint32_t assetType;
	uint32_t assetName;
	uint32_t pad5[7];

public:
	fwArchetypeDef()
	{
		flags = 0x10000; // was 0x2000
		lodDist = 299.0f;
		hdTextureDist = 375.0f;

		drawableDictionary = 0;
		assetType = 3;
		assetName = 0x12345678;

		specialAttribute = 31;

		pad = 0;
		pad2 = 0;
		clipDictionary = 0;
		physicsDictionary = 0;
		memset(pad5, 0, sizeof(physicsDictionary));
	}
};

class fwEntity;

class STREAMING_EXPORT fwArchetype
{
public:
	virtual ~fwArchetype() = default;

	virtual void m_8() = 0;

	virtual void InitializeFromArchetypeDef(uint32_t mapTypesStoreIdx, fwArchetypeDef* archetypeDef, bool) = 0;

	virtual fwEntity* CreateEntity() = 0;

public:
	char pad[16];
	uint32_t hash;
	char pad2[16];
	float radius;
	float aabbMin[4];
	float aabbMax[4];
	uint32_t flags;

	uint8_t pad3[12];
	uint8_t assetType;
	uint8_t pad4;

	uint32_t assetIndex;
};

class STREAMING_EXPORT fwEntityDef
{
public:
	virtual ~fwEntityDef();

	virtual int64_t GetTypeIdentifier();

public:
	uint32_t archetypeName;
	uint32_t flags;
	uint32_t guid;

	uint32_t pad[3];

	float position[4];
	float rotation[4];

	float scaleXY;
	float scaleZ;

	int32_t parentIndex;

	float lodDist;
	float childLodDist;

	int32_t lodLevel;
	int32_t numChildren;

	int32_t priorityLevel;

	int32_t pad2[4];
	int32_t ambientOcclusionMultiplier;
	int32_t artificialAmbientOcclusion;
	int32_t pad3[2];

public:
	fwEntityDef()
	{
		flags = 0x180000; // was 0x180010
		parentIndex = -1;
		scaleXY = 1.0f;
		scaleZ = 1.0f;
		lodDist = 4000.f;
		childLodDist = 500.f;
		lodLevel = 2;
		numChildren = 9;
		ambientOcclusionMultiplier = 0xFF;
		artificialAmbientOcclusion = 0xFF;
		priorityLevel = 0;

		memset(pad, 0, sizeof(pad));
		memset(pad2, 0, sizeof(pad2));
		memset(pad3, 0, sizeof(pad3));
	}
};

extern STREAMING_EXPORT atArray<fwFactoryBase<fwArchetype>*>* g_archetypeFactories;

class STREAMING_EXPORT fwEntity : public rage::fwRefAwareBase
{
public:
	virtual ~fwEntity() = default;

	virtual bool IsOfType(uint32_t hash) = 0;

	template<typename T>
	bool IsOfType()
	{
		return reinterpret_cast<T*>(this->IsOfType(HashString(boost::typeindex::type_id<T>().pretty_name().substr(6).c_str())));
	}

	virtual void m_10() = 0;

	virtual void m_18() = 0;

	virtual void m_20() = 0;

	virtual void m_28() = 0;

	virtual void m_30() = 0;

	virtual void SetupFromEntityDef(fwEntityDef* entityDef, fwArchetype* archetype, uint32_t) = 0;

	virtual void SetModelIndex(uint32_t* mi) = 0;

	virtual void m_48() = 0;
	virtual void m_50() = 0;
	virtual void m_58() = 0;
	virtual void m_60() = 0;
	virtual void m_68() = 0;
	virtual void m_70() = 0;
	virtual void m_78() = 0;
	virtual void m_80() = 0;
	virtual void m_88() = 0;
	virtual void m_90() = 0;
	virtual void m_98() = 0;
	virtual void m_a0() = 0;
	virtual void m_a8() = 0;
	virtual void m_b0() = 0;
	virtual void SetTransform(const Matrix4x4& matrix, bool updateScene) = 0;
	virtual void UpdateTransform(const Matrix4x4& matrix, bool updateScene) = 0;
	virtual void m_c8() = 0;
	virtual void m_d0() = 0;
	virtual void m_d8() = 0;
	virtual void m_e0() = 0;
	virtual void m_e8() = 0;
	virtual void m_f0() = 0;
	virtual void m_f8() = 0;
	virtual void m_100() = 0;
	virtual void m_108() = 0;
	virtual void AddToSceneWrap() = 0;
	virtual void AddToScene() = 0;
	virtual void RemoveFromScene() = 0; // ?
	virtual void m_128() = 0;
	virtual void m_130() = 0;
	virtual void m_138() = 0;
	virtual void m_140() = 0;
	virtual void m_148() = 0;
	virtual void m_150() = 0;
	virtual void m_158() = 0;
	virtual void m_160() = 0;
	virtual void m_168() = 0;
	virtual void m_170() = 0;
	virtual void m_178() = 0;
	virtual void m_180() = 0;
	virtual void m_188() = 0;
	virtual float GetRadius() = 0;

public:
	inline const Matrix4x4& GetTransform() const
	{
		return m_transform;
	}

	inline Vector3 GetPosition() const
	{
		return Vector3(m_transform._41, m_transform._42, m_transform._43);
	}

	inline void* GetNetObject() const
	{
		return m_netObject;
	}

private:
	char m_pad[96 - 8];
	Matrix4x4 m_transform;
	char m_pad2[48];
	void* m_netObject;
};

STREAMING_EXPORT class VehicleSeatManager
{
public:
	inline int GetNumSeats()
	{
		return m_numSeats;
	}

	inline fwEntity* GetOccupant(uint32_t index)
	{
		if (index > _countof(m_occupants))
		{
			return nullptr;
		}

		return m_occupants[index];
	}

private:
	uint8_t m_numSeats;

	fwEntity* m_occupants[16];
};

class STREAMING_EXPORT CVehicle : public fwEntity
{
public:
	VehicleSeatManager* GetSeatManager();
};

struct PopulationCreationState
{
	float position[3];
	uint32_t model;
	bool allowed;
};

STREAMING_EXPORT extern fwEvent<PopulationCreationState*> OnCreatePopulationPed;

struct GameEventMetaData
{
	char name[256];
	size_t numArguments;
	uintptr_t arguments[48];
};

STREAMING_EXPORT extern fwEvent<const GameEventMetaData&> OnTriggerGameEvent;

struct CMapDataContents
{
	virtual ~CMapDataContents() = 0;
	virtual void Add() = 0;
	virtual void Remove() = 0;
	virtual void PrepareInteriors(void* meta, void* data, uint32_t id) = 0;

	void* sceneNodes;
	void** entities;
	uint32_t numEntities;
};

struct STREAMING_EXPORT CMapData : rage::sysUseAllocator
{
	CMapData();
	virtual ~CMapData() = default;

	virtual CMapDataContents* CreateMapDataContents() { assert(false); return nullptr; };

	uint32_t name; // +8
	uint32_t parent; // +12
	int32_t flags; // +16
	int32_t contentFlags; // +20
	alignas(16) float streamingExtentsMin[4]; // +32
	alignas(16) float streamingExtentsMax[4]; // +48
	alignas(16) float entitiesExtentsMin[4]; // +64
	alignas(16) float entitiesExtentsMax[4]; // +72
	atArray<fwEntityDef*> entities;

	char pad[512 - 104];

	// etc.
};
