/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include "NativeWrappers.h"
#include <ScriptEngine.h>
#include <atArray.h>

#include <jitasm.h>
#include <Hooking.h>
#include <GameInit.h>
#include <nutsnbolts.h>
#include <gameSkeleton.h>

#include <limits>
#include <bitset>
#include <unordered_set>

#include <CoreConsole.h>
#include <Resource.h>

#include <fxScripting.h>

#include <MinHook.h>

#include <xinput.h>

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Gaming.Input.h>

#include "DeferredInitializer.h"

using namespace winrt::Windows::Gaming::Input;

struct PatternPair
{
	std::string_view pattern;
	int offset;
};

struct FlyThroughWindscreenParam
{
	float* currentPtr;
	float* defaultPtr;
};

struct TrainDoor
{
	char pad_0[0x2C];
	float ratio;
	char pad_30[0x40];
};
static_assert(sizeof(TrainDoor) == 0x70);

struct VehicleXenonLightsColor
{
	float colorR;
	float colorG;
	float colorB;
	uint32_t colorARGB;

	VehicleXenonLightsColor(uint8_t red, uint8_t green, uint8_t blue)
	{
		Update(red, green, blue);
	}

	void Update(uint8_t red, uint8_t green, uint8_t blue)
	{
		colorR = red / 255.0;
		colorG = green / 255.0;
		colorB = blue / 255.0;
		colorARGB = (0xFF << 24) | (red << 16) | (green << 8) | blue;
	}
};

struct VehicleDashboardData
{
	float RPM;
	float speed;
	float fuel;
	float temp;
	float vacuum;
	float boost;
	float waterTemp;
	float oilTemp;
	float oilPressure;
	char _pad[0x3F]; // aircraft data
	bool indicator_left;
	bool indicator_right;
	bool handbrakeLight;
	bool engineLight;
	bool ABSLight;
	bool gasLight;
	bool oilLight;
	bool headlights;
	bool highBeam;
	bool batteryLight;
};

static std::unordered_set<fwEntity*> g_skipRepairVehicles{};

static std::vector<FlyThroughWindscreenParam> g_flyThroughWindscreenParams{};

static std::map<fwEntity*, VehicleXenonLightsColor> g_vehicleXenonLightsColors{};

static bool* g_flyThroughWindscreenDisabled;
static bool isFlyThroughWindscreenEnabledConVar = false;

template<typename T>
inline static T readValue(fwEntity* ptr, int offset)
{
	return (T)*(T*)((char*)ptr + offset);
}

template<typename T>
inline static void writeValue(fwEntity* ptr, int offset, T value)
{
	*(T*)((char*)ptr + offset) = value;
}

static fwEntity* getAndCheckVehicle(fx::ScriptContext& context, std::string_view name)
{
	auto traceFn = [name](std::string_view msg)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			auto resource = (fx::Resource*)runtime->GetParentObject();

			if (resource)
			{
				console::Printf(fmt::sprintf("script:%s", resource->GetName()), "%s: %s\n", name, msg);
				return;
			}
		}

		trace("%s: %s\n", name, msg);
	};

	if (context.GetArgumentCount() < 1)
	{
		traceFn("At least one argument must be passed");
		return nullptr;
	}

	fwEntity* vehicle = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

	if (!vehicle)
	{
		traceFn("No such entity\n");
		return nullptr;
	}

	if (!vehicle->IsOfType<CVehicle>())
	{
		traceFn("Can not read from an entity that is not a vehicle\n");
		return nullptr;
	}

	return vehicle;
}

template<typename T, int* offset, typename U = T>
static void readVehicleMemory(fx::ScriptContext& context, std::string_view nn)
{
	if (fwEntity* vehicle = getAndCheckVehicle(context, nn))
	{
		context.SetResult<U>((U)readValue<T>(vehicle, *offset));
	}
}

template<int* offset, int bit>
static void readVehicleMemoryBit(fx::ScriptContext& context, std::string_view nn)
{
	if (fwEntity* vehicle = getAndCheckVehicle(context, nn))
	{
		std::bitset<sizeof(int) * 8> value(readValue<int>(vehicle, *offset));

		context.SetResult<bool>(value[bit]);
	}
}

template<int* offset, unsigned char bit>
static void writeVehicleMemoryBit(fx::ScriptContext& context, std::string_view nn)
{
	if (context.GetArgumentCount() < 2)
	{
		trace("Insufficient arguments count, 2 expected");
		return;
	}

	if (fwEntity* vehicle = getAndCheckVehicle(context, nn))
	{
		std::bitset<sizeof(int) * 8> value(readValue<int>(vehicle, *offset));
		value[bit] = context.GetArgument<bool>(1);
		writeValue<unsigned char>(vehicle, *offset, static_cast<unsigned char>(value.to_ulong()));
	}
}

template<typename T, int* offset>
static void writeVehicleMemory(fx::ScriptContext& context, std::string_view nn)
{
	if (context.GetArgumentCount() < 2)
	{
		trace("Insufficient arguments count, 2 expected");
		return;
	}

	if (fwEntity* vehicle = getAndCheckVehicle(context, nn))
	{
		writeValue<T>(vehicle, *offset, context.GetArgument<T>(1));
	}
}

static int StreamRenderGfxPtrOffset;
static int HandlingDataPtrOffset;
static int DrawHandlerPtrOffset;
static int ModelInfoPtrOffset;
static int WheelsPtrOffset;

static int FuelLevelOffset;
static int OilLevelOffset;
static int GravityOffset;
static int IsEngineStartingOffset;
static int IsWantedOffset;
static int DashSpeedOffset;
static int VehicleTypeOffset;
static int HighGearOffset;
static int CurrentGearOffset;
static int NextGearOffset;
static int HandbrakeOffset;
static int EngineTempOffset;
static int NumWheelsOffset;
static int SteeringAngleOffset;
static int SteeringScaleOffset;
static int IsAlarmSetOffset;
static int AlarmTimeLeftOffset;
static int WheelieStateOffset;
static int TrainTrackNodeIndexOffset;
static int PreviouslyOwnedByPlayerOffset;
static int NeedsToBeHotwiredOffset;
static int IsInteriorLightOnOffset;
static int BlinkerStateOffset;
static int ThrottleOffsetOffset;
static int CurrentRPMOffset;
static int StreamRenderWheelWidthOffset;
static int StreamRenderWheelSizeOffset;
static int DrawnWheelAngleMultOffset;
static int TurboBoostOffset; // = 0x8D8;
static int ClutchOffset; // = 0x8C0;
//static int VisualHeightGetOffset = 0x080; // There is a vanilla native for this.
static int VisualHeightSetOffset = 0x07C;
static int LightMultiplierGetOffset;

// TODO: Wheel class.
static int WheelYRotOffset = 0x008;
static int WheelInvYRotOffset = 0x010;
static int WheelXOffsetOffset = 0x030;
static int WheelTyreRadiusOffset = 0x110;
static int WheelRimRadiusOffset = 0x114;
static int WheelTyreWidthOffset = 0x118;
static int WheelSuspensionCompressionOffset;
static int WheelRotationSpeedOffset; // = 0x170;
static int WheelTractionVectorLengthOffset; // = 0x1B8;
static int WheelSteeringAngleOffset; // = 0x1CC;
static int WheelBrakePressureOffset; // = 0x1D0;
static int WheelPowerOffset;
static int WheelHealthOffset; // = 0x1E8; // 75 24 F3 0F 10 81 ? ? ? ? F3 0F
static int WheelSurfaceMaterialOffset;
static int WheelFlagsOffset;

static char* VehicleTopSpeedModifierPtr;
static int VehicleCheatPowerIncreaseOffset;

static bool* g_trainsForceDoorsOpen;
static int TrainDoorCountOffset;
static int TrainDoorArrayPointerOffset;

static int VehicleRepairMethodVtableOffset;

static std::unordered_set<fwEntity*> g_deletionTraces;
static std::unordered_set<void*> g_deletionTraces2;

static void(*g_origDeleteVehicle)(void* vehicle);

static void SetCanPedStandOnVehicle(fwEntity* vehicle, int flag);

static void DeleteVehicleWrap(fwEntity* vehicle)
{
	if (g_deletionTraces.find(vehicle) != g_deletionTraces.end())
	{
		uintptr_t* traceStart = (uintptr_t*)_AddressOfReturnAddress();

		trace("Processed vehicle deletion for 0x%016llx - stack trace:", (uintptr_t)vehicle);

		for (int i = 0; i < 96; i++)
		{
			if ((i % 6) == 0)
			{
				trace("\n");
			}

			trace("%016llx ", traceStart[i]);
		}

		trace("\n");

		g_deletionTraces.erase(vehicle);
		g_deletionTraces2.erase(vehicle);
	}

	// save handling data pointer
	CHandlingData* handling = readValue<CHandlingData*>(vehicle, HandlingDataPtrOffset);

	// call original destructor
	g_origDeleteVehicle(vehicle);

	// run cleanup after destructor
	g_skipRepairVehicles.erase(vehicle);
	g_vehicleXenonLightsColors.erase(vehicle);

	// remove flag
	SetCanPedStandOnVehicle(vehicle, 0);

	// Delete the handling if it has been set to hooked.
	if (*((char*)handling + 28) == 1)
	{
		delete handling;
	}
}

static void(*g_origDeleteNetworkClone)(void* objectMgr, void* netObject, int reason, bool forceRemote1, bool forceRemote2);

static void DeleteNetworkCloneWrap(void* objectMgr, void* netObject, int reason, bool forceRemote1, bool forceRemote2)
{
	void* entity = *(void**)((char*)netObject + 80);

	if (g_deletionTraces2.find(entity) != g_deletionTraces2.end())
	{
		uintptr_t* traceStart = (uintptr_t*)_AddressOfReturnAddress();

		trace("Processed clone deletion for 0x%016llx (object 0x%016llx - reason %d) - stack trace:", (uintptr_t)entity, (uintptr_t)netObject, reason);

		for (int i = 0; i < 96; i++)
		{
			if ((i % 6) == 0)
			{
				trace("\n");
			}

			trace("%016llx ", traceStart[i]);
		}

		trace("\n");

		g_deletionTraces2.erase(entity);
	}

	return g_origDeleteNetworkClone(objectMgr, netObject, reason, forceRemote1, forceRemote2);
}

static void ResetFlyThroughWindscreenParams()
{
	for (auto& entry : g_flyThroughWindscreenParams)
	{
		*entry.currentPtr = *entry.defaultPtr;
	}
}

static bool (*g_origCanPedStandOnVehicle)(CVehicle*);
static bool g_overrideCanPedStandOnVehicle;
static std::unordered_map<fwEntity*, int> g_canPedStandOnVehicles;

static void SetCanPedStandOnVehicle(fwEntity* vehicle, int flag)
{
	if (flag == 0)
	{
		g_canPedStandOnVehicles.erase(vehicle);
		return;
	}

	g_canPedStandOnVehicles[vehicle] = flag;
}

static bool CanPedStandOnVehicleWrap(CVehicle* vehicle)
{
	if (g_overrideCanPedStandOnVehicle)
	{
		return true;
	}

	if (auto it = g_canPedStandOnVehicles.find(vehicle); it != g_canPedStandOnVehicles.end())
	{
		auto can = it->second;

		if (can == -1)
		{
			return false;
		}
		else if (can == 1)
		{
			return true;
		}
	}

	return g_origCanPedStandOnVehicle(vehicle);
}

static void OverrideVehicleXenonColor(CVehicle* vehicle, float* color, uint32_t* colorARGB)
{
	if (auto it = g_vehicleXenonLightsColors.find(vehicle); it != g_vehicleXenonLightsColors.end())
	{
		color[0] = it->second.colorR;
		color[1] = it->second.colorG;
		color[2] = it->second.colorB;
		*colorARGB = it->second.colorARGB;
	}
}

static VehicleDashboardData g_DashboardData{};
void (*g_origDashboardHandler)(void* modelInfo, VehicleDashboardData* data);

static void DashboardHandler(void* modelInfo, VehicleDashboardData* data)
{
	if (data)
	{
		g_DashboardData = *data;

		g_origDashboardHandler(modelInfo, data);
	}
}

TrainDoor* GetTrainDoor(fwEntity* train, uint32_t index)
{
	return &(*((TrainDoor**)(((char*)train) + TrainDoorArrayPointerOffset)))[index];
}

static bool* isNetworkGame;

static HookFunction initFunction([]()
{
	isNetworkGame = hook::get_address<bool*>(hook::get_pattern("24 07 3C 03 75 12 40 38 35 ? ? ? ? 75 09 83", 9));

	{
		ModelInfoPtrOffset = *hook::get_pattern<uint8_t>("48 8B 40 ? 0F B6 80 ? ? ? ? 83 E0 1F", 3);
		GravityOffset = *hook::get_pattern<uint32_t>("0F C6 F6 00 F3 0F 59 05", -4);
		DashSpeedOffset = *hook::get_pattern<uint32_t>("0F 84 ? ? ? ? 44 89 AE ? ? ? ? 44 84 F3", 9);
		VehicleTypeOffset = *hook::get_pattern<uint32_t>("41 83 BF ? ? ? ? 0B 74", 3);
		HighGearOffset = *hook::get_pattern<uint32_t>("88 44 24 20 45 0F 28 D0", -4);
		HandbrakeOffset = *hook::get_pattern<uint32_t>("8A C2 24 01 C0 E0 04 08 81", 19);
		EngineTempOffset = *hook::get_pattern<uint32_t>("48 8D 8F ? ? ? ? 45 32 FF", -4);
		SteeringScaleOffset = *hook::get_pattern<uint32_t>("41 8B 80 ? ? ? ? C1 E8 ? 40 84 C5 74", 18);
		WheelieStateOffset = *hook::get_pattern<uint32_t>("8B 87 ? ? ? ? 48 8B 57 ? 89 87", 39);
		TrainTrackNodeIndexOffset = *hook::get_pattern<uint32_t>("E8 ? ? ? ? 40 8A F8 84 C0 75 ? 48 8B CB E8", -4);
		StreamRenderGfxPtrOffset = *hook::get_pattern<uint32_t>("4C 8D 48 ? 80 E1 01", -4);
		DrawnWheelAngleMultOffset = *hook::get_pattern<uint32_t>("48 8B C8 E8 ? ? ? ? 84 C0 74 ? F3 0F 10 83", 33);
		VehicleTopSpeedModifierPtr = hook::get_pattern<char>("48 8B D9 48 81 C1 ? ? ? ? 48 89 5C 24 28 44 0F 29 40 C8");
		VehicleCheatPowerIncreaseOffset = *hook::get_pattern<uint32_t>("E8 ? ? ? ? 8B 83 ? ? ? ? C7 83", 23);
		WheelSurfaceMaterialOffset = *hook::get_pattern<uint32_t>("48 8B 4A 10 0F 28 CF F3 0F 59 05", -4);
		WheelHealthOffset = *hook::get_pattern<uint32_t>("75 24 F3 0F 10 ? ? ? 00 00 F3 0F", 6);
		LightMultiplierGetOffset = *hook::get_pattern<uint32_t>("00 00 48 8B CE F3 0F 59 ? ? ? 00 00 F3 41", 9);
		VehicleRepairMethodVtableOffset = *hook::get_pattern<uint32_t>("C1 E8 19 A8 01 74 ? 48 8B 81", -14);
	}

	if (xbr::IsGameBuildOrGreater<2372>())
	{
		auto location = hook::get_pattern<char>("89 87 ? ? ? ? 48 3B F5 74 2C 48 8B 6D 00 48 8B 0E 48");

		FuelLevelOffset = *(uint32_t*)(location - 16);
		OilLevelOffset = *(uint32_t*)(location - 4);
	}
	else
	{
		auto location = hook::get_pattern<char>("48 3B CA 0F 84 ? ? ? ? 8B 81");

		FuelLevelOffset = *(uint32_t*)(location + 49);
		OilLevelOffset = *(uint32_t*)(location + 61);
	}

	{
		auto location = hook::get_pattern<char>("F3 0F 10 9F ? ? ? ? 0F 2F DF 73 0A");

		TurboBoostOffset = *(uint32_t*)(location + 4);
	}

	{
		auto location = hook::get_pattern<char>("8A 96 ? ? ? ? 0F B6 C8 84 D2 41");

		IsEngineStartingOffset = *(uint32_t*)(location + 31);
		IsWantedOffset = *(uint32_t*)(location + 40);
	}

	{
		auto location = hook::get_pattern<char>("A8 02 0F 84 ? ? ? ? 0F B7 86");

		CurrentGearOffset = *(uint32_t*)(location + 11);
		NextGearOffset = *(uint32_t*)(location + 18);
	}

	{
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 48 63 87 ? ? ? ? 48 8B 8F");

		NumWheelsOffset = *(uint32_t*)(location + 8);
		WheelsPtrOffset = *(uint32_t*)(location + 15);
		SteeringAngleOffset = *(uint32_t*)(location + 23);
	}

	{
		auto location = hook::get_pattern<char>("24 07 3C 03 74 ? E8");

		IsAlarmSetOffset = *(uint32_t*)(location + 52);
		AlarmTimeLeftOffset = IsAlarmSetOffset;
	}

	{
		auto location = hook::get_pattern<char>("45 33 C9 41 B0 01 40 8A D7");

		PreviouslyOwnedByPlayerOffset = *(uint32_t*)(location - 5);
		NeedsToBeHotwiredOffset = PreviouslyOwnedByPlayerOffset;
	}

	{
		auto location = hook::get_pattern<char>("FD 02 DB 08 98 ? ? ? ? 48 8B 5C 24 30");

		IsInteriorLightOnOffset = *(uint32_t*)(location - 4);
		BlinkerStateOffset = IsInteriorLightOnOffset;
	}

	{
		auto location = hook::get_pattern<char>("F6 83 ? ? ? ? 07 75 ? 44 0F");

		CurrentRPMOffset = *(uint32_t*)(location - 42);
		ClutchOffset = CurrentRPMOffset + 12;
		ThrottleOffsetOffset = CurrentRPMOffset + 16;
	}

	{
		auto location = hook::get_pattern<char>("45 0F 57 ? F3 0F 11 ? ? ? 00 00 F3 0F 5C");

		WheelSuspensionCompressionOffset = *(uint32_t*)(location + 8);
		WheelRotationSpeedOffset = *(uint32_t*)(location + 8) + 0xC;
	}

	{
		char* location;
		if (xbr::IsGameBuildOrGreater<2060>())
		{
			location = hook::get_pattern<char>("0F 2F ? ? ? 00 00 0F 97 C0 EB ? D1");
		}
		else
		{
			location = hook::get_pattern<char>("0F 2F ? ? ? 00 00 0F 97 C0 EB DA");
		}
		WheelSteeringAngleOffset = (*(uint32_t*)(location + 3));
		WheelBrakePressureOffset = (*(uint32_t*)(location + 3)) + 0x4;
		WheelPowerOffset = (*(uint32_t*)(location + 3)) + 0x8;
		WheelTractionVectorLengthOffset = (*(uint32_t*)(location + 3)) - 0x14;
	}

	{
		auto location = hook::get_pattern<char>("75 11 48 8B 01 8B 88");

		WheelFlagsOffset = *(uint32_t*)(location + 7);
	}

	{
		auto location = hook::get_pattern<char>("48 89 01 B8 00 00 80 3F 66 44 89 51");

		StreamRenderWheelWidthOffset = *(uint32_t*)(location + 23);
		StreamRenderWheelSizeOffset = *(uint8_t*)(location + 20);
	}

	{
		auto location = hook::get_pattern<char>("44 0F 2F 43 48 45 8D");

		DrawHandlerPtrOffset = *(uint8_t*)(location + 4);
		HandlingDataPtrOffset = *(uint32_t*)(location - 35);
	}

	{
		// replace netgame check for train doors with our own
		g_trainsForceDoorsOpen = (bool*)hook::AllocateStubMemory(1);
		*g_trainsForceDoorsOpen = true;

		auto location = hook::get_pattern<uint32_t>("74 56 40 38 BB ? ? ? ? 74 49 48 8B CB E8", -8);
		hook::put<int32_t>(location, (intptr_t)g_trainsForceDoorsOpen - (intptr_t)location - 4);
	}

	{
		auto location = hook::get_pattern<char>("44 8B 91 ? ? ? ? 45 33 C0 45 8B C8 45 85 D2");

		TrainDoorCountOffset = *(uint32_t*)(location + 3);
		TrainDoorArrayPointerOffset = *(uint32_t*)(location + 33);
	}

	{
		// allow door collisions to be changed at any train stage, not just at the station so remove check
		// mov     eax, [rdi+1548h]       <-- train stage
		// sub     eax, 2
		// cmp     eax, 2
		// ja      short loc_140F634B9
		auto location = hook::get_pattern<char>("8B 87 ? ? ? ? 83 E8 02 83 F8 02 77 04");
		hook::nop(location, 14);
	}

	{
		// replace netgame check for fly through windscreen with our variable
		g_flyThroughWindscreenDisabled = (bool*)hook::AllocateStubMemory(1);
		static ConVar<bool> enableFlyThroughWindscreen("game_enableFlyThroughWindscreen", ConVar_Replicated, false, &isFlyThroughWindscreenEnabledConVar);

		auto location = hook::get_pattern<uint32_t>("45 33 ED 44 38 2D ? ? ? ? 4D", 6);
		hook::put<int32_t>(location, (intptr_t)g_flyThroughWindscreenDisabled - (intptr_t)location - 4);
	}

	{
		std::initializer_list<PatternPair> list = {
			{ "44 38 ? ? ? ? 02 74 ? F3 0F 10 1D", 13 },
			{ "44 38 ? ? ? ? 02 74 ? F3 0F 10 3D", 13 },
			{ "48 8B 10 FF 52 ? 0F 28 D8 F3 0F 59", -23 },
			{ "F3 0F 10 0D ? ? ? ? 0F 2F 8B ? ? ? ? 0F", 4 }
		};

		auto index = 0;
		auto stub = (uint64_t)hook::AllocateStubMemory(sizeof(float) * list.size());

		for (auto& entry : list)
		{
			auto location = hook::pattern(entry.pattern).count(1).get(0).get<int32_t>(entry.offset);

			auto defaultAddr = hook::get_address<float*>(location);
			auto currentAddr = (float*)(stub + (sizeof(float) * index));

			FlyThroughWindscreenParam data;
			data.defaultPtr = defaultAddr;
			data.currentPtr = currentAddr;

			hook::put<int32_t>(location, (intptr_t)currentAddr - (intptr_t)location - 4);
			++index;

			g_flyThroughWindscreenParams.push_back(data);
		}
	}

	// not a vehicle native
	static uint32_t setAngVelocityOffset = *hook::get_pattern<uint32_t>("75 11 48 8B 06 48 8D 54 24 20 48 8B CE FF 90", 15) / 8;

	fx::ScriptEngine::RegisterNativeHandler("SET_ENTITY_ROTATION_VELOCITY", [](fx::ScriptContext& context)
	{
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("Invalid entity ID passed to SET_ENTITY_ROTATION_VELOCITY.\n");
			return;
		}

		auto vtbl = *(uintptr_t**)entity;
		auto setAngularVelocity = (void(*)(fwEntity*, float*))vtbl[setAngVelocityOffset];

		alignas(16) float newVelocity[4];
		newVelocity[0] = context.GetArgument<float>(1);
		newVelocity[1] = context.GetArgument<float>(2);
		newVelocity[2] = context.GetArgument<float>(3);
		newVelocity[3] = 0.0f;

		setAngularVelocity(entity, newVelocity);
	});

	using namespace std::placeholders;

	// vehicle natives
	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_FUEL_LEVEL", std::bind(readVehicleMemory<float, &FuelLevelOffset>, _1, "GET_VEHICLE_FUEL_LEVEL"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_FUEL_LEVEL", std::bind(writeVehicleMemory<float, &FuelLevelOffset>, _1, "SET_VEHICLE_FUEL_LEVEL"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_OIL_LEVEL", std::bind(readVehicleMemory<float, &OilLevelOffset>, _1, "GET_VEHICLE_OIL_LEVEL"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_OIL_LEVEL", std::bind(writeVehicleMemory<float, &OilLevelOffset>, _1, "SET_VEHICLE_OIL_LEVEL"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_GRAVITY_AMOUNT", std::bind(readVehicleMemory<float, &GravityOffset>, _1, "GET_VEHICLE_GRAVITY_AMOUNT"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_GRAVITY_AMOUNT", std::bind(writeVehicleMemory<float, &GravityOffset>, _1, "SET_VEHICLE_GRAVITY_AMOUNT"));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_ENGINE_STARTING", std::bind(readVehicleMemoryBit<&IsEngineStartingOffset, 5>, _1, "IS_VEHICLE_ENGINE_STARTING"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_SPEED", std::bind(readVehicleMemory<float, &DashSpeedOffset>, _1, "GET_VEHICLE_DASHBOARD_SPEED"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_THROTTLE_OFFSET", std::bind(readVehicleMemory<float, &ThrottleOffsetOffset>, _1, "GET_VEHICLE_THROTTLE_OFFSET"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CURRENT_RPM", std::bind(readVehicleMemory<float, &CurrentRPMOffset>, _1, "GET_VEHICLE_CURRENT_RPM"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_CURRENT_RPM", std::bind(writeVehicleMemory<float, &CurrentRPMOffset>, _1, "SET_VEHICLE_CURRENT_RPM"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HIGH_GEAR", std::bind(readVehicleMemory<unsigned char, &HighGearOffset>, _1, "GET_VEHICLE_HIGH_GEAR"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_HIGH_GEAR", std::bind(writeVehicleMemory<unsigned char, &HighGearOffset>, _1, "SET_VEHICLE_HIGH_GEAR"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CURRENT_GEAR", std::bind(readVehicleMemory<unsigned char, &CurrentGearOffset, int>, _1, "GET_VEHICLE_CURRENT_GEAR"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_CURRENT_GEAR", std::bind(writeVehicleMemory<unsigned char, &CurrentGearOffset>, _1, "SET_VEHICLE_CURRENT_GEAR"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NEXT_GEAR", std::bind(readVehicleMemory<unsigned char, &NextGearOffset, int>, _1, "GET_VEHICLE_NEXT_GEAR"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_NEXT_GEAR", std::bind(writeVehicleMemory<unsigned char, &NextGearOffset>, _1, "SET_VEHICLE_NEXT_GEAR"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CLUTCH", std::bind(readVehicleMemory<float, &ClutchOffset>, _1, "GET_VEHICLE_CLUTCH"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_CLUTCH", std::bind(writeVehicleMemory<float, &ClutchOffset>, _1, "SET_VEHICLE_CLUTCH"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_TURBO_PRESSURE", std::bind(readVehicleMemory<float, &TurboBoostOffset>, _1, "GET_VEHICLE_TURBO_PRESSURE"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_TURBO_PRESSURE", std::bind(writeVehicleMemory<float, &TurboBoostOffset>, _1, "SET_VEHICLE_TURBO_PRESSURE"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HANDBRAKE", std::bind(readVehicleMemory<bool, &HandbrakeOffset>, _1, "GET_VEHICLE_HANDBRAKE")); // just a getter

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ENGINE_TEMPERATURE", std::bind(readVehicleMemory<float, &EngineTempOffset>, _1, "GET_VEHICLE_ENGINE_TEMPERATURE"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_ENGINE_TEMPERATURE", std::bind(writeVehicleMemory<float, &EngineTempOffset>, _1, "SET_VEHICLE_ENGINE_TEMPERATURE"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_RPM", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_DashboardData.RPM);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_FUEL", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_DashboardData.fuel);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_TEMP", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_DashboardData.temp);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_VACUUM", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_DashboardData.vacuum);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_BOOST", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_DashboardData.boost);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_WATER_TEMP", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_DashboardData.waterTemp);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_OIL_PRESSURE", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_DashboardData.oilPressure);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_OIL_TEMP", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_DashboardData.oilTemp);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_LIGHTS", [](fx::ScriptContext& context)
	{
		int lightState = (g_DashboardData.indicator_left << 0)
						 | (g_DashboardData.indicator_right << 1)
						 | (g_DashboardData.handbrakeLight << 2)
						 | (g_DashboardData.engineLight << 3)
						 | (g_DashboardData.ABSLight << 4)
						 | (g_DashboardData.gasLight << 5)
						 | (g_DashboardData.oilLight << 6)
						 | (g_DashboardData.headlights << 7)
						 | (g_DashboardData.highBeam << 8)
						 | (g_DashboardData.batteryLight << 9);
		context.SetResult<int>(lightState);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_SUSPENSION_HEIGHT", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 2)
		{
			return;
		}

		if (fwEntity* vehicle = getAndCheckVehicle(context, "SET_VEHICLE_SUSPENSION_HEIGHT"))
		{
			auto wheelsAddress = readValue<uint64_t>(vehicle, WheelsPtrOffset);
			auto addr = *reinterpret_cast<float*>(wheelsAddress + VisualHeightSetOffset) = context.GetArgument<float>(1);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NUMBER_OF_WHEELS", std::bind(readVehicleMemory<unsigned char, &NumWheelsOffset>, _1, "GET_VEHICLE_NUMBER_OF_WHEELS"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_SPEED", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 2)
		{
			context.SetResult<float>(0.0f);
			return;
		}

		unsigned char wheelIndex = context.GetArgument<int>(1);

		if (fwEntity* vehicle = getAndCheckVehicle(context, "GET_VEHICLE_WHEEL_SPEED"))
		{
			unsigned char numWheels = readValue<unsigned char>(vehicle, NumWheelsOffset);
			if (wheelIndex >= numWheels)
			{
				context.SetResult<float>(0.0f);
				return;
			}

			auto wheelsAddress = readValue<uint64_t>(vehicle, WheelsPtrOffset);
			auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelsAddress + 0x008 * wheelIndex);
			float speed = -*reinterpret_cast<float*>(wheelAddr + WheelRotationSpeedOffset);
			auto tyreRadius = *reinterpret_cast<float*>(wheelAddr + WheelTyreRadiusOffset);
			context.SetResult<float>(speed * tyreRadius);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DRAWN_WHEEL_ANGLE_MULT", [](fx::ScriptContext& context)
	{
		if (fwEntity* vehicle = getAndCheckVehicle(context, "GET_VEHICLE_DRAWN_WHEEL_ANGLE_MULT"))
		{
			auto infoAddress = readValue<uint64_t>(vehicle, ModelInfoPtrOffset);
			float angle = *reinterpret_cast<float*>(infoAddress + DrawnWheelAngleMultOffset);
			context.SetResult<float>(angle);
		}
	});

	auto makeWheelFunction = [](auto cb)
	{
		return [=](fx::ScriptContext& context)
		{
			if (context.GetArgumentCount() < 2)
			{
				context.SetResult<float>(0.0f);
				return;
			}

			unsigned char wheelIndex = context.GetArgument<int>(1);

			if (fwEntity* vehicle = getAndCheckVehicle(context, "makeWheelFunction"))
			{
				unsigned char numWheels = readValue<unsigned char>(vehicle, NumWheelsOffset);
				if (wheelIndex >= numWheels)
				{
					return;
				}

				auto wheelsAddress = readValue<uint64_t>(vehicle, WheelsPtrOffset);
				auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelsAddress + 0x008 * wheelIndex);

				cb(context, vehicle, wheelAddr);
			}
		};
	};

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_BRAKE_PRESSURE", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelBrakePressureOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_BRAKE_PRESSURE", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<float*>(wheelAddr + WheelBrakePressureOffset) = context.GetArgument<float>(2);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_POWER", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelPowerOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_POWER", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<float*>(wheelAddr + WheelPowerOffset) = context.GetArgument<float>(2);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_TRACTION_VECTOR_LENGTH", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelTractionVectorLengthOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_TRACTION_VECTOR_LENGTH", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<float*>(wheelAddr + WheelTractionVectorLengthOffset) = context.GetArgument<float>(2);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_ROTATION_SPEED", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelRotationSpeedOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_ROTATION_SPEED", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<float*>(wheelAddr + WheelRotationSpeedOffset) = context.GetArgument<float>(2);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_SUSPENSION_COMPRESSION", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelSuspensionCompressionOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_FLAGS", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<uint16_t>(*reinterpret_cast<uint16_t*>(wheelAddr + WheelFlagsOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_FLAGS", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<uint16_t*>(wheelAddr + WheelFlagsOffset) = context.GetArgument<uint16_t>(2);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_IS_POWERED", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		auto wheelFlags = *reinterpret_cast<uint16_t*>(wheelAddr + WheelFlagsOffset);
		context.SetResult<bool>(wheelFlags & 0x10);
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_IS_POWERED", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		auto wheelFlags = *reinterpret_cast<uint16_t*>(wheelAddr + WheelFlagsOffset);
		*reinterpret_cast<uint16_t*>(wheelAddr + WheelFlagsOffset) = context.GetArgument<bool>(2) ? wheelFlags | 0x10 : wheelFlags & ~0x10;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_STEERING_ANGLE", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelSteeringAngleOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_HEALTH", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelHealthOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_SURFACE_MATERIAL", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<unsigned char>(*reinterpret_cast<unsigned char*>(wheelAddr + WheelSurfaceMaterialOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_HEALTH", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		if (context.GetArgumentCount() < 3)
		{
			return;
		}

		*reinterpret_cast<float*>(wheelAddr + WheelHealthOffset) = context.GetArgument<float>(2);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_Y_ROTATION", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelYRotOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_Y_ROTATION", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<float*>(wheelAddr + WheelYRotOffset) = context.GetArgument<float>(2);
		*reinterpret_cast<float*>(wheelAddr + WheelInvYRotOffset) = -(context.GetArgument<float>(2));
	}));

	// Wheel collider stuff
	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_TIRE_COLLIDER_SIZE", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelTyreRadiusOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_TIRE_COLLIDER_SIZE", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<float*>(wheelAddr + WheelTyreRadiusOffset) = context.GetArgument<float>(2);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_RIM_COLLIDER_SIZE", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelRimRadiusOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_RIM_COLLIDER_SIZE", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<float*>(wheelAddr + WheelRimRadiusOffset) = context.GetArgument<float>(2);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_TIRE_COLLIDER_WIDTH", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelTyreWidthOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_TIRE_COLLIDER_WIDTH", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<float*>(wheelAddr + WheelTyreWidthOffset) = context.GetArgument<float>(2);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_SIZE", [](fx::ScriptContext& context)
	{
		float result = 0.0f;

		if (fwEntity* vehicle = getAndCheckVehicle(context, "GET_VEHICLE_WHEEL_SIZE"))
		{
			auto drawHandler = *reinterpret_cast<uint64_t*>((uint64_t)vehicle + DrawHandlerPtrOffset);
			auto streamRenderGfx = *reinterpret_cast<uint64_t*>(drawHandler + StreamRenderGfxPtrOffset);

			if (streamRenderGfx != 0)
			{
				result = *(float*)(streamRenderGfx + StreamRenderWheelSizeOffset);
			}
		}

		context.SetResult<float>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_SIZE", [](fx::ScriptContext& context)
	{
		bool success = false;

		if (fwEntity* vehicle = getAndCheckVehicle(context, "SET_VEHICLE_WHEEL_SIZE"))
		{
			auto drawHandler = *reinterpret_cast<uint64_t*>((uint64_t)vehicle + DrawHandlerPtrOffset);
			auto streamRenderGfx = *reinterpret_cast<uint64_t*>(drawHandler + StreamRenderGfxPtrOffset);

			if (streamRenderGfx != 0)
			{
				auto size = context.GetArgument<float>(1);

				if (size != 0)
				{
					*reinterpret_cast<float*>(streamRenderGfx + StreamRenderWheelSizeOffset) = size;
					success = true;
				}
			}
		}
		context.SetResult<bool>(success);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_WIDTH", [](fx::ScriptContext& context)
	{
		float result = 0.0f;

		if (fwEntity* vehicle = getAndCheckVehicle(context, "GET_VEHICLE_WHEEL_WIDTH"))
		{
			auto drawHandler = *reinterpret_cast<uint64_t*>((uint64_t)vehicle + DrawHandlerPtrOffset);
			auto streamRenderGfx = *reinterpret_cast<uint64_t*>(drawHandler + StreamRenderGfxPtrOffset);

			if (streamRenderGfx != 0)
			{
				result = *(float*)(streamRenderGfx + StreamRenderWheelWidthOffset);
			}
		}

		context.SetResult<float>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_WIDTH", [](fx::ScriptContext& context)
	{
		bool success = false;

		if (fwEntity* vehicle = getAndCheckVehicle(context, "SET_VEHICLE_WHEEL_WIDTH"))
		{
			auto drawHandler = *reinterpret_cast<uint64_t*>((uint64_t)vehicle + DrawHandlerPtrOffset);
			auto streamRenderGfx = *reinterpret_cast<uint64_t*>(drawHandler + StreamRenderGfxPtrOffset);

			if (streamRenderGfx != 0)
			{
				auto width = context.GetArgument<float>(1);

				if (width != 0)
				{
					*reinterpret_cast<float*>(streamRenderGfx + StreamRenderWheelWidthOffset) = width;
					success = true;
				}
			}
		}

		context.SetResult<bool>(success);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_STEERING_ANGLE", [](fx::ScriptContext& context)
	{
		if (fwEntity* vehicle = getAndCheckVehicle(context, "GET_VEHICLE_STEERING_ANGLE"))
		{
			float angle = readValue<float>(vehicle, SteeringAngleOffset);

			context.SetResult<float>(angle * (180.0f / 3.14159265358979323846));
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_STEERING_ANGLE", [](fx::ScriptContext& context)
	{
		float angle = context.GetArgument<float>(1);

		if (fwEntity* vehicle = getAndCheckVehicle(context, "SET_VEHICLE_STEERING_ANGLE"))
		{
			writeValue<float>(vehicle, SteeringAngleOffset, angle / (180.0f / 3.14159265358979323846));
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_STEERING_SCALE", std::bind(readVehicleMemory<float, &SteeringScaleOffset>, _1, "GET_VEHICLE_STEERING_SCALE"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_STEERING_SCALE", std::bind(writeVehicleMemory<float, &SteeringScaleOffset>, _1, "SET_VEHICLE_STEERING_SCALE"));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_ALARM_SET", [](fx::ScriptContext& context)
	{
		if (fwEntity* vehicle = getAndCheckVehicle(context, "IS_VEHICLE_ALARM_SET"))
		{
			unsigned short alarmTime = readValue<unsigned short>(vehicle, IsAlarmSetOffset);

			context.SetResult<bool>(alarmTime == std::numeric_limits<unsigned short>::max());
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ALARM_TIME_LEFT", [](fx::ScriptContext& context)
	{
		if (fwEntity* vehicle = getAndCheckVehicle(context, "GET_VEHICLE_ALARM_TIME_LEFT"))
		{
			unsigned short alarmTime = readValue<unsigned short>(vehicle, AlarmTimeLeftOffset);

			int timeLeft = 0;

			if (alarmTime != std::numeric_limits<unsigned short>::max())
			{
				timeLeft = alarmTime;
			}

			context.SetResult<int>(timeLeft);
		}
	});
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_ALARM_TIME_LEFT", [](fx::ScriptContext& context)
	{
		if (fwEntity* vehicle = getAndCheckVehicle(context, "SET_VEHICLE_ALARM_TIME_LEFT"))
		{
			unsigned short alarmTime = context.GetArgument<unsigned short>(1);

			if (alarmTime != std::numeric_limits<unsigned short>::max())
			{
				writeValue<unsigned short>(vehicle, AlarmTimeLeftOffset, alarmTime);
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_WANTED", std::bind(readVehicleMemoryBit<&IsWantedOffset, 3>, _1, "IS_VEHICLE_WANTED"));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_PREVIOUSLY_OWNED_BY_PLAYER", std::bind(readVehicleMemoryBit<&PreviouslyOwnedByPlayerOffset, 1>, _1, "IS_VEHICLE_PREVIOUSLY_OWNED_BY_PLAYER"));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_NEEDS_TO_BE_HOTWIRED", std::bind(readVehicleMemoryBit<&NeedsToBeHotwiredOffset, 2>, _1, "IS_VEHICLE_NEEDS_TO_BE_HOTWIRED"));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_INTERIOR_LIGHT_ON", std::bind(readVehicleMemoryBit<&IsInteriorLightOnOffset, 6>, _1, "IS_VEHICLE_INTERIOR_LIGHT_ON"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_INDICATOR_LIGHTS", [](fx::ScriptContext& context)
	{
		if (fwEntity* vehicle = getAndCheckVehicle(context, "GET_VEHICLE_INDICATOR_LIGHTS"))
		{
			// We're only interested in the 2 lowest bits, higher bits are utilized by different properties such as IsInteriorLightOnOffset
			context.SetResult<unsigned char>(readValue<unsigned char>(vehicle, BlinkerStateOffset) & 3);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_LIGHT_MULTIPLIER", std::bind(readVehicleMemory<float, &LightMultiplierGetOffset>, _1, "GET_VEHICLE_LIGHT_MULTIPLIER"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEELIE_STATE", std::bind(readVehicleMemory<unsigned char, &WheelieStateOffset>, _1, "GET_VEHICLE_WHEELIE_STATE"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEELIE_STATE", std::bind(writeVehicleMemory<unsigned char, &WheelieStateOffset>, _1, "SET_VEHICLE_WHEELIE_STATE"));

	fx::ScriptEngine::RegisterNativeHandler("GET_TRAIN_CURRENT_TRACK_NODE", [](fx::ScriptContext& context)
	{
		int trackNode = -1;

		if (fwEntity* vehicle = getAndCheckVehicle(context, "GET_TRAIN_CURRENT_TRACK_NODE"))
		{
			if (readValue<int>(vehicle, VehicleTypeOffset) == 14) // is vehicle a train
			{
				trackNode = readValue<int>(vehicle, TrainTrackNodeIndexOffset);
			}
		}

		context.SetResult<int>(trackNode);
	});

	auto makeTrainFunction = [](auto cb)
	{
		return [=](fx::ScriptContext& context)
		{
			if (fwEntity* vehicle = getAndCheckVehicle(context, "makeTrainFunction"))
			{
				if (vehicle->IsOfType<CVehicle>() && readValue<int>(vehicle, VehicleTypeOffset) == 14)
				{
					cb(context, vehicle);
				}
			}
		};
	};

	fx::ScriptEngine::RegisterNativeHandler("SET_TRAINS_FORCE_DOORS_OPEN", [](fx::ScriptContext& context)
	{
		bool forceOpen = context.GetArgument<bool>(0);
		*g_trainsForceDoorsOpen = forceOpen;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TRAIN_DOOR_COUNT", makeTrainFunction([](fx::ScriptContext& context, fwEntity* train)
	{
		context.SetResult(readValue<int>(train, TrainDoorCountOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_TRAIN_DOOR_OPEN_RATIO", makeTrainFunction([](fx::ScriptContext& context, fwEntity* train)
	{
		int doorIndex = context.GetArgument<int>(1);
		if (doorIndex < 0 || doorIndex >= readValue<int>(train, TrainDoorCountOffset)) return;

		float ratio = GetTrainDoor(train, doorIndex)->ratio;
		context.SetResult(ratio);
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_TRAIN_DOOR_OPEN_RATIO", makeTrainFunction([](fx::ScriptContext& context, fwEntity* train)
	{
		int doorIndex = context.GetArgument<int>(1);
		if (doorIndex < 0 || doorIndex >= readValue<int>(train, TrainDoorCountOffset)) return;

		float ratio = context.GetArgument<float>(2);
		if (ratio < 0.0f || ratio > 1.0f) return;

		GetTrainDoor(train, doorIndex)->ratio = ratio;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_X_OFFSET", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelXOffsetOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_X_OFFSET", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<float*>(wheelAddr + WheelXOffsetOffset) = context.GetArgument<float>(2);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_TOP_SPEED_MODIFIER", [](fx::ScriptContext& context)
	{
		float result = -1.0f;

		if (fwEntity* vehicle = getAndCheckVehicle(context, "GET_VEHICLE_TOP_SPEED_MODIFIER"))
		{
			auto value = (uint64_t)vehicle + *(uint32_t*)(VehicleTopSpeedModifierPtr + 6) + *(uint32_t*)(VehicleTopSpeedModifierPtr + 32);
			result = *(float*)value;
		}

		context.SetResult<float>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CHEAT_POWER_INCREASE", [](fx::ScriptContext& context)
	{
		float result = -1.0f;

		if (fwEntity* vehicle = getAndCheckVehicle(context, "GET_VEHICLE_CHEAT_POWER_INCREASE"))
		{
			result = *(float*)((uint64_t)vehicle + VehicleCheatPowerIncreaseOffset);
		}

		context.SetResult<float>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_FLY_THROUGH_WINDSCREEN_PARAMS", [](fx::ScriptContext& context)
	{
		bool success = false;
		auto paramsCount = g_flyThroughWindscreenParams.size();

		if (context.GetArgumentCount() == paramsCount)
		{
			for (int i = 0; i < paramsCount; i++)
			{
				auto entry = g_flyThroughWindscreenParams.at(i);
				*entry.currentPtr = context.GetArgument<float>(i);
			}

			success = true;
		}

		context.SetResult<bool>(success);
	});

	fx::ScriptEngine::RegisterNativeHandler("RESET_FLY_THROUGH_WINDSCREEN_PARAMS", [](fx::ScriptContext& context)
	{
		ResetFlyThroughWindscreenParams();
	});

	static struct : jitasm::Frontend
	{
		static bool ShouldSkipRepairFunc(fwEntity* VehPointer)
		{
			return g_skipRepairVehicles.find(VehPointer) != g_skipRepairVehicles.end();
		}
		virtual void InternalMain() override
		{
			//restore dl after asm call
			mov(dl, 1);

			//save parameter registers
			push(rax);
			push(rcx);
			push(rdx);
			push(r8);

			sub(rsp, 0x28);
			mov(rax, (uintptr_t)ShouldSkipRepairFunc);
			call(rax);
			add(rsp, 0x28);

			//restore parameter registers
			pop(r8);
			pop(rdx);
			pop(rcx);

			test(al, al);
			jne("skiprepair");
			pop(rax);
			sub(rsp, 0x28);
			AppendInstr(jitasm::InstrID::I_CALL, 0xFF, 0, jitasm::Imm8(2), qword_ptr[rax + VehicleRepairMethodVtableOffset]);
			add(rsp, 0x28);
			ret();
			L("skiprepair");
			pop(rax);
			ret();
		}
	} asmfunc;

	if (GetModuleHandle(L"AdvancedHookV.dll") == nullptr)
	{
		{
			auto repairFunc = hook::get_pattern("F7 D0 48 8B CB 21 83 ? ? ? ? E8 ? ? ? ? 48 8B 03", 27);
			hook::nop(repairFunc, 6);
			hook::call_reg<2>(repairFunc, asmfunc.GetCode());
		}

		{
			auto repairFunc = hook::get_pattern("FF 90 ? ? ? ? 8A 83 ? ? ? ? 24 07");
			hook::nop(repairFunc, 6);
			hook::call_reg<2>(repairFunc, asmfunc.GetCode());
		}
	}

	rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_CORE)
		{
			ResetFlyThroughWindscreenParams();
		}
	});

	OnMainGameFrame.Connect([]()
	{
		*g_flyThroughWindscreenDisabled = *isNetworkGame && !isFlyThroughWindscreenEnabledConVar;
	});

	OnKillNetworkDone.Connect([]()
	{
		g_skipRepairVehicles.clear();
		ResetFlyThroughWindscreenParams();
		*g_trainsForceDoorsOpen = true;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_AUTO_REPAIR_DISABLED", [](fx::ScriptContext& context) {
		auto vehHandle = context.GetArgument<int>(0);
		auto shouldDisable = context.GetArgument<bool>(1);

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(vehHandle);

		if (shouldDisable)
		{
			g_skipRepairVehicles.insert(entity);
		}
		else
		{
			g_skipRepairVehicles.erase(entity);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("ADD_VEHICLE_DELETION_TRACE", [](fx::ScriptContext& context)
	{
		auto vehHandle = context.GetArgument<int>(0);
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(vehHandle);

		if (entity->IsOfType<CVehicle>())
		{
			g_deletionTraces.insert(entity);
			g_deletionTraces2.insert(entity);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("OVERRIDE_VEHICLE_PEDS_CAN_STAND_ON_TOP_FLAG", [](fx::ScriptContext& context)
	{
		auto vehHandle = context.GetArgument<int>(0);
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(vehHandle);

		if (entity->IsOfType<CVehicle>())
		{
			bool can = context.GetArgument<bool>(1);
			SetCanPedStandOnVehicle(entity, can ? 1 : -1);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("RESET_VEHICLE_PEDS_CAN_STAND_ON_TOP_FLAG", [](fx::ScriptContext& context)
	{
		auto vehHandle = context.GetArgument<int>(0);
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(vehHandle);

		if (entity->IsOfType<CVehicle>())
		{
			SetCanPedStandOnVehicle(entity, 0);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("OVERRIDE_PEDS_CAN_STAND_ON_TOP_FLAG", [](fx::ScriptContext& context)
	{
		g_overrideCanPedStandOnVehicle = context.GetArgument<bool>(0);
	});

	// vehicle xenon lights patches to support RGB colors
	{
		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				// original code
				movss(dword_ptr[rbp + 0x1C], xmm2);
				movss(dword_ptr[rbp + 0x18], xmm1);
				movss(dword_ptr[rbp + 0x14], xmm0);

				// save registers
				push(rax);
				push(rcx);
				push(rdx);
				push(r8);

				sub(rsp, 0x28);

				// prepare arguments
				mov(rcx, rbx); // vehicle
				lea(rdx, dword_ptr[rbp + 0x10]); // float rgb for light cones
				lea(r8, dword_ptr[rbp + 0x84]); // uint argb for light flares

				mov(rax, (uintptr_t)OverrideVehicleXenonColor);
				call(rax);

				add(rsp, 0x28);

				// restore registers
				pop(r8);
				pop(rdx);
				pop(rcx);
				pop(rax);

				ret();
			}
		} vehicleHeadlightsColorStub;

		{
			auto location = hook::get_pattern("0F C6 D2 FF F3 0F 11 55 1C F3", 4);
			hook::nop(location, 15);
			hook::call(location, vehicleHeadlightsColorStub.GetCode());
		}

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				// original code
				movss(dword_ptr[rbp - 0x74], xmm2);
				movss(dword_ptr[rbp - 0x78], xmm1);

				// save registers
				push(rax);
				push(rcx);
				push(rdx);
				push(r8);

				sub(rsp, 0x28);

				// prepare arguments
				mov(rcx, rsi); // vehicle
				lea(rdx, dword_ptr[rbp - 0x80]); // float rgb for light cones
				lea(r8, dword_ptr[rbp - 0x34]); // uint argb for light flares

				mov(rax, (uintptr_t)OverrideVehicleXenonColor);
				call(rax);

				add(rsp, 0x28);

				// restore registers
				pop(r8);
				pop(rdx);
				pop(rcx);
				pop(rax);

				ret();
			}
		} vehicleHighbeamsColorStub;

		{
			auto location = hook::get_pattern("F3 0F 11 55 80 0F C6 CA AA", 13);
			hook::nop(location, 10);
			hook::call(location, vehicleHighbeamsColorStub.GetCode());
		}
	}

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_XENON_LIGHTS_CUSTOM_COLOR", [](fx::ScriptContext& context)
	{
		if (fwEntity* vehicle = getAndCheckVehicle(context, "SET_VEHICLE_XENON_LIGHTS_CUSTOM_COLOR"))
		{
			auto colorR = context.GetArgument<uint8_t>(1);
			auto colorG = context.GetArgument<uint8_t>(2);
			auto colorB = context.GetArgument<uint8_t>(3);

			if (auto it = g_vehicleXenonLightsColors.find(vehicle); it != g_vehicleXenonLightsColors.end())
			{
				it->second.Update(colorR, colorG, colorB);
			}
			else
			{
				VehicleXenonLightsColor color(colorR, colorG, colorB);
				g_vehicleXenonLightsColors.insert({ vehicle, color });
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_XENON_LIGHTS_CUSTOM_COLOR", [](fx::ScriptContext& context)
	{
		uint8_t colorR = 0;
		uint8_t colorG = 0;
		uint8_t colorB = 0;
		bool valid = false;

		if (fwEntity* vehicle = getAndCheckVehicle(context, "GET_VEHICLE_XENON_LIGHTS_CUSTOM_COLOR"))
		{
			if (auto it = g_vehicleXenonLightsColors.find(vehicle); it != g_vehicleXenonLightsColors.end())
			{
				uint32_t colorARGB = it->second.colorARGB;
				colorR = (colorARGB & 0xFF0000) >> 16;
				colorG = (colorARGB & 0xFF00) >> 8;
				colorB = (colorARGB & 0xFF);
				valid = true;
			}
		}

		*context.GetArgument<uint8_t*>(1) = colorR;
		*context.GetArgument<uint8_t*>(2) = colorG;
		*context.GetArgument<uint8_t*>(3) = colorB;

		context.SetResult<bool>(valid);
	});

	fx::ScriptEngine::RegisterNativeHandler("CLEAR_VEHICLE_XENON_LIGHTS_CUSTOM_COLOR", [](fx::ScriptContext& context)
	{
		if (fwEntity* vehicle = getAndCheckVehicle(context, "CLEAR_VEHICLE_XENON_LIGHTS_CUSTOM_COLOR"))
		{
			g_vehicleXenonLightsColors.erase(vehicle);
		}
	});

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("E8 ? ? ? ? 8A 83 DA 00 00 00 24 0F 3C 02", -0x32), DeleteVehicleWrap, (void**)&g_origDeleteVehicle);
	MH_CreateHook(hook::get_pattern("80 7A 4B 00 45 8A F9", -0x1D), DeleteNetworkCloneWrap, (void**)&g_origDeleteNetworkClone);
	MH_CreateHook(hook::get_call(hook::get_pattern("74 22 48 8B CA E8 ? ? ? ? 84 C0 74 16", 5)), CanPedStandOnVehicleWrap, (void**)&g_origCanPedStandOnVehicle);
	MH_CreateHook(hook::get_call(hook::get_pattern("48 8B 4F 20 48 8D 54 24 ? E8", 0x9)), DashboardHandler, (void**)&g_origDashboardHandler);
	MH_EnableHook(MH_ALL_HOOKS);
});

#include <scrEngine.h>

int GetWheelCount(CVehicle* vehicle)
{
	return readValue<int>(vehicle, NumWheelsOffset);
}

float CalculateWheelValue(CVehicle* vehicle, int wheelCount, bool drive)
{
	float val = 0.f;

	auto isFront = [wheelCount](int idx)
	{
		return (wheelCount > 2) ? (idx < 2) : (idx == 0);
	};

	auto handling = vehicle->GetHandlingData();
	auto handlingPtr = (char*)handling;

	for (size_t wheelIndex = 0; wheelIndex < wheelCount; wheelIndex++)
	{
		auto wheelsAddress = readValue<uint64_t>(vehicle, WheelsPtrOffset);
		auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelsAddress + (8 * wheelIndex));

		float frontBias = (drive) ? *(float*)(handlingPtr + 0x48) : *(float*)(handlingPtr + 0x74);
		float rearBias = (drive) ? *(float*)(handlingPtr + 0x4C) : *(float*)(handlingPtr + 0x78);

		val += *(float*)(wheelAddr + WheelTractionVectorLengthOffset) * (isFront(wheelIndex) ? frontBias : rearBias) * ((wheelCount == 2) ? 1.0f : 0.5f);
	}

	return val;
}

static DWORD(WINAPI* g_origXInputGetState)(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE* pState);

static bool g_useWGI = true;

static std::mutex gamepadMutex;
static std::vector<Gamepad> g_gamepads;
static Gamepad::GamepadAdded_revoker addedRevoker;
static Gamepad::GamepadRemoved_revoker removedRevoker;

void OnGamepadAdded(winrt::Windows::Foundation::IInspectable const& /* sender */, Gamepad const& gamepad)
{
	std::lock_guard<std::mutex> _(gamepadMutex);
	
	g_gamepads.push_back(gamepad);
}

void OnGamepadRemoved(winrt::Windows::Foundation::IInspectable const& /* sender */, Gamepad const& gamepad)
{
	std::lock_guard<std::mutex> _(gamepadMutex);

	g_gamepads.erase(std::remove(g_gamepads.begin(), g_gamepads.end(), gamepad), g_gamepads.end());
}


static DWORD WINAPI XInputGetStateHook(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE* pState)
{
	std::lock_guard<std::mutex> _(gamepadMutex);

	if (!g_useWGI || g_gamepads.size() == 0)
	{
		// e.g. if using some sort of hook
		return g_origXInputGetState(dwUserIndex, pState);
	}

	if (dwUserIndex < g_gamepads.size())
	{
		auto gamepad = g_gamepads[dwUserIndex];
		auto reading = gamepad.GetCurrentReading();

		auto mapShort = [](float val)
		{
			return (val > 0) ? (val * 32767.f) : (val * 32768.f);
		};

		pState->dwPacketNumber = reading.Timestamp;

		pState->Gamepad.sThumbLX = mapShort(reading.LeftThumbstickX);
		pState->Gamepad.sThumbLY = mapShort(reading.LeftThumbstickY);
		pState->Gamepad.sThumbRX = mapShort(reading.RightThumbstickX);
		pState->Gamepad.sThumbRY = mapShort(reading.RightThumbstickY);
		pState->Gamepad.bLeftTrigger = reading.LeftTrigger * 255.f;
		pState->Gamepad.bRightTrigger = reading.RightTrigger * 255.f;
		pState->Gamepad.wButtons = 0;

		auto mapButton = [&reading, pState](GamepadButtons button, DWORD value)
		{
			if ((int)(reading.Buttons & button) != 0)
			{
				pState->Gamepad.wButtons |= value;
			}
		};

		mapButton(GamepadButtons::A, XINPUT_GAMEPAD_A);
		mapButton(GamepadButtons::B, XINPUT_GAMEPAD_B);
		mapButton(GamepadButtons::X, XINPUT_GAMEPAD_X);
		mapButton(GamepadButtons::Y, XINPUT_GAMEPAD_Y);
		mapButton(GamepadButtons::LeftThumbstick, XINPUT_GAMEPAD_LEFT_THUMB);
		mapButton(GamepadButtons::LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
		mapButton(GamepadButtons::RightThumbstick, XINPUT_GAMEPAD_RIGHT_THUMB);
		mapButton(GamepadButtons::RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);
		mapButton(GamepadButtons::Menu, XINPUT_GAMEPAD_START);
		mapButton(GamepadButtons::View, XINPUT_GAMEPAD_BACK);
		mapButton(GamepadButtons::DPadUp, XINPUT_GAMEPAD_DPAD_UP);
		mapButton(GamepadButtons::DPadDown, XINPUT_GAMEPAD_DPAD_DOWN);
		mapButton(GamepadButtons::DPadLeft, XINPUT_GAMEPAD_DPAD_LEFT);
		mapButton(GamepadButtons::DPadRight, XINPUT_GAMEPAD_DPAD_RIGHT);

		auto vibration = gamepad.Vibration();
		auto origVibration = vibration;

		// add vehicle bits
		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			static auto vibrationVar = console::GetDefaultContext()->GetVariableManager()->FindEntryRaw("profile_vibration");

			if (!vibrationVar || vibrationVar->GetValue() != "0")
			{
				// #TODO: gun recoil
				// #TODO: take trigger swap into account
				int veh = NativeInvoke::Invoke<0x6094AD011A2EA87D, int>(NativeInvoke::Invoke<0xD80958FC74E988A6, int>());

				CVehicle* currentPlayerVehicle = nullptr;

				if (veh)
				{
					currentPlayerVehicle = (CVehicle*)rage::fwScriptGuid::GetBaseFromGuid(veh);
				}

				if (currentPlayerVehicle)
				{
					if (GetWheelCount(currentPlayerVehicle) == 2 || GetWheelCount(currentPlayerVehicle) == 4)
					{
						double leftTrigger = CalculateWheelValue(currentPlayerVehicle, GetWheelCount(currentPlayerVehicle), false) / 2.5f;
						double rightTrigger = CalculateWheelValue(currentPlayerVehicle, GetWheelCount(currentPlayerVehicle), true) / 2.5f;

						// #TODO: make these tunable as some sort of 3-point curve
						leftTrigger = std::clamp(leftTrigger - 0.35f, 0.0, 25.0);
						rightTrigger = std::clamp(rightTrigger - 0.35f, 0.0, 25.0);

						vibration.LeftTrigger = std::clamp(reading.LeftTrigger * (leftTrigger / 25.0f), 0.0, 1.0);
						vibration.RightTrigger = std::clamp(reading.RightTrigger * (rightTrigger / 25.0f), 0.0, 1.0);
					}
				}
			}
		}

		if (origVibration != vibration)
		{
			gamepad.Vibration(vibration);
		}
	}

	return ERROR_SUCCESS;
}

static DWORD(*WINAPI g_origXInputSetState)(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration);

static DWORD WINAPI XInputSetStateHook(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration)
{
	std::lock_guard<std::mutex> _(gamepadMutex);

	if (!g_useWGI || g_gamepads.size() == 0)
	{
		// e.g. if using some sort of hook
		return g_origXInputSetState(dwUserIndex, pVibration);
	}

	if (dwUserIndex < g_gamepads.size())
	{
		auto gamepad = g_gamepads[dwUserIndex];

		GamepadVibration vibration = gamepad.Vibration();
		vibration.LeftMotor = pVibration->wLeftMotorSpeed / 65535.f;
		vibration.RightMotor = pVibration->wRightMotorSpeed / 65535.f;

		gamepad.Vibration(vibration);
	}

	return ERROR_SUCCESS;
}

static HookFunction inputFunction([]()
{
	DWORDLONG viMask = 0;
	OSVERSIONINFOEXW osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwBuildNumber = 15063; // RS2+

	VER_SET_CONDITION(viMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

	if (!VerifyVersionInfoW(&osvi, VER_BUILDNUMBER, viMask))
	{
		return;
	}

	auto getStateRef = hook::get_address<void**>(hook::get_call(hook::get_pattern<char>("75 13 48 8D 54 24 20 8B CF E8", 9)) + 2);
	auto setStateRef = hook::get_address<void**>(hook::get_call(hook::get_pattern<char>("8B CF 89 73 42 89 74 24 40 E8", 9)) + 2);

	static auto initializer = DeferredInitializer::Create([getStateRef, setStateRef]()
	{
		HMODULE hLib = LoadLibraryW(L"Windows.Gaming.Input.dll");

		if (!hLib)
		{
			return;
		}

		addedRevoker = Gamepad::GamepadAdded(winrt::auto_revoke, OnGamepadAdded);
		removedRevoker = Gamepad::GamepadRemoved(winrt::auto_revoke, OnGamepadRemoved);

		if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)hLib, &hLib))
		{
			trace("Failed to pin WGI DLL.\n");
		}

		{
			g_origXInputGetState = (decltype(g_origXInputGetState))*getStateRef;
			hook::put(getStateRef, XInputGetStateHook);
		}

		{
			g_origXInputSetState = (decltype(g_origXInputSetState))*setStateRef;
			hook::put(setStateRef, XInputSetStateHook);
		}
	});
});
