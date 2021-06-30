#pragma once

#include "StdInc.h"

#include <rageVectors.h>

namespace rage
{
// ----------- rage::ropeData -----------

class grcTexture;

class ropeData
{
public:
	virtual ~ropeData() = 0;

	virtual const char* GetName() = 0;

	virtual parStructure* parser_GetStructure() = 0;

	uint32_t numSections; // 0x8
	float radius; // 0xC
	uint32_t diffuseTextureNameHash; // 0x10
	uint32_t normalMapNameHash; // 0x14
	grcTexture* diffuseTexture; // 0x18
	grcTexture* normalMap; // 0x20
	float distanceMappingScale; // 0x28
	float UVScaleX; // 0x2C
	float UVScaleY; // 0x30
	float specularFresnel; // 0x34
	float specularFalloff; // 0x38
	float specularIntensity; // 0x3C
	float bumpiness; // 0x40
	uint32_t color; // 0x44
};
static_assert(sizeof(ropeData) == 0x48);

// ----------- rage::ropeDataManager -----------

class ropeDataManager
{
public:
	virtual ~ropeDataManager() = 0;

	atArray<ropeData*> typeData; // 0x8

	static ropeDataManager* GetInstance();
};
static_assert(sizeof(ropeDataManager) == 0x18);

// ----------- rage::ropeInstance -----------

class phVerletCloth;
class rmcRopeDrawable;
class phInst;
class environmentCloth;
class scriptHandler;

enum eRopeFlags : uint8_t
{
	DrawShadowEnabled = 2,
	Breakable = 4,
	RopeUnwindingFront = 8,
	RopeWinding = 32
};

class ropeInstance
{
public:
	ropeInstance* currentRope; // 0x0
	ropeInstance* nextRope; // 0x8
	ropeInstance* previousRope; // 0x10
	phVerletCloth* verletCloth; // 0x18
	char pad_20[0x38]; // 0x20
	rmcRopeDrawable* drawable; // 0x58
	phInst* phInst; // 0x60
	environmentCloth* environmentCloth; // 0x68
	char pad_70[0x8]; // 0x70
	uint32_t attachLength; // 0x78
	char pad_8C[0x24]; // 0x70
	char* attachBoneOne; // 0xA0
	char* attachBoneTwo; // 0xA8
	Vector3 entityOneOffset; // 0xB0
	Vector3 entityTwoOffset; // 0xC0
	char pad_D0[0x4]; // 0xD0
	Vector3 attachPosition; // 0xD4
	char pad_E4[0xB0]; // 0xE4
	scriptHandler* scriptHandler; // 0x198
	uint32_t updateOrder; // 0x1A0
	int handle; // 0x1A4
	char pad_1A8[0x4]; // 0x1A8
	uint32_t ropeTypeIndex; // 0x1AC
	char pad_1B0[0x8]; // 0x1B0
	float field_1B4; // 0x1B8
	uint32_t field_1BC; // 0x1BC
	float forcedLength; // 0x1C0
	float initLength; // 0x1C4
	float minLength; // 0x1C8
	float lengthChangeRate; // 0x1CC
	float sectionLength; // 0x1D0
	float timeMultiplier; // 0x1D4
	char pad_1D8[0x6]; // 0x1D8
	eRopeFlags flags; // 0x1DE
	char pad_1DF[0x1]; // 0x1DF
};
static_assert(sizeof(ropeInstance) == 0x1E0);

// ----------- rage::ropeManager -----------

class verletTaskManager;

template<typename T>
class LinkedList
{
public:
	T head;
	T tail;
};

class ropeManager
{
public:
	virtual ~ropeManager() = 0;

	char pad_8[0x8]; // 0x8
	verletTaskManager* verletTaskManager; // 0x10
	uint32_t numAllocated; // 0x18
	uint32_t numAvailable; // 0x1C
	char pad_20[0x8]; // 0x20
	LinkedList<ropeInstance*> allocated; // 0x28
	LinkedList<ropeInstance*> available; // 0x38
	char pad_48[0x10]; // 0x48
	LinkedList<phVerletCloth*> cloths; // 0x58

	ropeInstance* FindRope(int handle);

	static ropeManager* GetInstance();
};
static_assert(sizeof(ropeManager) == 0x68);
}
