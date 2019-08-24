using System;
using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
	public sealed class Vehicle : Entity
	{
		public enum VehicleLockStatus
		{
			None,
			Unlocked,
			Locked,
			LockedForPlayer,
			StickPlayerInside,
			CanBeBrokenInto = 7,
			CanBeBrokenIntoPersist,
			CannotBeTriedToEnter = 10
		}

		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/>s body health.
		/// </summary>
		public Vehicle(int handle) : base(handle)
		{
		}
		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/>s body health.
		/// </summary>
		public float BodyHealth
		{
			get
			{
				return API.GetVehicleBodyHealth(Handle);
			}
		}
		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/> engine health.
		/// </summary>
		public float EngineHealth
		{
			get
			{
				return API.GetVehicleEngineHealth(Handle);
			}
		}
		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/> petrol tank health.
		/// </summary>
		public float PetrolTankHealth
		{
			get
			{
				return API.GetVehiclePetrolTankHealth(Handle);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/>s engine is running.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Vehicle"/>s engine is running; otherwise, <c>false</c>.
		/// </value>
		public bool IsEngineRunning
		{
			get => API.GetIsVehicleEngineRunning(Handle);
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/>s engine is currently starting.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Vehicle"/>s engine is starting; otherwise, <c>false</c>.
		/// </value>
		public bool IsEngineStarting
		{
			get
			{
				return API.IsVehicleEngineStarting(Handle);
			}
		}
		/// <summary>
		/// Sets this <see cref="Vehicle"/>s radio station.
		/// </summary>
		public int RadioStation
		{
			get
			{
				return API.GetVehicleRadioStationIndex(Handle);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> has its siren turned on.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has its siren turned on; otherwise, <c>false</c>.
		/// </value>
		public bool IsSirenActive
		{
			get
			{
				return API.IsVehicleSirenOn(Handle);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> has its lights on.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has its lights on; otherwise, <c>false</c>.
		/// </value>
		public bool AreLightsOn
		{
			get
			{
				bool lightState1 = false, lightState2 = false;

				API.GetVehicleLightsState(Handle, ref lightState1, ref lightState2);

				return lightState1;
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> has its high beams on.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has its high beams on; otherwise, <c>false</c>.
		/// </value>
		public bool AreHighBeamsOn
		{
			get
			{
				bool lightState1 = false, lightState2 = false;

				API.GetVehicleLightsState(Handle, ref lightState1, ref lightState2);

				return lightState2;
			}
		}
		/// <summary>
		/// Sets a value indicating whether the Handbrake on this <see cref="Vehicle"/> is forced on.
		/// </summary>
		/// <value>
		///   <c>true</c> if the Handbrake on this <see cref="Vehicle"/> is forced on; otherwise, <c>false</c>.
		/// </value>
		public bool IsHandbrakeForcedOn
		{
			get
			{
				return API.GetVehicleHandbrake(Handle);
			}

		}
		public VehicleLockStatus LockStatus
		{
			get
			{
				return (VehicleLockStatus)API.GetVehicleDoorLockStatus(Handle);
			}
		}
	}
}
