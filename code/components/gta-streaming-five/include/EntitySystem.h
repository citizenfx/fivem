#pragma once

#include <atArray.h>

#include <directxmath.h>

#include <CrossBuildRuntime.h>

#ifdef COMPILING_GTA_STREAMING_FIVE
#define STREAMING_EXPORT DLL_EXPORT
#else
#define STREAMING_EXPORT DLL_IMPORT
#endif

using Vector3 = DirectX::XMFLOAT3;
using Matrix4x4 = DirectX::XMFLOAT4X4;

class fwEntity;

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

	class STREAMING_EXPORT fwScriptGuid
	{
	public:
		static fwEntity* GetBaseFromGuid(int handle);
	};

	using fwEntity = ::fwEntity;
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

class STREAMING_EXPORT fwSceneUpdateExtension
{
public:
	virtual ~fwSceneUpdateExtension() = default;

	static uint32_t GetClassId();

	inline uint32_t GetUpdateFlags()
	{
		return m_updateFlags;
	}

private:
	void* m_entity;
	uint32_t m_updateFlags;
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

	// +100
	uint32_t assetIndex;

	// +104
	char m_pad[53];
	uint8_t miType : 5;
};

namespace rage
{
class parStructure;
}

class STREAMING_EXPORT fwEntityDef
{
public:
	virtual ~fwEntityDef();

	virtual rage::parStructure* GetTypeIdentifier();

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

class STREAMING_EXPORT fwExtensionList
{
public:
	void* Get(uint32_t id);

private:
	uintptr_t dummyVal;
};

class STREAMING_EXPORT fwEntity : public rage::fwRefAwareBase
{
public:
	virtual ~fwEntity() = default;

	virtual bool IsOfType(uint32_t hash) = 0;

	inline fwArchetype* GetArchetype()
	{
		return m_archetype;
	}

	inline void* GetExtension(uint32_t id)
	{
		return m_extensionList.Get(id);
	}

	template<typename T>
	inline T* GetExtension()
	{
		return reinterpret_cast<T*>(GetExtension(typename T::GetClassId()));
	}

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
		static_assert(offsetof(fwEntity, m_netObject) == 208, "wrong GetNetObject");
		return m_netObject;
	}

private:
	char m_pad[8];
	fwExtensionList m_extensionList;
	char m_pad2[8];
	fwArchetype* m_archetype;
	char m_pad3[96 - 40];
	Matrix4x4 m_transform;
	char m_pad4[48];
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


class CBaseSubHandlingData
{
public:
	virtual ~CBaseSubHandlingData() = default;
	virtual void* GetParser() = 0;
	virtual int GetUnk() = 0;
	virtual void ProcessOnLoad() = 0;
};

class CHandlingData
{
private:
	uint32_t m_name;
	char m_pad[332]; // 1290, 1365, 1493, 1604, 1868, 2060
	atArray<CBaseSubHandlingData*> m_subHandlingData;
	// ^ find offset using a variant of 48 85 C9 74 13 BA 04 00 00 00 E8 (and go to the call in there)
	char m_pad2[1000];

public:
	CHandlingData(CHandlingData* orig)
	{
		memcpy(this, orig, sizeof(*this));

		CBaseSubHandlingData* shds[6] = { 0 };

		for (int i = 0; i < m_subHandlingData.GetCount(); i++)
		{
			if (m_subHandlingData.Get(i))
			{
				shds[i] = (CBaseSubHandlingData*)rage::GetAllocator()->allocate(1024, 16, 0);
				memcpy(shds[i], m_subHandlingData.Get(i), 1024);
			}
		}

		m_subHandlingData.m_offset = nullptr;
		m_subHandlingData.Clear();

		m_subHandlingData.Set(0, shds[0]);
		m_subHandlingData.Set(1, shds[1]);
		m_subHandlingData.Set(2, shds[2]);
		m_subHandlingData.Set(3, shds[3]);
		m_subHandlingData.Set(4, shds[4]);
		m_subHandlingData.Set(5, shds[5]);
	}

	virtual ~CHandlingData() = default;

	inline uint32_t GetName()
	{
		return m_name;
	}

	inline atArray<CBaseSubHandlingData*>& GetSubHandlingData()
	{
		return m_subHandlingData;
	}

	void ProcessEntry();
};

class STREAMING_EXPORT CVehicle : public fwEntity
{
private:
	template<int HandlingStart>
	struct Impl
	{
		//char m_pad[0x8C0]; // 1290, 1365, 1493
		char m_pad[HandlingStart - sizeof(fwEntity)]; // 1604, 1737, 1868
		CHandlingData* m_handlingData;
		// find ^ with `85 C0 74 49 48 8B 86 ? ? 00 00 48 8B CE` ??s (before 1604)
		// 1604+: `75 4D 48 8B 86 ? ? ? ? F6 80`
	};

	union
	{
		Impl<0x918> m1604;
		Impl<0x938> m2060;
	} impl;

public:
	virtual ~CVehicle() = default;

	inline CHandlingData* GetHandlingData()
	{
		if (Is2060())
		{
			return impl.m2060.m_handlingData;
		}
		else
		{
			return impl.m1604.m_handlingData;
		}
	}

	inline void SetHandlingData(CHandlingData* ptr)
	{
		// Use an alignment byte within CHandlingDataMgr to represent the handling as hooked.
		*((char*)ptr + 28) = 1;
		
		if (Is2060())
		{
			impl.m2060.m_handlingData = ptr;
		}
		else
		{
			impl.m1604.m_handlingData = ptr;
		}
	}

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

struct MapDataVec4
{
	float values[4];

	inline float& operator[](int i)
	{
		return values[i];
	}
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

	char padAlign[8]; // we can **not** use alignas here as it'll align `name` and such too

	MapDataVec4 streamingExtentsMin; // +32
	MapDataVec4 streamingExtentsMax; // +48
	MapDataVec4 entitiesExtentsMin; // +64
	MapDataVec4 entitiesExtentsMax; // +72
	atArray<fwEntityDef*> entities;

	char pad[512 - 104];

	// etc.
};

namespace rage
{
class fwInteriorLocation
{
public:
	inline fwInteriorLocation()
	{
		m_interiorIndex = -1;
		m_isPortal = false;
		m_unk = false;
		m_innerIndex = -1;
	}

	inline fwInteriorLocation(uint16_t interiorIndex, bool isPortal, uint16_t innerIndex)
		: fwInteriorLocation()
	{
		m_interiorIndex = interiorIndex;
		m_isPortal = isPortal;
		m_innerIndex = innerIndex;
	}

	inline uint16_t GetInteriorIndex()
	{
		return m_interiorIndex;
	}

	inline uint16_t GetRoomIndex()
	{
		assert(!m_isPortal);

		return m_innerIndex;
	}

	inline uint16_t GetPortalIndex()
	{
		assert(m_isPortal);

		return m_innerIndex;
	}

	inline bool IsPortal()
	{
		return m_isPortal;
	}

private:
	uint16_t m_interiorIndex;
	uint16_t m_isPortal : 1;
	uint16_t m_unk : 1;
	uint16_t m_innerIndex : 14;
};
}
