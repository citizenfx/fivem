using System;
using CitizenFX.Core;
using CitizenFX.Shared;
using CitizenFX.RedM.Native;

namespace CitizenFX.RedM
{
	public sealed class Ped : Entity, Shared.IPed
	{
		public Ped(int handle) : base(handle)
		{
		}

		/// <summary>
		/// Returns the Ped or null if <paramref name="handle"/> is 0
		/// </summary>
		public static Ped GetPedOrNull(int handle) {
			return handle == 0 ? null : new Ped(handle);
		}

		/// <summary>
		/// Returns true if the <see cref="Ped"/> is currently walking
		/// </summary>
		public bool IsWalking => Natives.IsPedWalking(Handle);

		/// <summary>
		/// Returns true if the <see cref="Ped"/> is a player.
		/// </summary>
		public bool IsPlayer => Natives.IsPedAPlayer(Handle);

		/// <summary>
		/// Returns true if the <see cref="Ped"/> is in any vehicle
		/// </summary>
		public bool IsInAnyVehicle => Natives.IsPedInAnyVehicle(Handle, true);

		public Vehicle CurrentVehicle => Vehicle.GetVehicleOrNull(Natives.GetVehiclePedIsIn(Handle, false));

		// Doesn't seem too work for draft vehicles
		// public Vehicle LastVehicle => new Vehicle(Natives.GetVehiclePedIsIn(Handle, true));
	}
}
