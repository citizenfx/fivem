#include "StdInc.h"

#include <atArray.h>
#include <Local.h>
#include <Hooking.h>
#include <ScriptEngine.h>
#include <nutsnbolts.h>
#include <atPool.h>
#include <DirectXMath.h>
#include <CrossBuildRuntime.h>
#include <GameInit.h>

#include "GameValueStub.h"

#define DECLARE_ACCESSOR(x) \
	decltype(impl.m2060.x)& x()        \
	{                       \
		return (xbr::IsGameBuildOrGreater<2060>() ? impl.m2060.x : impl.m1604.x);   \
	} \
	const decltype(impl.m2060.x)& x() const                         \
	{                                                    \
		return (xbr::IsGameBuildOrGreater<2060>() ? impl.m2060.x : impl.m1604.x);  \
	}

using Matrix3x4 = DirectX::XMFLOAT3X4;

struct Vector
{
	float x; // +0
	float y; // +4
	float z; // +8
	float w; // +16
};

struct iCEntityDef
{
	void* vtable; // +0
	uint32_t archetypeName; // +8
	uint32_t flags; // +12
	char pad1[16]; // +16
	Vector position; // +32
	Vector rotation; // +48
	float scaleXY; // +64
	float scaleZ; // +68
	int16_t parentIndex; // +72
	uint16_t lodDist; // +74
	uint16_t childLodDist; // +74
	char pad2[36]; // +76 (todo: extensions)
	float ambientOcclusionMultiplier; // +112
	float artificialAmbientOcclusion; // +116
	int16_t tintValue; // +120
	char pad3[6]; // +122
};

struct CMloRoomDef
{
	void* vtable; // +0
	char* name; // +8
	int64_t unkFlag; // +16
	char pad1[8]; // +24
	Vector bbMin; // +32
	Vector bbMax; // +48
	float blend; // +64
	uint32_t timecycleName; // +68
	uint32_t secondaryTimecycleName;
	uint32_t flags; // +76
	uint32_t portalsCount; // +80
	int32_t floorId; // +84
	int16_t exteriorVisibilityDepth; // +88
	atArray<uint32_t> attachedObjects; // +90
};

struct CMloPortalDef
{
	void* vtable; // +0
	uint32_t roomFrom; // +8
	uint32_t roomTo; // +12
	uint32_t flags; // +16
	int32_t mirrorPriority; // +20
	float opacity;	// +24
	int32_t audioOcclusion; // +28
	atArray<Vector> corners; // +32
	atArray<uint32_t> attachedObjects; // +48
};

struct CMloEntitySet
{
	void* vtable; // +0
	uint64_t name; // +8
	atArray<int32_t> locations; // +12
	atArray<iCEntityDef> entities; // +28
};

struct CMloTimeCycleModifier
{
	void* vtable; // +0
	uint64_t name; // +8
	Vector sphere; // +16
	float percentage; // +32
	float range; // +36
	uint32_t startHour; // +40
	uint32_t endHour; // +44
};

struct CMloModelInfo
{
	char pad1[200]; // +0
	atArray<iCEntityDef*>* entities; // +200
	atArray<CMloRoomDef>* rooms; // +208
	atArray<CMloPortalDef>* portals; // +216
	atArray<CMloEntitySet>* entitySets; // +224
	atArray<CMloTimeCycleModifier>* timecycleModifiers; // +232
};

struct CInteriorInst
{
	char pad1[32]; // +0
	CMloModelInfo* mloModel; // +32
	char pad2[56]; // +40
	Matrix3x4 matrix; // +96
	Vector position; // +144
	char pad3[228]; // +164
	// CInteriorProxy* proxy; // +392
};

struct InteriorProxy
{
public:
	struct Impl1604
	{
		void* vtbl;
		uint32_t mapIndex; // +8
		char pad1[4]; // +12
		uint32_t occlusionIndex; // +16
		char pad2[44]; // +20
		CInteriorInst* instance; // +64
		char pad3[24]; // +72
		Vector rotation; // +96
		Vector position; // +112
		Vector entitiesExtentsMin; // +128
		Vector entitiesExtentsMax; // +144
		char pad4[68]; // +160
		uint32_t archetypeHash; // +228
		char pad5[8]; // +232
	};

	struct Impl2060
	{
		void* vtbl;
		uint32_t mapIndex; // +8
		char pad1[4]; // +12
		uint32_t occlusionIndex; // +16
		char pad2[44]; // +20
		uint32_t unkFlag; // +64, some flag added in 1868, hardcoded for ch_dlc_arcade
		char pad3[4]; // +68
		CInteriorInst* instance; // +72
		char pad4[16]; // +80
		Vector rotation; // +96
		Vector position; // +112
		Vector entitiesExtentsMin; // +128
		Vector entitiesExtentsMax; // +144
		char pad5[68]; // +160
		uint32_t archetypeHash; // +228
		char pad6[8]; // +232
	};

	union
	{
		Impl1604 m1604;
		Impl2060 m2060;
	} impl;

public:
	DECLARE_ACCESSOR(instance);
	DECLARE_ACCESSOR(rotation);
	DECLARE_ACCESSOR(position);
	DECLARE_ACCESSOR(entitiesExtentsMin);
	DECLARE_ACCESSOR(entitiesExtentsMax);
};

template<int Build>
using CInteriorProxy = std::conditional_t<(Build >= 2060), InteriorProxy::Impl2060, InteriorProxy::Impl1604>;

template<int Build>
static atPool<CInteriorProxy<Build>>** g_interiorProxyPool;

template<int Build>
static CInteriorProxy<Build>* GetInteriorProxy(int handle)
{
	return (*g_interiorProxyPool<Build>)->GetAtHandle<CInteriorProxy<Build>>(handle);
}

static CMloModelInfo* GetInteriorArchetype(int interiorId)
{
	CInteriorInst* instance = nullptr;

	if (xbr::IsGameBuildOrGreater<2060>())
	{
		CInteriorProxy<2060>* proxy = GetInteriorProxy<2060>(interiorId);
		if (proxy)
		{
			instance = proxy->instance;
		}
	}
	else
	{
		CInteriorProxy<1604>* proxy = GetInteriorProxy<1604>(interiorId);
		if (proxy)
		{
			instance = proxy->instance;
		}
	}

	if (instance && instance->mloModel)
	{
		return instance->mloModel;
	}

	return nullptr;
}

static CMloRoomDef* GetInteriorRoomDef(int interiorId, int roomId)
{
	CMloModelInfo* arch = GetInteriorArchetype(interiorId);

	if (arch == nullptr || roomId < 0 || roomId >= arch->rooms->GetCount())
	{
		return nullptr;
	}

	return &(arch->rooms->Get(roomId));
}

static CMloPortalDef* GetInteriorPortalDef(int interiorId, int portalId)
{
	CMloModelInfo* arch = GetInteriorArchetype(interiorId);

	if (arch == nullptr || portalId < 0 || portalId >= arch->portals->GetCount())
	{
		return nullptr;
	}

	return &(arch->portals->Get(portalId));
}

static CMloEntitySet* GetInteriorEntitySet(int interiorId, int setId)
{
	CMloModelInfo* arch = GetInteriorArchetype(interiorId);

	if (arch == nullptr || setId < 0 || setId >= arch->entitySets->GetCount())
	{
		return nullptr;
	}

	return &(arch->entitySets->Get(setId));
}

static iCEntityDef* GetInteriorPortalEntityDef(int interiorId, int portalId, int entityId)
{
	CMloModelInfo* arch = GetInteriorArchetype(interiorId);
	if (arch == nullptr)
	{
		return nullptr;
	}

	CMloPortalDef* portalDef = GetInteriorPortalDef(interiorId, portalId);
	if (portalDef == nullptr)
	{
		return nullptr;
	}

	if (entityId < 0 || entityId > (portalDef->attachedObjects.GetCount()) - 1)
	{
		return nullptr;
	}

	return arch->entities->Get(portalDef->attachedObjects[entityId]);
}

static CMloTimeCycleModifier* GetInteriorTimecycleModifier(int interiorId, int modId)
{
	CMloModelInfo* arch = GetInteriorArchetype(interiorId);

	if (arch == nullptr || modId < 0 || modId >= arch->timecycleModifiers->GetCount())
	{
		return nullptr;
	}

	return &(arch->timecycleModifiers->Get(modId));
}

static int GetInteriorRoomIdByHash(CMloModelInfo* arch, int searchHash)
{
	auto count = arch->rooms->GetCount();

	for (int i = 0; i < count; ++i)
	{
		CMloRoomDef* room = &(arch->rooms->Get(i));

		if (HashString(room->name) == searchHash)
		{
			return i;
		}
	}

	return -1;
}

static GameValueStub<float> g_emitterAudioEntityProbeLength;
static float g_interiorProbeLengthOverride = 0.0;

static bool (*g_CPortalTracker__Probe)(Vector3* pos, CInteriorInst** ppInteriorInstance, int* roomId, Vector3* traceImpactPoint, float traceLength);
static bool CPortalTracker__Probe(Vector3* pos, CInteriorInst** ppInteriorInstance, int* roomId, Vector3* traceImpactPoint, float traceLength)
{
	// game code has a lot of different special case handling in CPortalTracker::vft0x8
	// joaat('xs_arena_interior') seems to be the case with the longest traceLength override (150.f)
	//
	if (g_interiorProbeLengthOverride > 0.0f && traceLength < g_interiorProbeLengthOverride)
	{
		traceLength = g_interiorProbeLengthOverride;
	}

	return g_CPortalTracker__Probe(pos, ppInteriorInstance, roomId, traceImpactPoint, traceLength);
}

static HookFunction initFunction([]()
{
	{
		auto location = hook::get_pattern<void>("E8 ? ? ? ? 40 8A F8 49 ? ? E8");
		hook::set_call(&g_CPortalTracker__Probe, location);
		hook::call(location, CPortalTracker__Probe);
	}

	{
		auto location = hook::get_pattern<uint32_t>("33 ED 39 A9 ? ? ? ? 0F 86 ? ? ? ? F3", 18);
		g_emitterAudioEntityProbeLength.Init(*hook::get_address<float*>(location));
		g_emitterAudioEntityProbeLength.SetLocation(location);
	}

	{
		auto location = hook::get_pattern<char>("BA A1 85 94 52 41 B8 01", 0x34);

		if (xbr::IsGameBuildOrGreater<2060>())
		{
			g_interiorProxyPool<2060> = hook::get_address<decltype(g_interiorProxyPool<2060>)>(location);
		}
		else
		{
			g_interiorProxyPool<1604> = hook::get_address<decltype(g_interiorProxyPool<1604>)>(location);
		}
	}

#ifdef _DEBUG
	fx::ScriptEngine::RegisterNativeHandler("INTERIOR_DEBUG", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);

		CMloModelInfo* arch = GetInteriorArchetype(interiorId);
		if (arch == nullptr)
		{
			return;
		}

		auto roomCount = arch->rooms->GetCount();
		auto portalCount = arch->portals->GetCount();
		auto entitySetCount = arch->entitySets->GetCount();
		auto timecycleModifierCount = arch->timecycleModifiers->GetCount();

		for (int roomId = 0; roomId < roomCount; ++roomId)
		{
			auto room = GetInteriorRoomDef(interiorId, roomId);
			trace("\nSearching for CMloRoomDef %d/%d at %08x\n", roomId + 1, roomCount, (uint64_t)room);

			trace(" - Found room %s with index %d\n", std::string(room->name), roomId);
			trace(" - BbMin: %f %f %f\n", room->bbMin.x, room->bbMin.y, room->bbMin.z);
			trace(" - BbMax: %f %f %f\n", room->bbMax.x, room->bbMax.y, room->bbMax.z);
			trace(" - Blend: %f | Flags: %d | Portals: %d\n", room->blend, room->flags, room->portalsCount);
			trace(" - TimecycleName Hash: %d / %d\n", room->timecycleName, room->secondaryTimecycleName);
			trace(" - FloorId: %d | ExteriorVisibilityDepth: %d\n", room->floorId, room->exteriorVisibilityDepth);
			trace(" - AttachedObjects: %d\n", room->attachedObjects.GetCount());
		}

		for (int portalId = 0; portalId < portalCount; ++portalId)
		{
			auto portal = GetInteriorPortalDef(interiorId, portalId);
			trace("\nSearching for CMloPortalDef %d/%d at %08x\n", portalId + 1, portalCount, (uint64_t)portal);


			trace(" - Found portal with index %d\n", portalId);
			trace(" - RoomFrom: %d | RoomTo: %d\n", portal->roomFrom, portal->roomTo);
			trace(" - Flags: %d | Opacity: %f\n", portal->flags, portal->opacity);
			trace(" - MirrorPrior: %d | AudioOcclusion: %d\n", portal->mirrorPriority, portal->audioOcclusion);

			for (int c = 0; c < portal->corners.GetCount(); ++c)
			{
				trace(" - Corner%d: %f %f %f\n", c, portal->corners[c].x, portal->corners[c].y, portal->corners[c].z);
			}

			trace(" - AttachedObjects: %d\n", portal->attachedObjects.GetCount());
		}

		for (int setId = 0; setId < entitySetCount; ++setId)
		{
			auto entitySet = GetInteriorEntitySet(interiorId, setId);
			trace("\nSearching for CMloEntitySet %d/%d at %08x\n", setId + 1, entitySetCount, (uint64_t)entitySet);

			trace(" - Found entitySet with hash %08x\n", entitySet->name);
			trace(" - Locations: %d | Entities: %d\n", entitySet->locations.GetCount(), entitySet->entities.GetCount());
		}

		for (int modId = 0; modId < timecycleModifierCount; ++modId)
		{
			auto modifier = GetInteriorTimecycleModifier(interiorId, modId);
			trace("\nSearching for CMloTimeCycleModifier %d/%d at %08x\n", modId + 1, timecycleModifierCount, (uint64_t)modifier);

			trace(" - Found timecycleModifier with hash %08x\n", modifier->name);
			trace(" - Sphere: %f %f %f %f\n", modifier->sphere.x, modifier->sphere.y, modifier->sphere.z, modifier->sphere.w);
			trace(" - Percentage: %f | Range: %f\n", modifier->percentage, modifier->range);
			trace(" - Hours: %d - %d\n", modifier->startHour, modifier->endHour);
		}
	});
#endif

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_PROBE_LENGTH", [=](fx::ScriptContext& context)
	{
		auto length = context.GetArgument<float>(0);
		if (!std::isfinite(length))
		{
			return false;
		}

		g_interiorProbeLengthOverride = std::clamp(length, 0.0f, 150.0f);
		return true;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_EMITTER_PROBE_LENGTH", [=](fx::ScriptContext& context)
	{
		auto length = context.GetArgument<float>(0);
		if (!std::isfinite(length))
		{
			return false;
		}

		g_emitterAudioEntityProbeLength.Set(std::clamp(length, 20.0f, 150.0f));
		return true;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_INDEX_BY_HASH", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto searchHash = context.GetArgument<int>(1);
		auto result = -1;

		CMloModelInfo* arch = GetInteriorArchetype(interiorId);
		if (arch != nullptr)
		{
			result = GetInteriorRoomIdByHash(arch, searchHash);
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_NAME", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		char* result = "";

		CMloRoomDef* roomDef = GetInteriorRoomDef(interiorId, roomId);
		if (roomDef != nullptr)
		{
			result = roomDef->name;
		}

		context.SetResult<char*>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_FLAG", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto result = -1;

		CMloRoomDef* roomDef = GetInteriorRoomDef(interiorId, roomId);
		if (roomDef != nullptr)
		{
			result = roomDef->flags;
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_ROOM_FLAG", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto newFlag = context.GetArgument<int>(2);

		CMloRoomDef* roomDef = GetInteriorRoomDef(interiorId, roomId);
		if (roomDef == nullptr)
		{
			return false;
		}

		roomDef->flags = newFlag;
		return true;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_EXTENTS", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);

		CMloRoomDef* roomDef = GetInteriorRoomDef(interiorId, roomId);
		if (roomDef == nullptr)
		{
			return false;
		}

		*context.GetArgument<float*>(2) = roomDef->bbMin.x;
		*context.GetArgument<float*>(3) = roomDef->bbMin.y;
		*context.GetArgument<float*>(4) = roomDef->bbMin.z;

		*context.GetArgument<float*>(5) = roomDef->bbMax.x;
		*context.GetArgument<float*>(6) = roomDef->bbMax.y;
		*context.GetArgument<float*>(7) = roomDef->bbMax.z;

		return true;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_ROOM_EXTENTS", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);

		CMloRoomDef* roomDef = GetInteriorRoomDef(interiorId, roomId);
		if (roomDef == nullptr)
		{
			return false;
		}

		roomDef->bbMin.x = context.GetArgument<float>(2);
		roomDef->bbMin.y = context.GetArgument<float>(3);
		roomDef->bbMin.z = context.GetArgument<float>(4);

		roomDef->bbMax.x = context.GetArgument<float>(5);
		roomDef->bbMax.y = context.GetArgument<float>(6);
		roomDef->bbMax.z = context.GetArgument<float>(7);

		return true;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_TIMECYCLE", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		int result = 0;

		CMloRoomDef* roomDef = GetInteriorRoomDef(interiorId, roomId);
		if (roomDef != nullptr)
		{
			result = roomDef->timecycleName;
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_ROOM_TIMECYCLE", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto timecycleHash = context.GetArgument<int>(2);

		CMloRoomDef* roomDef = GetInteriorRoomDef(interiorId, roomId);
		if (roomDef == nullptr)
		{
			return false;
		}

		roomDef->timecycleName = timecycleHash;
		return true;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_PORTAL_CORNER_POSITION", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto cornerIndex = context.GetArgument<int>(2);

		CMloPortalDef* portalDef = GetInteriorPortalDef(interiorId, portalId);

		if (cornerIndex < 0 || cornerIndex >= portalDef->corners.GetCount())
		{
			return false;
		}

		*context.GetArgument<float*>(3) = portalDef->corners[cornerIndex].x;
		*context.GetArgument<float*>(4) = portalDef->corners[cornerIndex].y;
		*context.GetArgument<float*>(5) = portalDef->corners[cornerIndex].z;

		return true;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_PORTAL_CORNER_POSITION", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto cornerIndex = context.GetArgument<int>(2);

		auto posX = context.GetArgument<float>(3);
		auto posY = context.GetArgument<float>(4);
		auto posZ = context.GetArgument<float>(5);

		CMloPortalDef* portalDef = GetInteriorPortalDef(interiorId, portalId);
		if (portalDef == nullptr)
		{
			return false;
		}

		portalDef->corners[cornerIndex].x = posX;
		portalDef->corners[cornerIndex].y = posY;
		portalDef->corners[cornerIndex].z = posZ;

		return true;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_PORTAL_ROOM_FROM", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto result = -1;

		CMloPortalDef* portalDef = GetInteriorPortalDef(interiorId, portalId);
		if (portalDef != nullptr)
		{
			result = portalDef->roomFrom;
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_PORTAL_ROOM_FROM", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto newValue = context.GetArgument<int>(2);

		CMloPortalDef* portalDef = GetInteriorPortalDef(interiorId, portalId);
		if (portalDef != nullptr)
		{
			portalDef->roomFrom = newValue;
			return true;
		}

		return false;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_PORTAL_ROOM_TO", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto result = -1;

		CMloPortalDef* portalDef = GetInteriorPortalDef(interiorId, portalId);
		if (portalDef != nullptr)
		{
			result = portalDef->roomTo;
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_PORTAL_ROOM_TO", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto newValue = context.GetArgument<int>(2);

		CMloPortalDef* portalDef = GetInteriorPortalDef(interiorId, portalId);
		if (portalDef != nullptr)
		{
			portalDef->roomTo = newValue;
			return true;
		}

		return false;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_PORTAL_FLAG", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto result = -1;

		CMloPortalDef* portalDef = GetInteriorPortalDef(interiorId, portalId);
		if (portalDef != nullptr)
		{
			result = portalDef->flags;
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_PORTAL_FLAG", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto newValue = context.GetArgument<int>(2);

		CMloPortalDef* portalDef = GetInteriorPortalDef(interiorId, portalId);
		if (portalDef != nullptr)
		{
			portalDef->flags = newValue;
			return true;
		}

		return false;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_POSITION", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);

		Vector* position;

		if (xbr::IsGameBuildOrGreater<2060>())
		{
			CInteriorProxy<2060>* proxy = GetInteriorProxy<2060>(interiorId);
			position = &proxy->position;
		}
		else
		{
			CInteriorProxy<1604>* proxy = GetInteriorProxy<1604>(interiorId);
			position = &proxy->position;
		}

		*context.GetArgument<float*>(1) = position->x;
		*context.GetArgument<float*>(2) = position->y;
		*context.GetArgument<float*>(3) = position->z;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROTATION", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);

		Vector* rotation;

		if (xbr::IsGameBuildOrGreater<2060>())
		{
			CInteriorProxy<2060>* proxy = GetInteriorProxy<2060>(interiorId);
			rotation = &proxy->rotation;
		}
		else
		{
			CInteriorProxy<1604>* proxy = GetInteriorProxy<1604>(interiorId);
			rotation = &proxy->rotation;
		}

		*context.GetArgument<float*>(1) = rotation->x;
		*context.GetArgument<float*>(2) = rotation->y;
		*context.GetArgument<float*>(3) = rotation->z;
		*context.GetArgument<float*>(4) = rotation->w;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ENTITIES_EXTENTS", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);

		Vector* bbMin;
		Vector* bbMax;

		if (xbr::IsGameBuildOrGreater<2060>())
		{
			CInteriorProxy<2060>* proxy = GetInteriorProxy<2060>(interiorId);
			bbMin = &proxy->entitiesExtentsMin;
			bbMax = &proxy->entitiesExtentsMax;
		}
		else
		{
			CInteriorProxy<1604>* proxy = GetInteriorProxy<1604>(interiorId);
			bbMin = &proxy->entitiesExtentsMin;
			bbMax = &proxy->entitiesExtentsMax;
		}

		*context.GetArgument<float*>(1) = bbMin->x;
		*context.GetArgument<float*>(2) = bbMin->y;
		*context.GetArgument<float*>(3) = bbMin->z;

		*context.GetArgument<float*>(4) = bbMax->x;
		*context.GetArgument<float*>(5) = bbMax->y;
		*context.GetArgument<float*>(6) = bbMax->z;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_PORTAL_COUNT", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto result = 0;

		auto arch = GetInteriorArchetype(interiorId);
		if (arch != nullptr)
		{
			result = arch->portals->GetCount();
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_COUNT", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto result = 0;

		auto arch = GetInteriorArchetype(interiorId);
		if (arch != nullptr)
		{
			result = arch->rooms->GetCount();
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_PORTAL_ENTITY_COUNT", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto result = 0;

		CMloPortalDef* portalDef = GetInteriorPortalDef(interiorId, portalId);
		if (portalDef != nullptr)
		{
			result = portalDef->attachedObjects.GetCount();
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_PORTAL_ENTITY_ARCHETYPE", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto entityId = context.GetArgument<int>(2);
		auto result = 0;

		iCEntityDef* entityDef = GetInteriorPortalEntityDef(interiorId, portalId, entityId);
		if (entityDef != nullptr)
		{
			result = entityDef->archetypeName;
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_PORTAL_ENTITY_FLAG", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto entityId = context.GetArgument<int>(2);
		auto result = -1;

		iCEntityDef* entityDef = GetInteriorPortalEntityDef(interiorId, portalId, entityId);
		if (entityDef != nullptr)
		{
			result = entityDef->flags;
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_PORTAL_ENTITY_FLAG", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto entityId = context.GetArgument<int>(2);
		auto newValue = context.GetArgument<int>(3);

		iCEntityDef* entityDef = GetInteriorPortalEntityDef(interiorId, portalId, entityId);
		if (entityDef != nullptr)
		{
			entityDef->flags = newValue;
			return true;
		}

		return false;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_PORTAL_ENTITY_POSITION", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto entityId = context.GetArgument<int>(2);

		iCEntityDef* entityDef = GetInteriorPortalEntityDef(interiorId, portalId, entityId);
		if (entityDef == nullptr)
		{
			return false;
		}

		*context.GetArgument<float*>(3) = entityDef->position.x;
		*context.GetArgument<float*>(4) = entityDef->position.y;
		*context.GetArgument<float*>(5) = entityDef->position.z;

		return true;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_PORTAL_ENTITY_ROTATION", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto portalId = context.GetArgument<int>(1);
		auto entityId = context.GetArgument<int>(2);

		iCEntityDef* entityDef = GetInteriorPortalEntityDef(interiorId, portalId, entityId);
		if (entityDef == nullptr)
		{
			return false;
		}

		*context.GetArgument<float*>(3) = entityDef->rotation.x;
		*context.GetArgument<float*>(4) = entityDef->rotation.y;
		*context.GetArgument<float*>(5) = entityDef->rotation.z;
		*context.GetArgument<float*>(6) = entityDef->rotation.w;

		return true;
	});

	// Sharing OnKillNetworkDone for probe lengths
	OnKillNetworkDone.Connect([]()
	{
		g_interiorProbeLengthOverride = 0.0f;
		g_emitterAudioEntityProbeLength.Reset();
	});
});
