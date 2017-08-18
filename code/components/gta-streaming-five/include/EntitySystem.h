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

	float drawDistance;
	uint32_t flags; // 0x10000 = alphaclip
	uint32_t unkField; // lower 5 bits == 31 -> use alpha clip, get masked to 31 in InitializeFromArchetypeDef
	uint32_t pad;
	void* pad2;
	float boundingBoxMin[4];
	float boundingBoxMax[4];
	float centroid[4];
	float radius;
	float unkDistance;
	uint32_t nameHash;
	uint32_t txdHash;
	uint32_t pad3;
	uint32_t dwdHash;
	uint32_t pad4;
	uint32_t unk_3;
	uint32_t unkHash;
	uint32_t pad5[7];

public:
	fwArchetypeDef()
	{
		flags = 0x10000; // was 0x2000
		drawDistance = 299.0f;
		unkDistance = 375.0f;

		dwdHash = 0;
		unk_3 = 3;
		unkHash = 0x12345678;

		unkField = 31;

		pad = 0;
		pad2 = 0;
		pad3 = 0;
		pad4 = 0;
		memset(pad5, 0, sizeof(pad4));
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
	char pad[36];
	float radius;
	float aabbMin[4];
	float aabbMax[4];
	uint32_t flags;

	uint8_t pad2[12];
	uint8_t assetType;
	uint8_t pad3;

	uint16_t assetIndex;
};

class STREAMING_EXPORT fwEntityDef
{
public:
	virtual ~fwEntityDef();

	virtual int64_t GetTypeIdentifier();

public:
	uint32_t archetypeNameHash;
	uint32_t flags;
	uint32_t guidHash;

	uint32_t pad[3];

	float position[4];
	float rotation[4];

	float float1;
	float float2;

	int32_t lodParentIdx;

	float unkFloat1;
	float unkFloat2;

	int32_t unkInt1;
	int32_t unkInt2;

	int32_t pad2[5];
	int32_t unkFF;
	int32_t unkFF_2;
	int32_t pad3[2];

public:
	fwEntityDef()
	{
		flags = 0x180000; // was 0x180010
		lodParentIdx = -1;
		float1 = 1.0f;
		float2 = 1.0f;
		unkFloat1 = 4000.f;
		unkFloat2 = 500.f;
		unkInt1 = 2;
		unkInt2 = 9;
		unkFF = 0xFF;
		unkFF_2 = 0xFF;

		memset(pad, 0, sizeof(pad));
		memset(pad2, 0, sizeof(pad2));
		memset(pad3, 0, sizeof(pad3));
	}
};

extern STREAMING_EXPORT atArray<fwFactoryBase<fwArchetype>*>* g_archetypeFactories;

class STREAMING_EXPORT fwEntity
{
public:
	virtual ~fwEntity() = default;

	virtual void m_8() = 0;

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

public:
	inline const Matrix4x4& GetTransform() const
	{
		return m_transform;
	}

	inline const Vector3& GetPosition() const
	{
		return Vector3(m_transform._41, m_transform._42, m_transform._43);
	}

private:
	char m_pad[96 - 8];
	Matrix4x4 m_transform;
};
