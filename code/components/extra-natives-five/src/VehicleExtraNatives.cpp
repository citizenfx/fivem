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
#include <GameInit.h>

#include <limits>
#include <bitset>
#include <unordered_set>

static std::unordered_set<fwEntity*> g_skipRepairVehicles{};

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

// 1290 now
const int HeliMainRotorHealthOffset = 0x19F0;
const int HeliTailRotorHealthOffset = 0x19F4;
const int HeliEngineHealthOffset = 0x19F8;
const int FuelLevelOffset = 0x7F4;
const int OilLevelOffset = 0x7E8;
const int GravityOffset = 0xBCC;
const int IsEngineStartingOffset = 0x8DA;
const int DashSpeedOffset = 0xA30;
const int HeliBladesSpeedOffset = 0x19E4;
const int AccelerationOffset = 0x874;
const int CurrentRPMOffset = 0x864;
const int HighGearOffset = 0x836;
const int CurrentGearOffset = 0x832;
const int NextGearOffset = 0x830;
const int RpmOffset = 0x874;
const int ClutchOffset = 0x870;
const int TurboBoostOffset = 0x888;
const int ThrottleInputOffset = 0x94C;
const int BrakeInputOffset = 0x950;
const int HandbrakeOffset = 0x954;
const int EngineTempOffset = 0x9FC;
const int NumWheelsOffset = 0xB68;
const int WheelsPtrOffset = 0xB60;

const int SteeringAngleOffset = 0x944;
const int SteeringScaleOffset = 0x93C;
const int IsAlarmSetOffset = 0xA38;
const int AlarmTimeLeftOffset = 0xA38;
const int IsWantedOffset = 0x8E4;
const int ProvidesCoverOffset = 0x8D4;
const int PreviouslyOwnedByPlayerOffset = 0x8DC;
const int NeedsToBeHotwiredOffset = 0x8DC;
const int IsInteriorLightOnOffset = 0x8D9;
const int LodMultiplierOffset = 0x12C8;
const int IsLeftHeadLightBrokenOffset = 0x80C;
const int IsRightHeadLightBrokenOffset = 0x80C;
const int EnginePowerMultiplierOffset = 0xA70;
const int CanWheelsBreakOffset = 0x8D3;
const int BlinkerState = 0x8D9;

// Wheel class
const int WheelXOffsetOffset = 0x030;
const int WheelTyreRadiusOffset = 0x110;
const int WheelRimRadiusOffset = 0x114;
const int WheelTyreWidthOffset = 0x118;
const int WheelRotationSpeedOffset = 0x168;
const int WheelHealthOffset = 0x1E0;
const int WheelXRotOffset = 0x008;
const int WheelInvXRotOffset = 0x010;

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

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HANDBRAKE", readVehicleMemory<bool, HandbrakeOffset>); // just a getter

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

			if (fwEntity* vehicle = getAndCheckVehicle(context))
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

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_XROT", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		context.SetResult<float>(*reinterpret_cast<float *>(wheelAddr + WheelXRotOffset));
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_WHEEL_XROT", makeWheelFunction([](fx::ScriptContext& context, fwEntity* vehicle, uintptr_t wheelAddr)
	{
		*reinterpret_cast<float *>(wheelAddr + WheelXRotOffset) = context.GetArgument<float>(2);
		*reinterpret_cast<float *>(wheelAddr + WheelInvXRotOffset) = -(context.GetArgument<float>(2));
	}));

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

	static struct : jitasm::Frontend
	{
		static void CleanupVehicle(fwEntity* VehPointer)
		{
			g_skipRepairVehicles.erase(VehPointer);
		}
		virtual void InternalMain() override
		{
			//overwritten assembly code
			mov(rbx, rcx);
			mov(qword_ptr[rcx], rax);

			//save registers
			push(rax);
			push(rcx);
			sub(rsp, 0x8);

			//actual cleanup function call
			mov(rax, (uintptr_t)CleanupVehicle);
			sub(rsp, 0x20);
			call(rax);
			add(rsp, 0x20);

			//restore registers
			add(rsp, 0x8);
			pop(rcx);
			pop(rax);
			ret();
		}
	} vehicleDeconstructorHook;
	auto vehicleDeconstructor = hook::get_pattern("48 8B 43 20 48 8B 88 B0 00 00 00 75 09 66 FF 89",-0x1D);
	hook::nop(vehicleDeconstructor, 0x6);
	hook::call_reg<2>(vehicleDeconstructor, vehicleDeconstructorHook.GetCode());

	OnKillNetworkDone.Connect([]() {
		g_skipRepairVehicles.clear();
	});


	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_AUTO_REPAIR_DISABLED", [](fx::ScriptContext& context) {
		auto vehHandle = context.GetArgument<int>(0);
		auto shouldDisable = context.GetArgument<bool>(1);
		fwEntity *entity = getScriptEntity(vehHandle);
		if (shouldDisable) {
			g_skipRepairVehicles.insert(entity);
		}
		else
		{
			g_skipRepairVehicles.erase(entity);
		}
	});
});
