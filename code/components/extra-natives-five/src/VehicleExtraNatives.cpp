/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <ScriptEngine.h>
#include <atArray.h>

#include <Hooking.h>

#include <HandlingLoader.h>

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

static hook::cdecl_stub<char*(int handle)> getScriptEntity([]()
{
	return hook::pattern("44 8B C1 49 8B 41 08 41 C1 F8 08 41 38 0C 00").count(1).get(0).get<void>(-12);
});

static char* getVehiclePtrFromContext(fx::ScriptContext& context)
{
	return getScriptEntity(context.GetArgument<int>(0));
}

inline static float readFloat(char* ptr, int offset)
{
	return *(float*)(ptr + offset);
}

inline static float writeFloat(char* ptr, int offset, float value)
{
	*(float*)(ptr + offset) = value;
}

static void VehicleGetFuelLevel(fx::ScriptContext& context)
{
	auto vehiclePtr = getVehiclePtrFromContext(context);

	context.SetResult<float>(readFloat(vehiclePtr, FuelLevelOffset));
}

static void VehicleSetFuelLevel(fx::ScriptContext& context)
{
	auto vehiclePtr = getVehiclePtrFromContext(context);

	writeFloat(vehiclePtr, FuelLevelOffset, context.GetArgument<float>(1));
}

static HookFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("VEHICLE_GET_FUEL_LEVEL", VehicleGetFuelLevel);
	fx::ScriptEngine::RegisterNativeHandler("VEHICLE_SET_FUEL_LEVEL", VehicleSetFuelLevel);
});