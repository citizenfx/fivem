#include <StdInc.h>
#include <Hooking.h>
#include <CrossBuildRuntime.h>

//
// For unknown reasons R* doesn't allow "CTaskAimGunVehicleDriveBy" to be replicated locally
// when a remote player (who's running the task) is using free camera (see "CPlayerCameraDataNode").
// Players that have "First Person Vehicle Hood" camera setting enabled, actually counts as using
// free camera mode when first person camera is used while sitting in a vehicle.
// This check might've been done intentionally to avoid some in-game issues, however
// we haven't found any. And this logic leads to the "silent" shooting bug which can be abused.
// So we patch this code path to allow this task to be replicated if a remote player is sitting in
// one of several vehicle types that support "drive by" shooting.
//

enum eVehicleType : uint32_t
{
	VEHICLE_TYPE_CAR = 0,
	VEHICLE_TYPE_PLANE,
	VEHICLE_TYPE_TRAILER,
	VEHICLE_TYPE_QUADBIKE,
	VEHICLE_TYPE_DRAFT,
	VEHICLE_TYPE_SUBMARINECAR,
	VEHICLE_TYPE_AMPHIBIOUS_AUTOMOBILE,
	VEHICLE_TYPE_AMPHIBIOUS_QUADBIKE,
	VEHICLE_TYPE_HELI,
	VEHICLE_TYPE_BLIMP,
	VEHICLE_TYPE_AUTOGYRO,
	VEHICLE_TYPE_BIKE,
	VEHICLE_TYPE_BICYCLE,
	VEHICLE_TYPE_BOAT,
	VEHICLE_TYPE_TRAIN,
	VEHICLE_TYPE_SUBMARINE,
};

static int PedCurrentVehicleOffset;
static int VehicleTypeOffset;

static char (*g_origShouldPedSkipDriveByTaskReplication)(char*);

static char ShouldPedSkipDriveByTaskReplication(char* ped)
{
	auto result = g_origShouldPedSkipDriveByTaskReplication(ped);

	// If we should skip replication, let's check for our conditions now.
	if (ped && result)
	{
		auto currentVehicle = *reinterpret_cast<uint64_t*>(ped + PedCurrentVehicleOffset);

		if (currentVehicle)
		{
			auto vehicleType = *reinterpret_cast<eVehicleType*>(currentVehicle + VehicleTypeOffset);

			switch (vehicleType)
			{
				case VEHICLE_TYPE_CAR:
				case VEHICLE_TYPE_PLANE:
				case VEHICLE_TYPE_SUBMARINECAR:
				case VEHICLE_TYPE_AMPHIBIOUS_AUTOMOBILE:
				case VEHICLE_TYPE_HELI:
				case VEHICLE_TYPE_BLIMP:
				case VEHICLE_TYPE_BOAT:
					return false;
			}
		}
	}

	return result;
}

static HookFunction hookFunction([]()
{
	// Stolen from "GamerTagNatives.cpp"
	if (xbr::IsGameBuild<1604>())
	{
		PedCurrentVehicleOffset = *hook::get_pattern<int>("4C 89 65 88 8B 8B", 13);
	}
	else
	{
		PedCurrentVehicleOffset = *hook::get_pattern<int>("48 8B B0 ? ? ? ? 48 89 45 B0", 3);
	}

	// Stolen from "VehicleExtraNatives.cpp"
	VehicleTypeOffset = *hook::get_pattern<int>("41 83 BF ? ? ? ? 0B 74", 3);

	auto location = hook::get_pattern<char>("48 8B CE E8 ? ? ? ? 40 0F B6 FF 84 C0", 3);
	hook::set_call(&g_origShouldPedSkipDriveByTaskReplication, location);
	hook::call(location, ShouldPedSkipDriveByTaskReplication);
});
