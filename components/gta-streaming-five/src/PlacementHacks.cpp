/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

#include <atArray.h>

#include <boost/preprocessor.hpp>

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

class fwArchetypeDef
{
public:
	virtual ~fwArchetypeDef();

	virtual int64_t GetTypeIdentifier();

	float drawDistance;
	uint32_t flags;
	void* pad[2];
	float boundingBoxMin[4];
	float boundingBoxMax[4];
	float centroid[4];
	float radius;
	float unkDistance;
	uint32_t nameHash;
	uint32_t txdHash;
	uint32_t pad2;
	uint32_t dwdHash;
	uint32_t pad3;
	uint32_t unk_3;
	uint32_t unkHash;
	uint32_t pad4[7];

public:
	fwArchetypeDef()
	{
		flags = 0x2000;
		drawDistance = 299.0f;
		unkDistance = 375.0f;

		dwdHash = 0;
		unk_3 = 3;
		unkHash = 0x12345678;

		memset(pad, 0, sizeof(pad));
		pad2 = 0;
		pad3 = 0;
		memset(pad4, 0, sizeof(pad4));
	}
};

fwArchetypeDef::~fwArchetypeDef()
{

}

int64_t* g_baseArchetypeDefId;

int64_t fwArchetypeDef::GetTypeIdentifier()
{
	return *g_baseArchetypeDefId;
}

//static_assert(sizeof(fwArchetypeDef) == 144, "fwArchetypeDef isn't of CBaseArchetypeDef's size...");

class fwArchetype
{
public:
	virtual ~fwArchetype() = 0;

	virtual void m_8() = 0;

	virtual void InitializeFromArchetypeDef(uint32_t nameHash, fwArchetypeDef* archetypeDef, bool) = 0;
};

class fwEntityDef
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

	uint32_t unkFloat1;
	uint32_t unkFloat2;

	int32_t unkInt1;
	int32_t unkInt2;

	int32_t pad2[5];
	int32_t unkFF;
	int32_t unkFF_2;
	int32_t pad3[2];

public:
	fwEntityDef()
	{
		flags = 0x180010;
		lodParentIdx = -1;
		float1 = 1.0f;
		float2 = 1.0f;
		unkFloat1 = 0x457a0000;
		unkFloat2 = 0x43fa0000;
		unkInt1 = 2;
		unkInt2 = 9;
		unkFF = 0xFF;
		unkFF_2 = 0xFF;

		memset(pad, 0, sizeof(pad));
		memset(pad2, 0, sizeof(pad2));
		memset(pad3, 0, sizeof(pad3));
	}
};

fwEntityDef::~fwEntityDef()
{

}

int64_t fwEntityDef::GetTypeIdentifier()
{
	return 0;
}

static hook::cdecl_stub<void*(fwEntityDef*, int fileIdx, fwArchetype* archetype, uint64_t* archetypeUnk)> fwEntityDef__instantiate([] ()
{
	return hook::get_call(hook::pattern("4C 8D 4C 24 40 4D 8B C6 41 8B D7 48 8B CF").count(1).get(0).get<void>(14));
});

static hook::cdecl_stub<fwArchetype*(uint32_t nameHash, uint64_t* archetypeUnk)> getArchetype([] ()
{
	return hook::get_call(hook::pattern("89 44 24 40 8B 4F 08 80 E3 01 E8").count(1).get(0).get<void>(10));
});

static atArray<fwFactoryBase<fwArchetype>*>* g_archetypeFactories;

static void*(*dataFileMgr__getEntries)(void*, int);

static void* g_origVT[90];

template<int OrigIdx>
void* CustomVTWrapper(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9, void* a10, void* a11, void* a12)
{
	trace("called custom VT func %d\n", OrigIdx);

	auto origFunc = (void*(*)(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9, void* a10, void* a11, void* a12))g_origVT[OrigIdx];

	return origFunc(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

struct CMapDataContents
{
	void* vtable;
	void* sceneNodes;
	void** entities;
	uint32_t numEntities;
};

struct CMapData
{
	uint8_t pad[20];
	uint32_t unkBool;
	uint8_t pad2[40];
	float aabbMin[4];
	float aabbMax[4];
};

static hook::cdecl_stub<CMapDataContents*()> makeMapDataContents([] ()
{
	return hook::pattern("48 00 00 00 E8 ? ? ? ? 48 8B D8 48 85 C0 74 14").count(1).get(0).get<void>(-7);
});

static hook::cdecl_stub<void(CMapDataContents*, CMapData*, bool, bool)> addToScene([] ()
{
	return hook::pattern("48 83 EC 50 83 79 18 00 0F 29 70 C8 41 8A F1").count(1).get(0).get<void>(-0x18);
});

static uintptr_t sceneNodeThing;

static void* DoBeforeGetEntries(void* dataFileMgr, int type)
{
	uint32_t modelHash = HashString("lovely");
	//uint32_t modelHash = HashString("bh1_07_build2");

	fwArchetypeDef* archetypeDef = new fwArchetypeDef();
	archetypeDef->boundingBoxMin[0] = -60.0f;
	archetypeDef->boundingBoxMin[1] = -60.0f;
	archetypeDef->boundingBoxMin[2] = -60.0f;

	archetypeDef->boundingBoxMax[0] = 60.0f;
	archetypeDef->boundingBoxMax[1] = 60.0f;
	archetypeDef->boundingBoxMax[2] = 60.0f;

	archetypeDef->centroid[0] = 0;
	archetypeDef->centroid[1] = 0;
	archetypeDef->centroid[2] = 0;

	archetypeDef->radius = 300.0f;

	archetypeDef->nameHash = modelHash;
	archetypeDef->txdHash = modelHash;

	//void* lovelyR = (*g_archetypeFactories)[1]->GetOrCreate(modelHash, 1);

	trace("HEY YOU\n");

	fwArchetype* lovely = nullptr;//(*g_archetypeFactories)[1]->Get(modelHash);

	trace("initial lovely: %p\n", lovely);

	if (!lovely)
	{
		void* lovelyR = g_archetypeFactories->Get(1)->GetOrCreate(modelHash, 1);

		lovely = g_archetypeFactories->Get(1)->Get(modelHash);

		lovely->InitializeFromArchetypeDef(modelHash, archetypeDef, true);

		*(uint32_t*)((char*)lovely + 80) &= ~(1 << 31);
	}

	//lovely->InitializeFromArchetypeDef(modelHash, archetypeDef, true);

	// unset some 'is from maptype' thing
	//*(uint32_t*)((char*)lovely + 80) &= ~(1 << 31);

	uint64_t archetypeUnk = 0xFFFFFFF;
	fwArchetype* lovely2 = getArchetype(modelHash, &archetypeUnk);

	assert(lovely && lovely2);

	trace("lovely %p - lovely2 %p\n", lovely, lovely2);

	fwEntityDef* entityDef = new fwEntityDef();
	entityDef->archetypeNameHash = modelHash;
	entityDef->guidHash = 0xCA3ECA3E;

	entityDef->position[0] = -426.858f; entityDef->position[1] = -957.54f; entityDef->position[2] = 3.621f;
	entityDef->rotation[0] = 0;
	entityDef->rotation[1] = 0;
	entityDef->rotation[2] = 0;
	entityDef->rotation[3] = 1;

	void* entity = fwEntityDef__instantiate(entityDef, 0, lovely2, &archetypeUnk);

	void** vtableRef = (void**)(entity);
	static void* customVT[90];

	memcpy(customVT, *vtableRef, sizeof(customVT));
	*vtableRef = customVT;

	memcpy(g_origVT, customVT, sizeof(g_origVT));

#define SET_VT(z, i, d) customVT[i] = CustomVTWrapper<i>;

	BOOST_PP_REPEAT(90, SET_VT, nullptr);

	assert(entity);

	trace("entity: %p\n", entity);

	CMapDataContents* contents = makeMapDataContents();
	contents->entities = new void*[1];
	contents->entities[0] = entity;
	contents->numEntities = 1;

	CMapData mapData = { 0 };
	/*mapData.aabbMin[0] = -300.0f;
	mapData.aabbMin[1] = -1300.0f;
	mapData.aabbMin[2] = -100.0f;

	mapData.aabbMax[0] = 300.0f;
	mapData.aabbMax[1] = 1300.0f;
	mapData.aabbMax[2] = 100.0f;*/

	mapData.aabbMax[0] = FLT_MAX;
	mapData.aabbMax[1] = FLT_MAX;
	mapData.aabbMax[2] = FLT_MAX;
	mapData.aabbMax[3] = FLT_MAX;

	mapData.aabbMin[0] = 0.0f - FLT_MAX;
	mapData.aabbMin[1] = 0.0f - FLT_MAX;
	mapData.aabbMin[2] = 0.0f - FLT_MAX;
	mapData.aabbMin[3] = 0.0f - FLT_MAX;

	mapData.unkBool = 2;

	trace("scene node before: %p\n", contents->sceneNodes);

	//addToScene(contents, &mapData, false, true);
	addToScene(contents, &mapData, false, false);

	trace("scene node after: %p %p\n", contents->sceneNodes, *(uintptr_t*)((char*)contents->sceneNodes + 24));

	sceneNodeThing = *(uintptr_t*)((char*)contents->sceneNodes + 24);

	return dataFileMgr__getEntries(dataFileMgr, type);
}

static HookFunction hookFunction([] ()
{
	char* creator = hook::pattern("48 8B 0C C8 48 8B 01 FF 50 08 41 B1 01 4C").count(1).get(0).get<char>(-4);

	g_archetypeFactories = (decltype(g_archetypeFactories))(creator + *(int32_t*)creator + 4);

	creator = hook::pattern("8D 57 FA 8B 48 08 E8").count(1).get(0).get<char>(-4);

	g_baseArchetypeDefId = (int64_t*)(creator + *(int32_t*)creator + 4);

	void* getEntries = hook::pattern("BA 03 00 00 00 E8 ? ? ? ? 45 33 E4 B9 20").count(1).get(0).get<void>(5);
	hook::set_call(&dataFileMgr__getEntries, getEntries);
	hook::call(getEntries, DoBeforeGetEntries);

	// yolo
	//hook::nop(hook::pattern("0F 50 C0 83 E0 07 3C 07 0F 94 C1 85 D1 74 43").count(1).get(0).get<void>(13), 2);

	//__debugbreak();
});