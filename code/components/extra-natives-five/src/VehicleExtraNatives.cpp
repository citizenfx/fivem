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

#include <Local.h>
#include <Hooking.h>
#include <GameInit.h>

#include <limits>
#include <bitset>
#include <unordered_set>

#include <CoreConsole.h>
#include <Resource.h>

#include <fxScripting.h>

#include <MinHook.h>

#include <xinput.h>
#include <winrt/Windows.Gaming.Input.h>

using namespace winrt::Windows::Gaming::Input;

static std::unordered_set<fwEntity*> g_skipRepairVehicles{};

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

// TODO: Not valid, figure out.
static int ClutchOffset = 0x8C0;
static int TurboBoostOffset = 0x8D8;

// TODO: Wheel class.
static int WheelYRotOffset = 0x008;
static int WheelInvYRotOffset = 0x010;
static int WheelXOffsetOffset = 0x030;
static int WheelTyreRadiusOffset = 0x110;
static int WheelRimRadiusOffset = 0x114;
static int WheelTyreWidthOffset = 0x118;
static int WheelRotationSpeedOffset = 0x170;
static int WheelTractionVectorLengthOffset = 0x1B8;
static int WheelSteeringAngleOffset = 0x1CC;
static int WheelBrakePressureOffset = 0x1D0;
static int WheelHealthOffset = 0x1E8; // 75 24 F3 0F 10 81 ? ? ? ? F3 0F

static char* VehicleTopSpeedModifierPtr;
static int VehicleCheatPowerIncreaseOffset;

static std::unordered_set<fwEntity*> g_deletionTraces;
static std::unordered_set<void*> g_deletionTraces2;

static void(*g_origDeleteVehicle)(void* vehicle);

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
	void* handling = readValue<void*>(vehicle, HandlingDataPtrOffset);

	// call original destructor
	g_origDeleteVehicle(vehicle);

	// run cleanup after destructor
	g_skipRepairVehicles.erase(vehicle);

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

static HookFunction initFunction([]()
{
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
	}

	{
		auto location = hook::get_pattern<char>("48 3B CA 0F 84 ? ? ? ? 8B 81");

		FuelLevelOffset = *(uint32_t*)(location + 49);
		OilLevelOffset = *(uint32_t*)(location + 61);
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
		ThrottleOffsetOffset = CurrentRPMOffset + 16;
	}

	{
		auto location = hook::get_pattern<char>("48 89 01 B8 00 00 80 3F 66 44 89 51");

		StreamRenderWheelWidthOffset = *(uint32_t*)(location + 23);
		StreamRenderWheelSizeOffset = *(uint8_t*)(location + 20);
	}

	{
		auto location = hook::get_pattern<char>("44 0F 2F 43 ? 45 8D 74 24 01");

		DrawHandlerPtrOffset = *(uint8_t*)(location + 4);
		HandlingDataPtrOffset = *(uint32_t*)(location - 35);
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

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_STEERING_ANGLE", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelSteeringAngleOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_HEALTH", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float*>(wheelAddr + WheelHealthOffset));
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

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_INDICATOR_LIGHTS", std::bind(readVehicleMemory<unsigned char, &BlinkerStateOffset>, _1, "GET_VEHICLE_INDICATOR_LIGHTS"));

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
			AppendInstr(jitasm::InstrID::I_CALL, 0xFF, 0, jitasm::Imm8(2), qword_ptr[rax + 0x5D0]);
			add(rsp, 0x28);
			ret();
			L("skiprepair");
			pop(rax);
			ret();
		}
	} asmfunc;

	if (GetModuleHandle(L"AdvancedHookV.dll") == nullptr)
	{
		auto repairFunc = hook::get_pattern("48 8B 03 45 33 C0 B2 01 48 8B CB FF 90 D0 05 00 00 48 8B 4B 20", 11);
		hook::nop(repairFunc, 6);
		hook::call_reg<2>(repairFunc, asmfunc.GetCode());
	}

	OnKillNetworkDone.Connect([]() {
		g_skipRepairVehicles.clear();
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_AUTO_REPAIR_DISABLED", [](fx::ScriptContext& context) {
		auto vehHandle = context.GetArgument<int>(0);
		auto shouldDisable = context.GetArgument<bool>(1);

		fwEntity *entity = rage::fwScriptGuid::GetBaseFromGuid(vehHandle);

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

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("E8 ? ? ? ? 8A 83 DA 00 00 00 24 0F 3C 02", -0x32), DeleteVehicleWrap, (void**)&g_origDeleteVehicle);
	MH_CreateHook(hook::get_pattern("80 7A 4B 00 45 8A F9", -0x1D), DeleteNetworkCloneWrap, (void**)&g_origDeleteNetworkClone);
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

static DWORD WINAPI XInputGetStateHook(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE* pState)
{
	auto gamepads = Gamepad::Gamepads();

	if (gamepads.Size() == 0 || !g_useWGI)
	{
		// e.g. if using some sort of hook
		return g_origXInputGetState(dwUserIndex, pState);
	}

	if (dwUserIndex < gamepads.Size())
	{
		auto gamepad = gamepads.GetAt(dwUserIndex);
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

		gamepad.Vibration(vibration);

	}

	return ERROR_SUCCESS;
}

static DWORD(*WINAPI g_origXInputSetState)(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration);

static DWORD WINAPI XInputSetStateHook(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration)
{
	auto gamepads = Gamepad::Gamepads();

	if (gamepads.Size() == 0 || !g_useWGI)
	{
		// e.g. if using some sort of hook
		return g_origXInputSetState(dwUserIndex, pVibration);
	}

	if (dwUserIndex < gamepads.Size())
	{
		auto gamepad = gamepads.GetAt(dwUserIndex);

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

	{
		auto getStateRef = hook::get_address<void**>(hook::get_call(hook::get_pattern<char>("75 13 48 8D 54 24 20 8B CF E8", 9)) + 2);
		g_origXInputGetState = (decltype(g_origXInputGetState))*getStateRef;
		*getStateRef = XInputGetStateHook;
	}

	{
		auto setStateRef = hook::get_address<void**>(hook::get_call(hook::get_pattern<char>("8B CF 89 73 42 89 74 24 40 E8", 9)) + 2);
		g_origXInputSetState = (decltype(g_origXInputSetState))*setStateRef;
		*setStateRef = XInputSetStateHook;
	}
});
