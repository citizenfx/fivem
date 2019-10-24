#include "StdInc.h"

#include <atArray.h>
#include <Local.h>
#include <Hooking.h>
#include <ScriptEngine.h>
#include <nutsnbolts.h>
#include <atPool.h>
#include <DirectXMath.h>

using Matrix3x4 = DirectX::XMFLOAT3X4;

struct Vector
{
	float x; // +0
	float y; // +4
	float z; // +8
	float w; // +16
};

struct CEntityDef
{
	void* vtable; // +0
	int32_t archetypeName; // +8
	int32_t flags; // +12
	char pad1[16]; // +16
	Vector position; // +32
	Vector rotation; // +48
	float scaleXY; // +64
	float scaleZ; // +68
	int16_t parentIndex; // +72
	int16_t lodDist; // +74
	int16_t childLodDist; // +74
	char pad2[36]; // +76 (todo: extensions)
	float ambientOcclusionMultiplier; // +112
	float artificialAmbientOcclusion; // +116
	int16_t tintValue; // +120
	char pad3[5]; // +122
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
	int32_t timecycleName; // +68
	int32_t secondaryTimecycleName;
	uint32_t flags; // +76
	uint32_t portalsCount; // +80
	int32_t floorId; // +84
	int16_t exteriorVisibiltyDepth; // +88
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
	int64_t name; // +8
	atArray<int32_t> locations; // +12
	atArray<CEntityDef> entities; // +28
};

struct CMloTimeCycleModifier
{
	void* vtable; // +0
	int64_t name; // +8
	Vector sphere; // +16
	float percentage; // +32
	float range; // +36
	uint32_t startHour; // +40
	uint32_t endHour; // +44
};

// not real CMloArchetypeDef, but +168 from it
struct CMloArchetypeDef
{
	atArray<CMloRoomDef> rooms; // +0
	atArray<CMloPortalDef> portals; // +16
	atArray<CMloEntitySet> entitySets; // +32
	atArray<CMloTimeCycleModifier> timecycleModifiers; // +48
};

struct CMloModel
{
	char pad1[208]; // +0
	CMloArchetypeDef* mloArchetypeDef; // +208
};

struct CInteriorInst
{
	char pad1[32]; // +0
	CMloModel* mloModel; // +32
	char pad2[56]; // +40
	Matrix3x4 matrix; // +96
	Vector position; // +144
	char pad3[228]; // +164
	// CInteriorProxy* proxy; // +392
};

struct CInteriorProxy
{
	char pad1[64]; // +0
	CInteriorInst* instance; // +64
	char pad2[24]; // +72
	Vector rotation; // +96
	Vector position; // +112
	Vector entitiesExtentsMin; // +128
	Vector entitiesExtentsMax; // +144
	char pad3[68]; // +160
	int32_t minimapHash; // +228
	char pad4[8]; // +232
};

static atPool<CInteriorProxy>** g_interiorProxyPool;

static CInteriorProxy* GetInteriorProxy(int handle)
{
	return (*g_interiorProxyPool)->GetAtHandle<CInteriorProxy>(handle);
}

static CMloArchetypeDef* GetInteriorArchetype(int interiorId)
{
	CInteriorProxy* interiorProxy = GetInteriorProxy(interiorId);
	return interiorProxy->instance->mloModel->mloArchetypeDef;
}

static CMloRoomDef* GetInteriorRoomDef(int interiorId, int roomId)
{
	CMloArchetypeDef* arch = GetInteriorArchetype(interiorId);

	if (arch == nullptr || roomId < 0 || roomId > arch->rooms.GetCount())
	{
		return nullptr;
	}

	return &(arch->rooms[roomId]);
}

static CMloPortalDef* GetInteriorPortalDef(int interiorId, int portalId)
{
	CMloArchetypeDef* arch = GetInteriorArchetype(interiorId);

	if (arch == nullptr || portalId < 0 || portalId > arch->portals.GetCount())
	{
		return nullptr;
	}

	return &(arch->portals[portalId]);
}

static CMloEntitySet* GetInteriorEntitySet(int interiorId, int setId)
{
	CMloArchetypeDef* arch = GetInteriorArchetype(interiorId);

	if (arch == nullptr || setId < 0 || setId > arch->entitySets.GetCount())
	{
		return nullptr;
	}

	return &(arch->entitySets[setId]);
}

static CMloTimeCycleModifier* GetInteriorTimecycleModifier(int interiorId, int modId)
{
	CMloArchetypeDef* arch = GetInteriorArchetype(interiorId);

	if (arch == nullptr || modId < 0 || modId > arch->timecycleModifiers.GetCount())
	{
		return nullptr;
	}

	return &(arch->timecycleModifiers[modId]);
}

static int GetInteriorRoomIdByHash(CMloArchetypeDef* arch, int searchHash)
{
	auto count = arch->rooms.GetCount();

	for (int i = 0; i < count; ++i)
	{
		CMloRoomDef* room = &(arch->rooms[i]);

		if (HashString(room->name) == searchHash)
		{
			return i;
		}
	}

	return -1;
}

static HookFunction initFunction([]()
{
	{
		auto location = hook::pattern("BA A1 85 94 52 41 B8 01").count(1).get(0).get<char>(0x34);
		g_interiorProxyPool = (decltype(g_interiorProxyPool))(location + *(int32_t*)location + 4);
	}

#ifdef _DEBUG
	fx::ScriptEngine::RegisterNativeHandler("INTERIOR_DEBUG", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);

		CMloArchetypeDef* arch = GetInteriorArchetype(interiorId);
		if (arch == nullptr)
		{
			return;
		}

		auto roomCount = arch->rooms.GetCount();
		auto portalCount = arch->portals.GetCount();
		auto entitySetCount = arch->entitySets.GetCount();
		auto timecycleModifierCount = arch->timecycleModifiers.GetCount();

		for (int roomId = 0; roomId < roomCount; ++roomId)
		{
			auto room = GetInteriorRoomDef(interiorId, roomId);
			trace("\nSearching for CMloRoomDef %d/%d at %08x\n", roomId + 1, roomCount, (int64_t)room);

			trace(" - Found room %s with index %d\n", std::string(room->name), roomId);
			trace(" - BbMin: %f %f %f\n", room->bbMin.x, room->bbMin.y, room->bbMin.z);
			trace(" - BbMax: %f %f %f\n", room->bbMax.x, room->bbMax.y, room->bbMax.z);
			trace(" - Blend: %f | Flags: %d | Portals: %d\n", room->blend, room->flags, room->portalsCount);
			trace(" - TimecycleName Hash: %d / %d\n", room->timecycleName, room->secondaryTimecycleName);
			trace(" - FloorId: %d | ExteriorVisibiltyDepth: %d\n", room->floorId, room->exteriorVisibiltyDepth);
			trace(" - AttachedObjects: %d\n", room->attachedObjects.GetCount());
		}

		for (int portalId = 0; portalId < portalCount; ++portalId)
		{
			auto portal = GetInteriorPortalDef(interiorId, portalId);
			trace("\nSearching for CMloPortalDef %d/%d at %08x\n", portalId + 1, portalCount, (int64_t)portal);


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
			trace("\nSearching for CMloEntitySet %d/%d at %08x\n", setId + 1, entitySetCount, (int64_t)entitySet);

			trace(" - Found entitySet with hash %08x\n", entitySet->name);
			trace(" - Locations: %d | Entities: %d\n", entitySet->locations.GetCount(), entitySet->entities.GetCount());
		}

		for (int modId = 0; modId < timecycleModifierCount; ++modId)
		{
			auto modifier = GetInteriorTimecycleModifier(interiorId, modId);
			trace("\nSearching for CMloTimeCycleModifier %d/%d at %08x\n", modId + 1, timecycleModifierCount, (int64_t)modifier);

			trace(" - Found timecycleModifier with hash %08x\n", modifier->name);
			trace(" - Sphere: %f %f %f %f\n", modifier->sphere.x, modifier->sphere.y, modifier->sphere.z, modifier->sphere.w);
			trace(" - Percentage: %f | Range: %f\n", modifier->percentage, modifier->range);
			trace(" - Hours: %d - %d\n", modifier->startHour, modifier->endHour);
		}
	});
#endif

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROOM_INDEX_BY_HASH", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto searchHash = context.GetArgument<int>(1);
		auto result = -1;

		CMloArchetypeDef* arch = GetInteriorArchetype(interiorId);
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

		*context.GetArgument<float*>(1) = roomDef->bbMin.x;
		*context.GetArgument<float*>(2) = roomDef->bbMin.x;
		*context.GetArgument<float*>(3) = roomDef->bbMin.x;

		*context.GetArgument<float*>(4) = roomDef->bbMax.x;
		*context.GetArgument<float*>(5) = roomDef->bbMax.y;
		*context.GetArgument<float*>(6) = roomDef->bbMax.z;

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
		roomDef->bbMin.x = context.GetArgument<float>(3);
		roomDef->bbMin.x = context.GetArgument<float>(4);

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

		if (cornerIndex < 0 || cornerIndex > portalDef->corners.GetCount())
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
		auto intProxy = GetInteriorProxy(interiorId);
		Vector* position = &intProxy->position;

		*context.GetArgument<float*>(1) = position->x;
		*context.GetArgument<float*>(2) = position->y;
		*context.GetArgument<float*>(3) = position->z;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ROTATION", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto intProxy = GetInteriorProxy(interiorId);
		Vector* rotation = &intProxy->rotation;

		*context.GetArgument<float*>(1) = rotation->x;
		*context.GetArgument<float*>(2) = rotation->y;
		*context.GetArgument<float*>(3) = rotation->z;
		*context.GetArgument<float*>(4) = rotation->w;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INTERIOR_ENTITIES_EXTENTS", [=](fx::ScriptContext& context)
	{
		auto interiorId = context.GetArgument<int>(0);
		auto proxy = GetInteriorProxy(interiorId);
		Vector* bbMin = &proxy->entitiesExtentsMin;
		Vector* bbMax = &proxy->entitiesExtentsMax;

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
			result = arch->portals.GetCount();
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
			result = arch->rooms.GetCount();
		}

		context.SetResult<int>(result);
	});
});
