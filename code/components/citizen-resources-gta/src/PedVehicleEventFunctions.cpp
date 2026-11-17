/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#if defined(GTA_FIVE)
#include <EntitySystem.h>
#include <nutsnbolts.h>
#include <scrEngine.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

enum NativeIdentifiers : uint64_t
{
	PLAYER_PED_ID = 0xD80958FC74E988A6,
	GET_VEHICLE_PED_IS_IN = 0x9A9112A0FE9A4713,
	GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS = 0xA7C4F2C6E744A550,
	GET_PED_IN_VEHICLE_SEAT = 0xBB40DD2270B65366,
};

static rage::fwEntity* g_trackedPed;
static int g_trackedVehicle;
static int g_trackedSeat;

// the local ped was removed from a seat, it is checked again on the next frame as it may have been seated again
static bool g_pedVehicleSettlePending;

static constexpr int kUnknownSeat = -2;

static int GetLocalPedHandle()
{
	return NativeInvoke::Invoke<PLAYER_PED_ID, int>();
}

static int FindPedSeat(int vehicle, int ped)
{
	// seat indices range from -1 (driver) to the maximum number of passengers minus one
	int maxPassengers = NativeInvoke::Invoke<GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS, int>(vehicle);

	for (int seat = -1; seat < maxPassengers; seat++)
	{
		if (NativeInvoke::Invoke<GET_PED_IN_VEHICLE_SEAT, int>(vehicle, seat) == ped)
		{
			return seat;
		}
	}

	return kUnknownSeat;
}

static void UpdateLocalPedVehicle(rage::fwEntity* ped)
{
	g_pedVehicleSettlePending = false;

	auto resman = Instance<fx::ResourceManager>::Get();
	auto rec = resman->GetComponent<fx::ResourceEventManagerComponent>();

	int pedHandle = rage::fwScriptGuid::GetGuidFromBase(ped);
	int vehicle = NativeInvoke::Invoke<GET_VEHICLE_PED_IS_IN, int>(pedHandle, false);
	int seat = vehicle ? FindPedSeat(vehicle, pedHandle) : 0;

	// the local ped was replaced (respawn, model change), forget the old one without reporting anything
	if (g_trackedPed != ped)
	{
		g_trackedPed = ped;
		g_trackedVehicle = 0;
		g_trackedSeat = 0;
	}

	int oldVehicle = g_trackedVehicle;
	int oldSeat = g_trackedSeat;

	// compared with what was reported and not with the state before the engine call, so a ped that was removed from its seat and seated again right after does not produce any events
	if (oldVehicle == vehicle && oldSeat == seat)
	{
		return;
	}

	g_trackedVehicle = vehicle;
	g_trackedSeat = seat;

	if (oldVehicle != 0 && oldVehicle == vehicle)
	{
		/*NETEV playerChangedVehicleSeat CLIENT
		/#*
		 * An event that is triggered when the local player moves to another seat of the vehicle it is in.
		 *
		 * @param vehicle - The vehicle the player is in.
		 * @param seat - The seat the player moved to (-1 is the driver).
		 * @param oldSeat - The seat the player moved from (-1 is the driver).
		 #/
		declare function playerChangedVehicleSeat(vehicle: number, seat: number, oldSeat: number): void;
		*/
		rec->QueueEvent2("playerChangedVehicleSeat", {}, vehicle, seat, oldSeat);

		return;
	}

	if (oldVehicle != 0)
	{
		/*NETEV playerLeftVehicle CLIENT
		/#*
		 * An event that is triggered when the local player leaves a vehicle.
		 *
		 * @param vehicle - The vehicle the player left, it may no longer exist if it was deleted while the player was inside.
		 * @param seat - The seat the player was in (-1 is the driver).
		 #/
		declare function playerLeftVehicle(vehicle: number, seat: number): void;
		*/
		rec->QueueEvent2("playerLeftVehicle", {}, oldVehicle, oldSeat);
	}

	if (vehicle != 0)
	{
		/*NETEV playerEnteredVehicle CLIENT
		/#*
		 * An event that is triggered when the local player gets into a vehicle.
		 *
		 * @param vehicle - The vehicle the player entered.
		 * @param seat - The seat the player took (-1 is the driver).
		 #/
		declare function playerEnteredVehicle(vehicle: number, seat: number): void;
		*/
		rec->QueueEvent2("playerEnteredVehicle", {}, vehicle, seat);
	}
}

static InitFunction initFunction([]
{
	OnPedVehicleEvent.Connect([](const PedVehicleEventMetaData& data)
	{
		if (!data.ped || GetCurrentThreadId() != g_mainThreadId)
		{
			return;
		}

		int localPed = GetLocalPedHandle();

		if (!localPed || rage::fwScriptGuid::GetGuidFromBase(data.ped) != localPed)
		{
			return;
		}

		if (data.type == PedVehicleEventMetaData::Type::RemovedFromSeat)
		{
			g_pedVehicleSettlePending = true;
			return;
		}

		UpdateLocalPedVehicle(data.ped);
	});

	OnMainGameFrame.Connect([]
	{
		if (!g_pedVehicleSettlePending)
		{
			return;
		}

		g_pedVehicleSettlePending = false;

		if (int localPed = GetLocalPedHandle())
		{
			if (auto ped = rage::fwScriptGuid::GetBaseFromGuid(localPed))
			{
				UpdateLocalPedVehicle(ped);
			}
		}
	});
});
#endif
