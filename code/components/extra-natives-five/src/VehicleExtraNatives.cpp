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
		traceFn("No such entity");
		return nullptr;
	}

	if (!vehicle->IsOfType<CVehicle>())
	{
		traceFn("Can not read from an entity that is not a vehicle");
		return nullptr;
	}

	return vehicle;
}

template<typename T, int offset, typename U = T>
static void readVehicleMemory(fx::ScriptContext& context, std::string_view nn)
{
	if (fwEntity* vehicle = getAndCheckVehicle(context, nn))
	{
		context.SetResult<U>((U)readValue<T>(vehicle, offset));
	}
}

template<int offset, int bit>
static void readVehicleMemoryBit(fx::ScriptContext& context, std::string_view nn)
{
	if (fwEntity* vehicle = getAndCheckVehicle(context, nn))
	{
		std::bitset<sizeof(int) * 8> value(readValue<int>(vehicle, offset));

		context.SetResult<bool>(value[bit]);
	}
}

template<int offset, unsigned char bit>
static void writeVehicleMemoryBit(fx::ScriptContext& context, std::string_view nn)
{
	if (context.GetArgumentCount() < 2)
	{
		trace("Insufficient arguments count, 2 expected");
		return;
	}

	if (fwEntity* vehicle = getAndCheckVehicle(context, nn))
	{
		std::bitset<sizeof(int) * 8> value(readValue<int>(vehicle, offset));
		value[bit] = context.GetArgument<bool>(1);
		writeValue<unsigned char>(vehicle, offset, static_cast<unsigned char>(value.to_ulong()));
	}
}

template<typename T, int offset>
static void writeVehicleMemory(fx::ScriptContext& context, std::string_view nn)
{
	if (context.GetArgumentCount() < 2)
	{
		trace("Insufficient arguments count, 2 expected");
		return;
	}

	if (fwEntity* vehicle = getAndCheckVehicle(context, nn))
	{
		writeValue<T>(vehicle, offset, context.GetArgument<T>(1));
	}
}

// 1290 now
// #TODO1365
// #TODO1493
// #TODO1604 <- really this time they changed since 1290
const int HeliMainRotorHealthOffset = 0x1AB0;
const int HeliTailRotorHealthOffset = 0x1AB4;
const int HeliEngineHealthOffset = 0x1AB8;
const int FuelLevelOffset = 0x834;
const int OilLevelOffset = 0x838;
const int GravityOffset = 0xC1C;
const int IsEngineStartingOffset = 0x92A;
const int DashSpeedOffset = 0xA80;
const int HeliBladesSpeedOffset = 0x1AA8;
const int AccelerationOffset = 0x8C4;
const int CurrentRPMOffset = 0x8B0;
const int HighGearOffset = 0x876;
const int CurrentGearOffset = 0x872;
const int NextGearOffset = 0x870;
const int RpmOffset = 0x8C4;
const int ClutchOffset = 0x8C0;
const int TurboBoostOffset = 0x8D8;
const int ThrottleInputOffset = 0x99C;
const int BrakeInputOffset = 0x9A0;
const int HandbrakeOffset = 0x9A4;
const int EngineTempOffset = 0xA4C;
const int NumWheelsOffset = 0xBB8;
const int WheelsPtrOffset = 0xBB0;
const int ModelInfoPtrOffset = 0x020;
const int DrawnWheelAngleMultOffset = 0x49c;

const int SteeringAngleOffset = 0x994;
const int SteeringScaleOffset = 0x99C;
const int IsAlarmSetOffset = 0xA88;
const int AlarmTimeLeftOffset = 0xA88;
const int IsWantedOffset = 0x934;
const int ProvidesCoverOffset = 0x924;
const int PreviouslyOwnedByPlayerOffset = 0x92C;
const int NeedsToBeHotwiredOffset = 0x92C;
const int IsInteriorLightOnOffset = 0x929;
const int LodMultiplierOffset = 0x1328;
const int IsLeftHeadLightBrokenOffset = 0x84C;
const int IsRightHeadLightBrokenOffset = 0x84C;
const int EnginePowerMultiplierOffset = 0xAC0;
const int CanWheelsBreakOffset = 0x923; // todo - check?
const int BlinkerState = 0x929;
const int WheelieState = 0x14F9;
const int VehicleTypeOffset = 0xBA8;
const int TrainTrackNodeIndex = 0x14C0;

// Wheel class
const int WheelXOffsetOffset = 0x030;
const int WheelTyreRadiusOffset = 0x110;
const int WheelRimRadiusOffset = 0x114;
const int WheelTyreWidthOffset = 0x118;
const int WheelRotationSpeedOffset = 0x170;
const int WheelHealthOffset = 0x1E8; // 75 24 F3 0F 10 81 ? ? ? ? F3 0F
const int WheelYRotOffset = 0x008;
const int WheelInvYRotOffset = 0x010;
const int WheelBrakePressureOffset = 0x1D0;
const int WheelSteeringAngleOffset = 0x1CC;

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
	void* handling = readValue<void*>(vehicle, 0x918);

	// call original destructor
	g_origDeleteVehicle(vehicle);

	// run cleanup after destructor
	g_skipRepairVehicles.erase(vehicle);

	// Delete the handling if it has been set to hooked.
	if (*((char*)handling + 28) == 1)
		delete handling;
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
	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_FUEL_LEVEL", std::bind(readVehicleMemory<float, FuelLevelOffset>, _1, "GET_VEHICLE_FUEL_LEVEL"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_FUEL_LEVEL", std::bind(writeVehicleMemory<float, FuelLevelOffset>, _1, "SET_VEHICLE_FUEL_LEVEL"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_OIL_LEVEL", std::bind(readVehicleMemory<float, OilLevelOffset>, _1, "GET_VEHICLE_OIL_LEVEL"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_OIL_LEVEL", std::bind(writeVehicleMemory<float, OilLevelOffset>, _1, "SET_VEHICLE_OIL_LEVEL"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_GRAVITY_AMOUNT", std::bind(readVehicleMemory<float, GravityOffset>, _1, "GET_VEHICLE_GRAVITY_AMOUNT"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_GRAVITY_AMOUNT", std::bind(writeVehicleMemory<float, GravityOffset>, _1, "SET_VEHICLE_GRAVITY_AMOUNT"));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_ENGINE_STARTING", std::bind(readVehicleMemoryBit<IsEngineStartingOffset, 5>, _1, "IS_VEHICLE_ENGINE_STARTING"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_SPEED", std::bind(readVehicleMemory<float, DashSpeedOffset>, _1, "GET_VEHICLE_DASHBOARD_SPEED"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CURRENT_ACCELERATION", std::bind(readVehicleMemory<float, AccelerationOffset>, _1, "GET_VEHICLE_CURRENT_ACCELERATION"));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_GRAVITY", std::bind(readVehicleMemory<float, AccelerationOffset>, _1, "SET_VEHICLE_GRAVITY"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CURRENT_RPM", std::bind(readVehicleMemory<float, CurrentRPMOffset>, _1, "GET_VEHICLE_CURRENT_RPM"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_CURRENT_RPM", std::bind(writeVehicleMemory<float, CurrentRPMOffset>, _1, "SET_VEHICLE_CURRENT_RPM"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HIGH_GEAR", std::bind(readVehicleMemory<unsigned char, HighGearOffset>, _1, "GET_VEHICLE_HIGH_GEAR"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_HIGH_GEAR", std::bind(writeVehicleMemory<unsigned char, HighGearOffset>, _1, "SET_VEHICLE_HIGH_GEAR"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CURRENT_GEAR", std::bind(readVehicleMemory<unsigned char, CurrentGearOffset, int>, _1, "GET_VEHICLE_CURRENT_GEAR"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_CURRENT_GEAR", std::bind(writeVehicleMemory<unsigned char, CurrentGearOffset>, _1, "SET_VEHICLE_CURRENT_GEAR"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NEXT_GEAR", std::bind(readVehicleMemory<unsigned char, NextGearOffset, int>, _1, "GET_VEHICLE_NEXT_GEAR"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_NEXT_GEAR", std::bind(writeVehicleMemory<unsigned char, NextGearOffset>, _1, "SET_VEHICLE_NEXT_GEAR"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CLUTCH", std::bind(readVehicleMemory<float, ClutchOffset>, _1, "GET_VEHICLE_CLUTCH"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_CLUTCH", std::bind(writeVehicleMemory<float, ClutchOffset>, _1, "SET_VEHICLE_CLUTCH"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_TURBO_PRESSURE", std::bind(readVehicleMemory<float, TurboBoostOffset>, _1, "GET_VEHICLE_TURBO_PRESSURE"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_TURBO_PRESSURE", std::bind(writeVehicleMemory<float, TurboBoostOffset>, _1, "SET_VEHICLE_TURBO_PRESSURE"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HANDBRAKE", std::bind(readVehicleMemory<bool, HandbrakeOffset>, _1, "GET_VEHICLE_HANDBRAKE")); // just a getter

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ENGINE_TEMPERATURE", std::bind(readVehicleMemory<float, EngineTempOffset>, _1, "GET_VEHICLE_ENGINE_TEMPERATURE"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_ENGINE_TEMPERATURE", std::bind(writeVehicleMemory<float, EngineTempOffset>, _1, "SET_VEHICLE_ENGINE_TEMPERATURE"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NUMBER_OF_WHEELS", std::bind(readVehicleMemory<unsigned char, NumWheelsOffset>, _1, "GET_VEHICLE_NUMBER_OF_WHEELS"));

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
			if (wheelIndex >= numWheels) {
				context.SetResult<float>(0.0f);
				return;
			}
			auto wheelsAddress = readValue<uint64_t>(vehicle, WheelsPtrOffset);
			auto wheelAddr = *reinterpret_cast<uint64_t *>(wheelsAddress + 0x008 * wheelIndex);
			float speed = -*reinterpret_cast<float *>(wheelAddr + WheelRotationSpeedOffset);
			auto tyreRadius = *reinterpret_cast<float *>(wheelAddr + WheelTyreRadiusOffset);
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
				if (wheelIndex >= numWheels) {
					return;
				}
				auto wheelsAddress = readValue<uint64_t>(vehicle, WheelsPtrOffset);
				auto wheelAddr = *reinterpret_cast<uint64_t *>(wheelsAddress + 0x008 * wheelIndex);

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
		context.SetResult<float>(*reinterpret_cast<float *>(wheelAddr + WheelHealthOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_HEALTH", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		if (context.GetArgumentCount() < 3)
		{
			return;
		}

		*reinterpret_cast<float *>(wheelAddr + WheelHealthOffset) = context.GetArgument<float>(2);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_Y_ROTATION", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float *>(wheelAddr + WheelYRotOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_Y_ROTATION", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<float *>(wheelAddr + WheelYRotOffset) = context.GetArgument<float>(2);
		*reinterpret_cast<float *>(wheelAddr + WheelInvYRotOffset) = -(context.GetArgument<float>(2));
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
		fwEntity* vehicle = getAndCheckVehicle(context, "GET_VEHICLE_WHEEL_SIZE");
		auto CVeh_0x48 = *reinterpret_cast<uint64_t*>((uint64_t)vehicle + 0x48);
		auto CVeh_0x48_0x370 = *reinterpret_cast<uint64_t*>(CVeh_0x48 + 0x370);
		if (CVeh_0x48_0x370 != 0) {
			float size = *(float*)(CVeh_0x48_0x370 + 0x8);
			context.SetResult<float>((float)size);
		}else {
			context.SetResult<float>(0);
		}
	});
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_SIZE", [](fx::ScriptContext& context)
	{
		bool success = false;
		if (fwEntity* vehicle = getAndCheckVehicle(context, "SET_VEHICLE_WHEEL_SIZE"))
		{
			auto CVeh_0x48 = *reinterpret_cast<uint64_t*>((uint64_t)vehicle + 0x48);
			auto CVeh_0x48_0x370 = *reinterpret_cast<uint64_t*>(CVeh_0x48 + 0x370);
			if (CVeh_0x48_0x370 != 0) {		// Cannot set visual size for default wheels
				auto size = context.GetArgument<float>(1);
				if (size != 0) {
					*reinterpret_cast<float*>(CVeh_0x48_0x370 + 0x8) = size;	// Set wheel visual size
					success = true;		// Visual size was set properly
				}
			}
		}
		context.SetResult<bool>(success);	// Could not set visual size
	});
	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_WIDTH", [](fx::ScriptContext& context)
	{
		fwEntity* vehicle = getAndCheckVehicle(context, "GET_VEHICLE_WHEEL_WIDTH");
		auto CVeh_0x48 = *reinterpret_cast<uint64_t*>((uint64_t)vehicle + 0x48);
		auto CVeh_0x48_0x370 = *reinterpret_cast<uint64_t*>(CVeh_0x48 + 0x370);
		if (CVeh_0x48_0x370 != 0) {
			float width = *(float*)(CVeh_0x48_0x370 + 0xBA0);
			context.SetResult<float>((float)width);
		}else {
			context.SetResult<float>(0);
		}
	});
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_WIDTH", [](fx::ScriptContext& context)
	{
		bool success = false;
		if (fwEntity* vehicle = getAndCheckVehicle(context, "SET_VEHICLE_WHEEL_WIDTH"))
		{
			auto CVeh_0x48 = *reinterpret_cast<uint64_t*>((uint64_t)vehicle + 0x48);
			auto CVeh_0x48_0x370 = *reinterpret_cast<uint64_t*>(CVeh_0x48 + 0x370);
			if (CVeh_0x48_0x370 != 0) {		// Cannot set visual size for default wheels
				auto width = context.GetArgument<float>(1);
				if (width != 0) {
					*reinterpret_cast<float*>(CVeh_0x48_0x370 + 0xBA0) = width;	// Set wheel visual width
					success = true;		// Visual width was set properly
				}
			}
		}
		context.SetResult<bool>(success);	// Could not set visual size
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

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_STEERING_SCALE", std::bind(readVehicleMemory<float, SteeringScaleOffset>, _1, "GET_VEHICLE_STEERING_SCALE"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_STEERING_SCALE", std::bind(writeVehicleMemory<float, SteeringScaleOffset>, _1, "SET_VEHICLE_STEERING_SCALE"));

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

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_WANTED", std::bind(readVehicleMemoryBit<IsWantedOffset, 3>, _1, "IS_VEHICLE_WANTED"));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_PREVIOUSLY_OWNED_BY_PLAYER", std::bind(readVehicleMemoryBit<PreviouslyOwnedByPlayerOffset, 1>, _1, "IS_VEHICLE_PREVIOUSLY_OWNED_BY_PLAYER"));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_NEEDS_TO_BE_HOTWIRED", std::bind(readVehicleMemoryBit<NeedsToBeHotwiredOffset, 2>, _1, "IS_VEHICLE_NEEDS_TO_BE_HOTWIRED"));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_INTERIOR_LIGHT_ON", std::bind(readVehicleMemoryBit<IsInteriorLightOnOffset, 6>, _1, "IS_VEHICLE_INTERIOR_LIGHT_ON"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_INDICATOR_LIGHTS", std::bind(readVehicleMemory<unsigned char, BlinkerState>, _1, "GET_VEHICLE_INDICATOR_LIGHTS"));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEELIE_STATE", std::bind(readVehicleMemory<unsigned char, WheelieState>, _1, "GET_VEHICLE_WHEELIE_STATE"));
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEELIE_STATE", std::bind(writeVehicleMemory<unsigned char, WheelieState>, _1, "SET_VEHICLE_WHEELIE_STATE"));

	fx::ScriptEngine::RegisterNativeHandler("GET_TRAIN_CURRENT_TRACK_NODE", [](fx::ScriptContext& context)
	{
		int trackNode = -1;

		if (fwEntity* vehicle = getAndCheckVehicle(context, "GET_TRAIN_CURRENT_TRACK_NODE"))
		{
			if (readValue<int>(vehicle, VehicleTypeOffset) == 14) // is vehicle a train
			{
				trackNode = readValue<int>(vehicle, TrainTrackNodeIndex);
			}
		}

		context.SetResult<int>(trackNode);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_X_OFFSET", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float *>(wheelAddr + WheelXOffsetOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_X_OFFSET", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<float *>(wheelAddr + WheelXOffsetOffset) = context.GetArgument<float>(2);
	}));

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
		if (shouldDisable) {
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
