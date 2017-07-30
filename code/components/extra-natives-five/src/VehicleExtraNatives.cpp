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
const int WheelSpeedOffset = 0xA00;
const int HeliBladesSpeedOffset = 0x1840;
const int AccelerationOffset = 0x834;
const int CurrentRPMOffset = 0x824;
const int HighGearOffset = 0x7F6;
const int CurrentGearOffset = 0x7F0;
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

static HookFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_FUEL_LEVEL", readVehicleMemory<float, FuelLevelOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_FUEL_LEVEL", writeVehicleMemory<float, FuelLevelOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_OIL_LEVEL", readVehicleMemory<float, OilLevelOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_OIL_LEVEL", writeVehicleMemory<float, OilLevelOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_GRAVITY", readVehicleMemory<float, GravityOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_GRAVITY", writeVehicleMemory<float, GravityOffset>);

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_ENGINE_STARTING", readVehicleMemoryBit<IsEngineStartingOffset, 5>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_SPEED", readVehicleMemory<float, WheelSpeedOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ACCELERATION", readVehicleMemory<float, AccelerationOffset>);

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_GRAVITY", readVehicleMemory<float, AccelerationOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CURRENT_RPM", readVehicleMemory<float, CurrentRPMOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_CURRENT_RPM", writeVehicleMemory<float, CurrentRPMOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HIGH_GEAR", readVehicleMemory<unsigned char, HighGearOffset>);
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_HIGH_GEAR", writeVehicleMemory<unsigned char, HighGearOffset>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CURRENT_GEAR", readVehicleMemory<unsigned char, CurrentGearOffset, int>);

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_STEERING_ANGLE", [](fx::ScriptContext& context)
	{
		if (fwEntity* vehicle = getAndCheckVehicle(context))
		{
			float angle = readValue<float>(vehicle, SteeringAngleOffset);

			context.SetResult<float>(angle * (180.0f / 3.14159265358979323846));
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
});
