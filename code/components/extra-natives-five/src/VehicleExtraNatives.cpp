/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <ScriptEngine.h>
#include <atArray.h>

#include <Local.h>
#include <Hooking.h>

#include <limits>
#include <bitset>

static hook::cdecl_stub<fwEntity*(int handle)> getScriptEntity([]()
{
	return hook::pattern("44 8B C1 49 8B 41 08 41 C1 F8 08 41 38 0C 00").count(1).get(0).get<void>(-12);
});

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

static fwEntity* getAndCheckVehicle(fx::ScriptContext& context)
{
	if (context.GetArgumentCount() < 1)
	{
		trace("At least one argument must be passed");
		return nullptr;
	}

	fwEntity* vehicle = getScriptEntity(context.GetArgument<int>(0));

	if (!vehicle)
	{
		trace("No such entity");
		return nullptr;
	}

	if (!vehicle->IsOfType<CVehicle>())
	{
		trace("Can not read from entity that is not vehicle");
		return nullptr;
	}

	return vehicle;
}

template<typename T, int offset, typename U = T>
static void readVehicleMemory(fx::ScriptContext& context)
{
	if (fwEntity* vehicle = getAndCheckVehicle(context))
	{
		context.SetResult<U>((U)readValue<T>(vehicle, offset));
	}
}

template<int offset, int bit>
static void readVehicleMemoryBit(fx::ScriptContext& context)
{
	if (fwEntity* vehicle = getAndCheckVehicle(context))
	{
		std::bitset<sizeof(int)> value(readValue<int>(vehicle, offset));

		context.SetResult<bool>(value[bit]);
	}
}

template<int offset, unsigned char bit>
static void writeVehicleMemoryBit(fx::ScriptContext& context)
{
	if (context.GetArgumentCount() < 2)
	{
		trace("Insufficient arguments count, 2 expected");
		return;
	}

	if (fwEntity* vehicle = getAndCheckVehicle(context))
	{
		std::bitset<sizeof(int)> value(readValue<int>(vehicle, offset));
		value[bit] = context.GetArgument<bool>(1);
		writeValue<unsigned char>(vehicle, offset, static_cast<unsigned char>(value.to_ulong()));
	}
}

template<typename T, int offset>
static void writeVehicleMemory(fx::ScriptContext& context)
{
	if (context.GetArgumentCount() < 2)
	{
		trace("Insufficient arguments count, 2 expected");
		return;
	}

	if (fwEntity* vehicle = getAndCheckVehicle(context))
	{
		writeValue<T>(vehicle, offset, context.GetArgument<T>(1));
	}
}

// 1103 specific
const int HeliMainRotorHealthOffset = 0x184C;
const int HeliTailRotorHealthOffset = 0x1850;
const int HeliEngineHealthOffset = 0x1854;
const int FuelLevelOffset = 0x7B8;
const int OilLevelOffset = 0x7BC;
const int GravityOffset = 0xB8C;
const int IsEngineStartingOffset = 0x89A;
const int DashSpeedOffset = 0xA00;
const int HeliBladesSpeedOffset = 0x1840;
const int AccelerationOffset = 0x834;
const int CurrentRPMOffset = 0x824;
const int HighGearOffset = 0x7F6;
const int CurrentGearOffset = 0x7F2;
const int NextGearOffset = 0x7F0;
const int RpmOffset = 0x824;
const int ClutchOffset = 0x830;
const int TurboBoostOffset = 0x848;
const int ThrottleInputOffset = 0x90C;
const int BrakeInputOffset = 0x910;
const int HandbrakeOffset = 0x914;
const int EngineTempOffset = 0x9BC;
const int NumWheelsOffset = 0xB28;
const int WheelsPtrOffset = 0xB20;

const int SteeringAngleOffset = 0x904;
const int SteeringScaleOffset = 0x8FC;
const int IsAlarmSetOffset = 0x9F4;
const int AlarmTimeLeftOffset = 0x9F4;
const int IsWantedOffset = 0x9A4;
const int ProvidesCoverOffset = 0x894;
const int PreviouslyOwnedByPlayerOffset = 0x89C;
const int NeedsToBeHotwiredOffset = 0x89C;
const int IsInteriorLightOnOffset = 0x899;
const int LodMultiplierOffset = 0x1274;
const int IsLeftHeadLightBrokenOffset = 0x7CC;
const int IsRightHeadLightBrokenOffset = 0x7CC;
const int EnginePowerMultiplierOffset = 0xA28;
const int CanWheelsBreakOffset = 0x893;
const int BlinkerState = 0x899;

// Wheel class
const int WheelTyreRadiusOffset = 0x110;
const int WheelRimRadiusOffset = 0x114;
const int WheelTyreWidthOffset = 0x118;
const int WheelRotationSpeedOffset = 0x168;
const int WheelHealthOffset = 0x1E0;

static HookFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_FUEL_LEVEL", readVehicleMemory<float, FuelLevelOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_FUEL_LEVEL", writeVehicleMemory<float, FuelLevelOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_OIL_LEVEL", readVehicleMemory<float, OilLevelOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_OIL_LEVEL", writeVehicleMemory<float, OilLevelOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_GRAVITY_AMOUNT", readVehicleMemory<float, GravityOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_GRAVITY_AMOUNT", writeVehicleMemory<float, GravityOffset>);

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_ENGINE_STARTING", readVehicleMemoryBit<IsEngineStartingOffset, 5>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_SPEED", readVehicleMemory<float, DashSpeedOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ACCELERATION", readVehicleMemory<float, AccelerationOffset>);

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_GRAVITY", readVehicleMemory<float, AccelerationOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CURRENT_RPM", readVehicleMemory<float, CurrentRPMOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_CURRENT_RPM", writeVehicleMemory<float, CurrentRPMOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HIGH_GEAR", readVehicleMemory<unsigned char, HighGearOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_HIGH_GEAR", writeVehicleMemory<unsigned char, HighGearOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CURRENT_GEAR", readVehicleMemory<unsigned char, CurrentGearOffset, int>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_CURRENT_GEAR", writeVehicleMemory<unsigned char, CurrentGearOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NEXT_GEAR", readVehicleMemory<unsigned char, NextGearOffset, int>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_NEXT_GEAR", writeVehicleMemory<unsigned char, NextGearOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CLUTCH", readVehicleMemory<float, ClutchOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_CLUTCH", writeVehicleMemory<float, ClutchOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_TURBO_PRESSURE", readVehicleMemory<float, TurboBoostOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_TURBO_PRESSURE", writeVehicleMemory<float, TurboBoostOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HANDBRAKE",	readVehicleMemory<bool, HandbrakeOffset>); // just a getter

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ENGINE_TEMPERATURE", readVehicleMemory<float, EngineTempOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_ENGINE_TEMPERATURE", writeVehicleMemory<float, EngineTempOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NUMBER_OF_WHEELS", readVehicleMemory<unsigned char, NumWheelsOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_SPEED", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 2)
		{
			context.SetResult<float>(0.0f);
			return;
		}

		unsigned char wheelIndex = context.GetArgument<int>(1);

		if (fwEntity* vehicle = getAndCheckVehicle(context))
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


	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_HEALTH", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 2)
		{
			context.SetResult<float>(0.0f);
			return;
		}

		unsigned char wheelIndex = context.GetArgument<int>(1);

		if (fwEntity* vehicle = getAndCheckVehicle(context))
		{
			unsigned char numWheels = readValue<unsigned char>(vehicle, NumWheelsOffset);
			if (wheelIndex >= numWheels) {
				context.SetResult<float>(0.0f);
				return;
			}
			auto wheelsAddress = readValue<uint64_t>(vehicle, WheelsPtrOffset);
			auto wheelAddr = *reinterpret_cast<uint64_t *>(wheelsAddress + 0x008 * wheelIndex);
			context.SetResult<float>(*reinterpret_cast<float *>(wheelAddr + WheelHealthOffset));
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_HEALTH", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 3)
		{
			return;
		}

		unsigned char wheelIndex = context.GetArgument<int>(1);

		if (fwEntity* vehicle = getAndCheckVehicle(context))
		{
			unsigned char numWheels = readValue<unsigned char>(vehicle, NumWheelsOffset);
			if (wheelIndex >= numWheels) {
				return;
			}
			auto wheelsAddress = readValue<uint64_t>(vehicle, WheelsPtrOffset);
			auto wheelAddr = *reinterpret_cast<uint64_t *>(wheelsAddress + 0x008 * wheelIndex);
			*reinterpret_cast<float *>(wheelAddr + WheelHealthOffset) = context.GetArgument<float>(2);;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_STEERING_ANGLE", [](fx::ScriptContext& context)
	{
		if (fwEntity* vehicle = getAndCheckVehicle(context))
		{
			float angle = readValue<float>(vehicle, SteeringAngleOffset);

			context.SetResult<float>(angle * (180.0f / 3.14159265358979323846));
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_STEERING_ANGLE", [](fx::ScriptContext& context)
	{
		float angle = context.GetArgument<float>(1);

		if (fwEntity* vehicle = getAndCheckVehicle(context))
		{
			writeValue<float>(vehicle, SteeringAngleOffset, angle / (180.0f / 3.14159265358979323846));
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_STEERING_SCALE", readVehicleMemory<float, SteeringScaleOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_STEERING_SCALE", writeVehicleMemory<float, SteeringScaleOffset>);

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_ALARM_SET", [](fx::ScriptContext& context)
	{
		if (fwEntity* vehicle = getAndCheckVehicle(context))
		{
			unsigned short alarmTime = readValue<unsigned short>(vehicle, IsAlarmSetOffset);

			context.SetResult<bool>(alarmTime == std::numeric_limits<unsigned short>::max());
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ALARM_TIME_LEFT", [](fx::ScriptContext& context)
	{
		if (fwEntity* vehicle = getAndCheckVehicle(context))
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
		if (fwEntity* vehicle = getAndCheckVehicle(context))
		{
			unsigned short alarmTime = context.GetArgument<unsigned short>(1);

			if (alarmTime != std::numeric_limits<unsigned short>::max())
			{
				writeValue<unsigned short>(vehicle, AlarmTimeLeftOffset, alarmTime);
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_WANTED", readVehicleMemoryBit<IsWantedOffset, 3>);

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_PREVIOUSLY_OWNED_BY_PLAYER", readVehicleMemoryBit<PreviouslyOwnedByPlayerOffset, 1>);

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_NEEDS_TO_BE_HOTWIRED", readVehicleMemoryBit<NeedsToBeHotwiredOffset, 2>);

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_INTERIOR_LIGHT_ON", readVehicleMemoryBit<IsInteriorLightOnOffset, 6>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_INDICATOR_LIGHTS", readVehicleMemory<unsigned char, BlinkerState>);
});
