#include "StdInc.h"

#include <atArray.h>
#include <Local.h>
#include <ScriptEngine.h>
#include <nutsnbolts.h>
#include <DirectXMath.h>
#include <atPool.h>
#include <GameInit.h>

#ifdef GTA_FIVE
#include <Hooking.h>
#include <CrossBuildRuntime.h>
#include <DirectXMath.h>
#elif IS_RDR3
#include <Pool.h>
#endif

#include "GameValueStub.h"

#ifdef GTA_FIVE
#define DECLARE_ACCESSOR(x) \
	decltype(impl.m2060.x)& x()        \
	{                       \
		return (xbr::IsGameBuildOrGreater<2060>() ? impl.m2060.x : impl.m1604.x);   \
	} \
	const decltype(impl.m2060.x)& x() const                         \
	{                                                    \
		return (xbr::IsGameBuildOrGreater<2060>() ? impl.m2060.x : impl.m1604.x);  \
	}
#elif IS_RDR3
#define DECLARE_ACCESSOR(x) \
	decltype(impl.x)& x()        \
	{                       \
		return impl.x;   \
	} \
	const decltype(impl.x)& x() const                         \
	{                                                    \
		return impl.x;  \
	}
#endif

#ifdef GTA_FIVE
#define GET_INTERIOR_PROXY(interiorId) \
	([](int handle) -> InteriorProxy* { \
		if (xbr::IsGameBuildOrGreater<2060>()) { \
			auto impl = GetInteriorProxyImpl<2060>(handle); \
			if (impl) { \
				static InteriorProxy proxy; \
				proxy.impl.m2060 = *impl; \
				return &proxy; \
			} \
		} else { \
			auto impl = GetInteriorProxyImpl<1604>(handle); \
			if (impl) { \
				static InteriorProxy proxy; \
				proxy.impl.m1604 = *impl; \
				return &proxy; \
			} \
		} \
		return nullptr; \
	})(interiorId)
#elif IS_RDR3
#define GET_INTERIOR_PROXY(interiorId) GetInteriorProxyImpl<1491>(interiorId)
#endif

using Matrix3x4 = DirectX::XMFLOAT3X4;

struct Vector
{
	float x; // +0
	float y; // +4
	float z; // +8
	float w; // +12
};

#ifdef IS_RDR3
struct CReflectionProbe
{
	char pad0[16]; // +0
	Vector minExtents; // +16
	Vector maxExtents; // +32
	Vector rotation; // +48
	Vector centerOffset; // +64
	Vector influenceExtents; // +80
	uint8_t probePriority; // +96
	char pad1[3]; // +97
	uint64_t guid; // +100
};
#endif

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
#ifdef IS_RDR3
	atArray<CReflectionProbe> reflectionProbes; // +106
#endif
};

struct CMloPortalDef
{
	void* vtable; // +0
	uint32_t roomFrom; // +8
	uint32_t roomTo; // +12
	uint32_t flags; // +16
	int32_t mirrorPriority; // +20
	float opacity;	// +24
#ifdef IS_RDR3
	float fyboamga_0x7c096c2e; // +28
	float iikmykba_0x91171666; // +32
	float rlibjjda_0x96816ddf; // +36
	float qdszgwfa_0x0854fdee; // +40
#endif
	int32_t audioOcclusion; // +28 (FiveM) / +44 (RDR3)
	atArray<Vector> corners; // +32 (FiveM) / +48 (RDR3)
	atArray<uint32_t> attachedObjects; // +48 (FiveM) / +64 (RDR3)
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

#ifdef GTA_FIVE
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
#elif IS_RDR3
	struct Impl1491
	{
		void* vtbl;
		uint32_t mapIndex; // +8
		char pad1[4]; // +12
		uint32_t occlusionIndex; // +16
		char pad2[36]; // +20
		CInteriorInst* instance; // +56
		char pad3[16]; // +64
		Vector rotation; // +80
		Vector position; // +96
		Vector center; // +112
		Vector entitiesExtentsMin; // +128
		Vector entitiesExtentsMax; // +144
		char pad4[80]; // +160
	};
#endif

#ifdef GTA_FIVE
	union
	{
		Impl1604 m1604;
		Impl2060 m2060;
	} impl;
#elif IS_RDR3
	Impl1491 impl;
#endif

public:
	DECLARE_ACCESSOR(instance);
	DECLARE_ACCESSOR(rotation);
	DECLARE_ACCESSOR(position);
	DECLARE_ACCESSOR(entitiesExtentsMin);
	DECLARE_ACCESSOR(entitiesExtentsMax);
};

#ifdef GTA_FIVE
template<int Build>
using CInteriorProxy = std::conditional_t<(Build >= 2060), InteriorProxy::Impl2060, InteriorProxy::Impl1604>;
template<int Build>
static atPool<CInteriorProxy<Build>>** g_interiorProxyPool;

template<int Build>
static CInteriorProxy<Build>* GetInteriorProxyImpl(int handle)
{
	return (*g_interiorProxyPool<Build>)->GetAtHandle<CInteriorProxy<Build>>(handle);
}
#elif IS_RDR3
template<int Build>
using CInteriorProxy = InteriorProxy;

template<int Build>
static CInteriorProxy<Build>* GetInteriorProxyImpl(int handle)
{
    static auto pool = rage::GetPool<CInteriorProxy<Build>>("InteriorProxy");
    if (pool)
    {
        return pool->GetAtHandle<CInteriorProxy<Build>>(handle);
    }
    return nullptr;
}
#endif

static CMloModelInfo* GetInteriorArchetype(int interiorId)
{
	CInteriorInst* instance = nullptr;

	auto proxy = GET_INTERIOR_PROXY(interiorId);
	if (proxy)
	{
		instance = proxy->instance();
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

#ifdef GTA_FIVE
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
#endif

static HookFunction initFunction([]()
{
#ifdef GTA_FIVE
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
#endif

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

		// Archetype Information
		trace("\n=== INTERIOR ARCHETYPE INFO ===\n");
		trace("Interior ID: %d\n", interiorId);
		trace("Archetype Address: %08x\n", (uint64_t)arch);
		trace("Room Count: %d\n", roomCount);
		trace("Portal Count: %d\n", portalCount);
		trace("Entity Set Count: %d\n", entitySetCount);
		trace("Timecycle Modifier Count: %d\n", timecycleModifierCount);
		
		// Get interior proxy for position/rotation info
		auto proxy = GET_INTERIOR_PROXY(interiorId);
		if (proxy)
		{
			auto position = proxy->position();
			auto rotation = proxy->rotation();
			auto entitiesExtentsMin = proxy->entitiesExtentsMin();
			auto entitiesExtentsMax = proxy->entitiesExtentsMax();
			trace("Interior Proxy Address: %08x\n", (uint64_t)proxy);
			trace("Interior Position: %f %f %f\n", position.x, position.y, position.z);
			trace("Interior Rotation: %f %f %f %f\n", rotation.x, rotation.y, rotation.z, rotation.w);
			trace("Entities Extents Min: %f %f %f\n", entitiesExtentsMin.x, entitiesExtentsMin.y, entitiesExtentsMin.z);
			trace("Entities Extents Max: %f %f %f\n", entitiesExtentsMax.x, entitiesExtentsMax.y, entitiesExtentsMax.z);
			
			// Raw memory dump of interior proxy (increased to 256 bytes to capture full struct)
			trace("\n--- Raw Interior Proxy Memory Dump ---\n");
			uint8_t* proxyBytes = (uint8_t*)proxy;
			for (int i = 0; i < 256; i += 16)
			{
				trace("%08x: ", (uint64_t)proxyBytes + i);
				for (int j = 0; j < 16; j++)
				{
					if (i + j < 256)
						trace("%02x ", proxyBytes[i + j]);
					else
						trace("   ");
				}
				trace(" |");
				for (int j = 0; j < 16 && i + j < 256; j++)
				{
					char c = proxyBytes[i + j];
					trace("%c", (c >= 32 && c <= 126) ? c : '.');
				}
				trace("|\n");
			}
			trace("------------------------------------\n");
			
		}
		
		// Raw memory dump of archetype (increased to 128 bytes)
		trace("\n--- Raw Archetype Memory Dump ---\n");
		uint8_t* archBytes = (uint8_t*)arch;
		for (int i = 0; i < 128; i += 16)
		{
			trace("%08x: ", (uint64_t)archBytes + i);
			for (int j = 0; j < 16; j++)
			{
				if (i + j < 128)
					trace("%02x ", archBytes[i + j]);
				else
					trace("   ");
			}
			trace(" |");
			for (int j = 0; j < 16 && i + j < 128; j++)
			{
				char c = archBytes[i + j];
				trace("%c", (c >= 32 && c <= 126) ? c : '.');
			}
			trace("|\n");
		}
		trace("--------------------------------\n");
		trace("===============================\n");

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
#ifdef IS_RDR3
			trace(" - ReflectionProbes: %d\n", room->reflectionProbes.GetCount());
			for (int probeId = 0; probeId < room->reflectionProbes.GetCount(); ++probeId)
			{
				auto probe = &room->reflectionProbes[probeId];
				trace("   - Probe %d: MinExtents(%f %f %f) MaxExtents(%f %f %f)\n", 
					probeId, probe->minExtents.x, probe->minExtents.y, probe->minExtents.z,
					probe->maxExtents.x, probe->maxExtents.y, probe->maxExtents.z);
				trace("     Rotation(%f %f %f %f) CenterOffset(%f %f %f)\n",
					probe->rotation.x, probe->rotation.y, probe->rotation.z, probe->rotation.w,
					probe->centerOffset.x, probe->centerOffset.y, probe->centerOffset.z);
				trace("     InfluenceExtents(%f %f %f) Priority(%d) GUID(0x%016llX)\n",
					probe->influenceExtents.x, probe->influenceExtents.y, probe->influenceExtents.z,
					probe->probePriority, probe->guid);
			}
#endif
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

#ifdef GTA_FIVE
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
#endif

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

// Native already exists in RDR3
#ifdef GTA_FIVE
	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_POSITION", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);

		auto proxy = GET_INTERIOR_PROXY(interiorId);
		if (proxy)
		{
			auto position = proxy->position();
			*context.GetArgument<float*>(1) = position.x;
			*context.GetArgument<float*>(2) = position.y;
			*context.GetArgument<float*>(3) = position.z;
		}
	});
#endif

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROTATION", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);

		auto proxy = GET_INTERIOR_PROXY(interiorId);
		if (proxy)
		{
			auto rotation = proxy->rotation();
			*context.GetArgument<float*>(1) = rotation.x;
			*context.GetArgument<float*>(2) = rotation.y;
			*context.GetArgument<float*>(3) = rotation.z;
			*context.GetArgument<float*>(4) = rotation.w;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ENTITIES_EXTENTS", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);

		auto proxy = GET_INTERIOR_PROXY(interiorId);
		if (proxy)
		{
			auto bbMin = proxy->entitiesExtentsMin();
			auto bbMax = proxy->entitiesExtentsMax();
			*context.GetArgument<float*>(1) = bbMin.x;
			*context.GetArgument<float*>(2) = bbMin.y;
			*context.GetArgument<float*>(3) = bbMin.z;
			*context.GetArgument<float*>(4) = bbMax.x;
			*context.GetArgument<float*>(5) = bbMax.y;
			*context.GetArgument<float*>(6) = bbMax.z;
		}
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

#ifdef IS_RDR3
	// Reflection Probe Natives
	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_REFLECTION_PROBE_COUNT", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto result = 0;

		auto room = GetInteriorRoomDef(interiorId, roomId);
		if (room)
		{
			result = room->reflectionProbes.GetCount();
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_REFLECTION_PROBE_EXTENTS", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto probeId = context.GetArgument<int>(2);

		auto room = GetInteriorRoomDef(interiorId, roomId);
		if (room && probeId >= 0 && probeId < room->reflectionProbes.GetCount())
		{
			auto probe = &room->reflectionProbes[probeId];
			*context.GetArgument<float*>(3) = probe->minExtents.x;
			*context.GetArgument<float*>(4) = probe->minExtents.y;
			*context.GetArgument<float*>(5) = probe->minExtents.z;
			*context.GetArgument<float*>(6) = probe->maxExtents.x;
			*context.GetArgument<float*>(7) = probe->maxExtents.y;
			*context.GetArgument<float*>(8) = probe->maxExtents.z;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_REFLECTION_PROBE_CENTER_OFFSET", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto probeId = context.GetArgument<int>(2);

		auto room = GetInteriorRoomDef(interiorId, roomId);
		if (room && probeId >= 0 && probeId < room->reflectionProbes.GetCount())
		{
			auto probe = &room->reflectionProbes[probeId];
			*context.GetArgument<float*>(3) = probe->centerOffset.x;
			*context.GetArgument<float*>(4) = probe->centerOffset.y;
			*context.GetArgument<float*>(5) = probe->centerOffset.z;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_REFLECTION_PROBE_INFLUENCE_EXTENTS", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto probeId = context.GetArgument<int>(2);

		auto room = GetInteriorRoomDef(interiorId, roomId);
		if (room && probeId >= 0 && probeId < room->reflectionProbes.GetCount())
		{
			auto probe = &room->reflectionProbes[probeId];
			*context.GetArgument<float*>(3) = probe->influenceExtents.x;
			*context.GetArgument<float*>(4) = probe->influenceExtents.y;
			*context.GetArgument<float*>(5) = probe->influenceExtents.z;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_REFLECTION_PROBE_ROTATION", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto probeId = context.GetArgument<int>(2);

		auto room = GetInteriorRoomDef(interiorId, roomId);
		if (room && probeId >= 0 && probeId < room->reflectionProbes.GetCount())
		{
			auto probe = &room->reflectionProbes[probeId];
			*context.GetArgument<float*>(3) = probe->rotation.x;
			*context.GetArgument<float*>(4) = probe->rotation.y;
			*context.GetArgument<float*>(5) = probe->rotation.z;
			*context.GetArgument<float*>(6) = probe->rotation.w;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_REFLECTION_PROBE_PRIORITY", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto probeId = context.GetArgument<int>(2);
		auto result = 0;

		auto room = GetInteriorRoomDef(interiorId, roomId);
		if (room && probeId >= 0 && probeId < room->reflectionProbes.GetCount())
		{
			auto probe = &room->reflectionProbes[probeId];
			result = probe->probePriority;
		}

		context.SetResult<int>(result);
	});

	// Reflection Probe Setters
	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_ROOM_REFLECTION_PROBE_EXTENTS", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto probeId = context.GetArgument<int>(2);
		auto minX = context.GetArgument<float>(3);
		auto minY = context.GetArgument<float>(4);
		auto minZ = context.GetArgument<float>(5);
		auto maxX = context.GetArgument<float>(6);
		auto maxY = context.GetArgument<float>(7);
		auto maxZ = context.GetArgument<float>(8);

		auto room = GetInteriorRoomDef(interiorId, roomId);
		if (room && probeId >= 0 && probeId < room->reflectionProbes.GetCount())
		{
			auto probe = &room->reflectionProbes[probeId];
			probe->minExtents.x = minX;
			probe->minExtents.y = minY;
			probe->minExtents.z = minZ;
			probe->maxExtents.x = maxX;
			probe->maxExtents.y = maxY;
			probe->maxExtents.z = maxZ;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_ROOM_REFLECTION_PROBE_CENTER_OFFSET", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto probeId = context.GetArgument<int>(2);
		auto x = context.GetArgument<float>(3);
		auto y = context.GetArgument<float>(4);
		auto z = context.GetArgument<float>(5);

		auto room = GetInteriorRoomDef(interiorId, roomId);
		if (room && probeId >= 0 && probeId < room->reflectionProbes.GetCount())
		{
			auto probe = &room->reflectionProbes[probeId];
			probe->centerOffset.x = x;
			probe->centerOffset.y = y;
			probe->centerOffset.z = z;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_ROOM_REFLECTION_PROBE_INFLUENCE_EXTENTS", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto probeId = context.GetArgument<int>(2);
		auto x = context.GetArgument<float>(3);
		auto y = context.GetArgument<float>(4);
		auto z = context.GetArgument<float>(5);

		auto room = GetInteriorRoomDef(interiorId, roomId);
		if (room && probeId >= 0 && probeId < room->reflectionProbes.GetCount())
		{
			auto probe = &room->reflectionProbes[probeId];
			probe->influenceExtents.x = x;
			probe->influenceExtents.y = y;
			probe->influenceExtents.z = z;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_ROOM_REFLECTION_PROBE_ROTATION", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto probeId = context.GetArgument<int>(2);
		auto x = context.GetArgument<float>(3);
		auto y = context.GetArgument<float>(4);
		auto z = context.GetArgument<float>(5);
		auto w = context.GetArgument<float>(6);

		auto room = GetInteriorRoomDef(interiorId, roomId);
		if (room && probeId >= 0 && probeId < room->reflectionProbes.GetCount())
		{
			auto probe = &room->reflectionProbes[probeId];
			probe->rotation.x = x;
			probe->rotation.y = y;
			probe->rotation.z = z;
			probe->rotation.w = w;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_INTERIOR_ROOM_REFLECTION_PROBE_PRIORITY", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto probeId = context.GetArgument<int>(2);
		auto priority = context.GetArgument<int>(3);

		auto room = GetInteriorRoomDef(interiorId, roomId);
		if (room && probeId >= 0 && probeId < room->reflectionProbes.GetCount())
		{
			auto probe = &room->reflectionProbes[probeId];
			probe->probePriority = (uint8_t)priority;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_REFLECTION_PROBE_GUID", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto roomId = context.GetArgument<int>(1);
		auto probeId = context.GetArgument<int>(2);

		auto room = GetInteriorRoomDef(interiorId, roomId);
		if (room && probeId >= 0 && probeId < room->reflectionProbes.GetCount())
		{
			context.SetResult<uint64_t>(room->reflectionProbes[probeId].guid);
		}
		else
		{
			context.SetResult<uint64_t>(0);
		}
	});
#endif

#ifdef GTA_FIVE
	// Sharing OnKillNetworkDone for probe lengths
	OnKillNetworkDone.Connect([]()
	{
		g_interiorProbeLengthOverride = 0.0f;
		g_emitterAudioEntityProbeLength.Reset();
	});
#endif
});
