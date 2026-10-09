#pragma once

#include <directxmath.h>

#ifdef COMPILING_GTA_STREAMING_RDR3
#define STREAMING_EXPORT DLL_EXPORT
#else
#define STREAMING_EXPORT DLL_IMPORT
#endif

using Vector3 = DirectX::XMFLOAT3;
using Matrix3x4 = DirectX::XMFLOAT3X4;

class fwEntity;

class STREAMING_EXPORT fwArchetype
{
public:
	virtual ~fwArchetype() = default;

public:
	void* dynamicArchetypeComponent;
	char pad[16];
	float bsCenter[3];
	float radius;
	float aabbMin[4];
	float aabbMax[4];
	char pad2[16];
	uint32_t hash;
	char pad3[12];
};

namespace rage
{
struct fwModelId
{
	union
	{
		uint32_t value;

		struct
		{
			uint16_t modelIndex;
			uint16_t mapTypesIndex : 12;
			uint16_t flags : 4;
		};
	};

	fwModelId()
		: modelIndex(0xFFFF), mapTypesIndex(0xFFF), flags(0)
	{
	}

	fwModelId(uint32_t idx)
		: value(idx)
	{
	}
};

static_assert(sizeof(fwModelId) == 4);

class STREAMING_EXPORT fwArchetypeManager
{
public:
	static fwArchetype* GetArchetypeFromHashKey(uint32_t hash, fwModelId& id);
};

class STREAMING_EXPORT fwRefAwareBase
{
public:
	~fwRefAwareBase() = default;

public:
	void AddKnownRef(void** ref) const;

	void RemoveKnownRef(void** ref) const;
};

class STREAMING_EXPORT fwExtension
{
public:
	// id +0, category +8, destructor +10h, init extensions +18h and +20h
	virtual int GetExtensionId() const = 0;

	// 2 for extensions on an entity
	virtual int GetCategoryId() const
	{
		return 2;
	}

	virtual ~fwExtension() = default;

	virtual void InitEntityExtensionFromDefinition(const void* extensionDef, ::fwEntity* entity)
	{
	}

	virtual void InitArchetypeExtensionFromDefinition(const void* extensionDef, ::fwArchetype* archetype)
	{
	}
};

class STREAMING_EXPORT fwExtensionList
{
public:
	// Nodes are allocated by Add; the list only keeps the head.
	struct Node
	{
		rage::fwExtension* extension; // +0
		Node* next; // +8
	};

	void Add(rage::fwExtension* extension);

	void* Get(uint32_t id);

private:
	Node* m_extensions; // +0, head of the list
	uint32_t m_identifiers[2]; // +8, bitmask used to skip the walk for ids below 62
};

class STREAMING_EXPORT fwScriptGuid
{
public:
	static fwEntity* GetBaseFromGuid(int handle);
};

using fwEntity = ::fwEntity;

struct PreciseTransform : Matrix3x4
{
	struct
	{
		float offsetX, offsetY, z;
		int16_t sectorX, sectorY;
	} position;
};
}

class STREAMING_EXPORT fwEntity : public rage::fwRefAwareBase
{
public:
	virtual ~fwEntity() = default;

	virtual bool IsOfType(uint32_t hash) = 0;

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

#define FORWARD_FUNC(name, offset, ...) \
	using TFn = decltype(&fwEntity::name); \
	void** vtbl = *(void***)(this); \
	return (this->*(get_member<TFn>(vtbl[(offset / 8)])))(__VA_ARGS__);

public:
	inline float GetRadius()
	{
		FORWARD_FUNC(GetRadius, 0x200);
	}

public:
	inline const rage::PreciseTransform& GetTransform() const
	{
		return m_transform;
	}
	
	inline Vector3 GetPosition() const
	{
		return Vector3(
			m_transform.position.sectorX * 32 + m_transform.position.offsetX,
			m_transform.position.sectorY * 32 + m_transform.position.offsetY,
			m_transform.position.z
		);
	}

	inline void* GetNetObject() const
	{
		static_assert(offsetof(fwEntity, m_netObject) == 224, "wrong GetNetObject");
		return m_netObject;
	}

	inline uint8_t GetType() const
	{
		return m_entityType;
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

private:
	char m_pad[8]; // +8
	rage::fwExtensionList m_extensionList; // +16, fwEntity::InitExtensionsFromDefinition passes +0x10
	fwArchetype* m_archetype; // +32
	char m_pad2[8]; // +40
	uint8_t m_entityType; // +48
	char m_pad3[15]; // +49
	rage::PreciseTransform m_transform; // +64
	char m_pad4[96]; // +128
	void* m_netObject; // +224
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

class CPickup : public fwEntity
{
};

class CObject : public fwEntity
{
};

class CVehicle : public fwEntity
{
};

class CPed : public fwEntity
{
};

struct PopulationCreationState
{
	float position[3];
	uint32_t model;
	bool allowed;
};

STREAMING_EXPORT extern fwEvent<PopulationCreationState*> OnCreatePopulationPed;
