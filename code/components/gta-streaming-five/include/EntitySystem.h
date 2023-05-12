#pragma once

#include <atArray.h>
#include <atPool.h>

#include <Pool.h>

#include <boost/type_index/ctti_type_index.hpp>

#include <directxmath.h>

#include <CrossBuildRuntime.h>
#include "XBRVirtual.h"

template<int Offset>
inline int MapEntityMethod()
{
	int offset = Offset;

	if (offset >= 0x18 && xbr::IsGameBuildOrGreater<2802>())
	{
		offset += 0x20;
	}
	else if (offset >= 0x10 && xbr::IsGameBuildOrGreater<2189>())
	{
		offset += 0x8;
	}

	return offset;
}

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
		virtual ~fwRefAwareBase() = default;

	public:
		void AddKnownRef(void** ref);

		void RemoveKnownRef(void** ref);
	};

	class STREAMING_EXPORT fwScriptGuid
	{
	public:
		static fwEntity* GetBaseFromGuid(int handle);

		static int GetGuidFromBase(fwEntity* base);
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

class STREAMING_EXPORT fwArchetypeDef : XBR_VIRTUAL_BASE_2802(0)
{
public:
	XBR_VIRTUAL_DTOR(fwArchetypeDef)

	XBR_VIRTUAL_METHOD(int64_t, GetTypeIdentifier, ())

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

class fwDynamicArchetypeComponent
{
public:
	inline bool IsPhysicsObject()
	{
		return (flags & 0x8000);
	}

private:
	uint8_t pad[42];
	uint16_t flags;
};

class STREAMING_EXPORT fwArchetype : XBR_VIRTUAL_BASE_2802(0)
{
public:
	XBR_VIRTUAL_DTOR(fwArchetype)

	XBR_VIRTUAL_METHOD(void, m_8, ())

	XBR_VIRTUAL_METHOD(void, InitializeFromArchetypeDef, (uint32_t mapTypesStoreIdx, fwArchetypeDef* archetypeDef, bool))

	XBR_VIRTUAL_METHOD(fwEntity*, CreateEntity, ())

	inline bool HasEmbeddedCollision()
	{
		// fragments are inherently colliding
		if (assetType == 1)
		{
			return true;
		}

		// if we have a dynamic archetype component, and it came from physicsDictionary
		return (dynamicArchetypeComponent && dynamicArchetypeComponent->IsPhysicsObject());
	}

public:
	char pad[16];
	uint32_t hash;
	char pad2[16];
	float radius;
	float aabbMin[4];
	float aabbMax[4];
	uint32_t flags;

	uint8_t pad3[4];
	fwDynamicArchetypeComponent* dynamicArchetypeComponent;
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
using fwArchetype = ::fwArchetype;

struct fwModelId
{
	uint64_t id;
};

class STREAMING_EXPORT fwArchetypeManager
{
public:
	static fwArchetype* GetArchetypeFromHashKey(uint32_t hash, fwModelId& id);
};
}

namespace rage
{
class parStructure;
}

class STREAMING_EXPORT fwExtensionDef : XBR_VIRTUAL_BASE_2802(0)
{
public:
	XBR_VIRTUAL_DTOR(fwExtensionDef)

	XBR_VIRTUAL_METHOD(void*, parser_GetStructure, ())

	uint32_t name;
};

class STREAMING_EXPORT fwExtensionDefImplOld
{
public:
	virtual ~fwExtensionDefImplOld() = default;

	virtual void* parser_GetStructure() = 0;

	uint32_t name;
};

class STREAMING_EXPORT fwExtensionDefImpl2802
{
private:
	inline virtual void* _2802_1()
	{
		return nullptr;
	}

	inline virtual void* _2802_2()
	{
		return nullptr;
	}

	inline virtual void* _2802_3()
	{
		return nullptr;
	}

	inline virtual void* _2802_4()
	{
		return nullptr;
	}

	inline virtual void* _2802_5()
	{
		return nullptr;
	}

	inline virtual void* _2802_6()
	{
		return nullptr;
	}

public:
	virtual ~fwExtensionDefImpl2802() = default;

	virtual void* parser_GetStructure() = 0;

	uint32_t name;
};

template<int Build>
class fwExtensionDefImpl : public std::conditional_t<Build >= 2802, fwExtensionDefImpl2802, fwExtensionDefImplOld>
{
};

class STREAMING_EXPORT fwEntityDef : XBR_VIRTUAL_BASE_2802(0)
{
public:
	XBR_VIRTUAL_DTOR(fwEntityDef)

	XBR_VIRTUAL_METHOD(rage::parStructure*, GetTypeIdentifier, ())

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
	atArray<fwExtensionDef*> extensions;

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
		priorityLevel = 0;

		memset(pad, 0, sizeof(pad));
	}
};

class STREAMING_EXPORT CEntityDef : fwEntityDef
{
public:
	int32_t ambientOcclusionMultiplier;
	int32_t artificialAmbientOcclusion;
	uint32_t tintValue;

	CEntityDef()
		: fwEntityDef()
	{
		ambientOcclusionMultiplier = 0xFF;
		artificialAmbientOcclusion = 0xFF;
		tintValue = 0;
	}
};

extern STREAMING_EXPORT atArray<fwFactoryBase<fwArchetype>*>* g_archetypeFactories;

namespace rage
{
class STREAMING_EXPORT fwExtension
{
public:
	virtual ~fwExtension() = default;

	virtual void InitEntityExtensionFromDefinition(const void* extensionDef, rage::fwEntity* entity)
	{
	}

	virtual void InitArchetypeExtensionFromDefinition(const void* extensionDef, rage::fwArchetype* entity)
	{
	}

	virtual int GetExtensionId() const = 0;
};
}

class STREAMING_EXPORT fwExtensionList
{
public:
	void Add(rage::fwExtension* extension);

	void* Get(uint32_t id);

private:
	uintptr_t dummyVal;
};

class STREAMING_EXPORT fwEntity : public rage::fwRefAwareBase
{
public:
	virtual ~fwEntity() = default;

	inline bool IsOfType(uint32_t hash)
	{
		return IsOfTypeH(hash);
	}

	inline fwArchetype* GetArchetype()
	{
		return m_archetype;
	}

	inline void* GetExtension(uint32_t id)
	{
		return m_extensionList.Get(id);
	}

	inline void AddExtension(rage::fwExtension* extension)
	{
		return m_extensionList.Add(extension);
	}

	template<typename T>
	inline T* GetExtension()
	{
		return reinterpret_cast<T*>(GetExtension(typename T::GetClassId()));
	}

	template<typename T>
	bool IsOfType()
	{
		constexpr auto typeName = std::string_view{
			boost::typeindex::ctti_type_index::type_id<T>().raw_name()
		};

		constexpr auto typeHash = HashString(typeName.substr(0, typeName.length() - boost::typeindex::detail::ctti_skip_size_at_end).substr(6));
		return this->IsOfType(typeHash);
	}

private:
	template<typename TMember>
	inline static TMember get_member(void* ptr)
	{
		union member_cast
		{
			TMember function;
			struct  
			{
				void* ptr;
				uintptr_t off;
			};
		};

		member_cast cast;
		cast.ptr = ptr;
		cast.off = 0;

		return cast.function;
	} 

public:

#undef FORWARD_FUNC
#define FORWARD_FUNC(name, offset, ...) \
	using TFn = decltype(&fwEntity::name); \
	void** vtbl = *(void***)(this); \
	return (this->*(get_member<TFn>(vtbl[MapEntityMethod<offset>() / 8])))(__VA_ARGS__);

private:
	inline bool IsOfTypeH(uint32_t hash)
	{
		if (xbr::IsGameBuildOrGreater<2802>())
		{
			return (GetTypeHash() == hash);
		}

		if (xbr::IsGameBuildOrGreater<2189>())
		{
			return IsOfTypeRef(hash);
		}

		FORWARD_FUNC(IsOfTypeH, 0x8, hash);
	}

	inline bool IsOfTypeRef(const uint32_t& hash)
	{
		if (xbr::IsGameBuildOrGreater<2802>())
		{
			return (GetTypeHash() == hash);
		}

		FORWARD_FUNC(IsOfTypeRef, 0x8, hash);
	}

private:
	inline uint32_t GetTypeHash()
	{
		if (!xbr::IsGameBuildOrGreater<2802>())
		{
			assert(false);
		}

		// #TODO2802: new RTTI method, a bit weird but works, definitely need to make it less dirty at some point...
		return (*(uint32_t(__fastcall**)(char*))(*(char**)this + 0x10))((char*)this);
	}

public:
	inline void SetupFromEntityDef(fwEntityDef* entityDef, fwArchetype* archetype, uint32_t a3)
	{
		FORWARD_FUNC(SetupFromEntityDef, 0x38, entityDef, archetype, a3);
	}

	inline void SetModelIndex(uint32_t* mi)
	{
		FORWARD_FUNC(SetModelIndex, 0x40, mi);
	}

	inline void SetTransform(const Matrix4x4& matrix, bool updateScene)
	{
		FORWARD_FUNC(SetTransform, 0xb8, matrix, updateScene);
	}

	inline void UpdateTransform(const Matrix4x4& matrix, bool updateScene, bool moreUpdate = false, bool evenMoreUpdate = false)
	{
		FORWARD_FUNC(UpdateTransform, 0xc0, matrix, updateScene, moreUpdate, evenMoreUpdate);
	}

	inline void AddToSceneWrap()
	{
		FORWARD_FUNC(AddToSceneWrap, 0x110);
	}

	inline void AddToScene()
	{
		FORWARD_FUNC(AddToScene, 0x118);
	}

	inline void RemoveFromScene()
	{
		FORWARD_FUNC(AddToSceneWrap, 0x120);
	}

	inline float GetRadius()
	{
		FORWARD_FUNC(GetRadius, 0x190);
	}

#undef FORWARD_FUNC

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

	inline uint8_t GetType() const
	{
		return m_entityType;
	}

private:
	char m_pad[8];
	fwExtensionList m_extensionList;
	char m_pad2[8];
	fwArchetype* m_archetype;
	uint8_t m_entityType;
	char m_pad3[96 - 41];
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

struct CHandlingObject
{
	// boost::typeindex::ctti_type_index doesn't expose a string_view :(
	static constexpr uint32_t kHash = HashString("CHandlingObject");
};

template<typename TPoolName>
struct PoolAllocated
{
public:
	inline void* operator new(size_t size)
	{
		return rage::PoolAllocate(rage::GetPoolBase(TPoolName::kHash));
	}

	inline void* operator new[](size_t size)
	{
		return PoolAllocated::operator new(size);
	}

	inline void operator delete(void* memory)
	{
		rage::PoolRelease(rage::GetPoolBase(TPoolName::kHash), memory);
	}

	inline void operator delete[](void* memory)
	{
		return PoolAllocated::operator delete(memory);
	}
};

class CBaseSubHandlingData : public PoolAllocated<CHandlingObject>
{
public:
	virtual ~CBaseSubHandlingData() = default;
	virtual void* GetParser() = 0;
	virtual int GetTypeIndex() = 0;
	virtual void ProcessOnLoad() = 0;
};

struct CAdvancedData : public rage::sysUseAllocator
{
	void* vtbl;
	uint8_t pad[16];
};

class CCarHandlingData2060 : public CBaseSubHandlingData
{
private:
	char pad[64 - 8];

public:
	atArray<CAdvancedData> AdvancedData;
};

struct BoardingPointData : public rage::sysUseAllocator
{
	uint8_t m_pad[132];
};

class CHandlingData : public PoolAllocated<CHandlingObject>
{
private:
	uint32_t m_name;
	char m_pad[316]; // 1290, 1365, 1493, 1604, 1868, 2060
	BoardingPointData* m_boardingPoints;
	char m_pad2[8];
	atArray<CBaseSubHandlingData*> m_subHandlingData; // +344?
	// ^ find offset using a variant of 48 85 C9 74 13 BA 04 00 00 00 E8 (and go to the call in there)
	char m_pad3[1000];

public:
	CHandlingData(CHandlingData* orig)
	{
		static_assert(offsetof(CHandlingData, m_boardingPoints) == 328, "m_boardingPoints");
		static_assert(offsetof(CHandlingData, m_subHandlingData) == 344, "m_subHandlingData");

		static auto pool = rage::GetPoolBase(CHandlingObject::kHash);
		memcpy(this, orig, pool->GetEntrySize());

		CBaseSubHandlingData* shds[6] = { 0 };

		for (int i = 0; i < m_subHandlingData.GetCount(); i++)
		{
			if (m_subHandlingData.Get(i))
			{
				shds[i] = (CBaseSubHandlingData*)rage::PoolAllocate(pool);
				memcpy(shds[i], m_subHandlingData.Get(i), pool->GetEntrySize());

				if (xbr::IsGameBuildOrGreater<2060>())
				{
					// CCarHandlingData
					if (shds[i]->GetTypeIndex() == 8)
					{
						auto origChd = static_cast<CCarHandlingData2060*>(m_subHandlingData.Get(i));
						auto chd = static_cast<CCarHandlingData2060*>(shds[i]);

						chd->AdvancedData.m_offset = 0;
						chd->AdvancedData = origChd->AdvancedData;
					}
				}
			}
		}

		m_subHandlingData.m_offset = nullptr;
		m_subHandlingData.Clear();
		m_subHandlingData.Expand(6);

		m_subHandlingData.Set(0, shds[0]);
		m_subHandlingData.Set(1, shds[1]);
		m_subHandlingData.Set(2, shds[2]);
		m_subHandlingData.Set(3, shds[3]);
		m_subHandlingData.Set(4, shds[4]);
		m_subHandlingData.Set(5, shds[5]);

		if (orig->m_boardingPoints)
		{
			m_boardingPoints = new BoardingPointData();
			memcpy(m_boardingPoints, orig->m_boardingPoints, sizeof(*m_boardingPoints));
		}
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
		Impl<0x918> m2802;
	} impl;

public:
	virtual ~CVehicle() = default;

	inline CHandlingData* GetHandlingData()
	{
		if (xbr::IsGameBuildOrGreater<2802>())
		{
			return impl.m2802.m_handlingData;
		}
		else if (xbr::IsGameBuildOrGreater<2060>())
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
		
		if (xbr::IsGameBuildOrGreater<2802>())
		{
			impl.m2802.m_handlingData = ptr;
		}
		else if (xbr::IsGameBuildOrGreater<2060>())
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
	const char* name;
	size_t numArguments;
	size_t arguments[48];
};

STREAMING_EXPORT extern fwEvent<const GameEventMetaData&> OnTriggerGameEvent;

struct GameEventData
{
	const char* name;
	std::string_view argsData;
};

STREAMING_EXPORT extern fwEvent<const GameEventData&> OnTriggerGameEventExt;

struct DamageEventMetaData
{
	rage::fwEntity* victim;
	rage::fwEntity* culprit;
	float baseDamage;
	uint32_t weapon;
};

STREAMING_EXPORT extern fwEvent<const DamageEventMetaData&> OnEntityDamaged;

class CMapData;

struct CMapDataContents : XBR_VIRTUAL_BASE_2802(0)
{
	XBR_VIRTUAL_DTOR(CMapDataContents)
	XBR_VIRTUAL_METHOD(void, Add, ())
	XBR_VIRTUAL_METHOD(void, Remove, (int id))
	XBR_VIRTUAL_METHOD(void, PrepareInteriors, (void* meta, void* data, uint32_t id))

	// 8
	void* sceneNodes;
	// 16
	void** entities;
	// 24
	uint32_t numEntities;
	// 28
	uint8_t pad[12];
	CMapData* mapData;
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

struct STREAMING_EXPORT CMapTypes : rage::sysUseAllocator
{
	CMapTypes();
	virtual ~CMapTypes() = default;

	atArray<fwExtensionDef*> extensions;
	atArray<fwArchetypeDef*> archetypes;

	uint32_t name;

	// atArray<???> dependencies;
	// atArray<CCompositeEntityType*> compositeEntityTypes;
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
