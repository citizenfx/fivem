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
};

namespace rage
{
struct fwModelId
{
	uint64_t id;
};

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

private:
	char m_pad[40]; // +8
	uint8_t m_entityType; // +48
	char m_pad2[15]; // +49
	rage::PreciseTransform m_transform; // +64
	char m_pad3[96]; // +128
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