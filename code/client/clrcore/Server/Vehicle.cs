using CitizenFX.Core.Native;

namespace CitizenFX.Core
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

	public enum VehicleSeat
	{
		None = -3,
		Any,
		Driver,
		Passenger,
		LeftFront = -1,
		RightFront,
		LeftRear,
		RightRear,
		ExtraSeat1,
		ExtraSeat2,
		ExtraSeat3,
		ExtraSeat4,
		ExtraSeat5,
		ExtraSeat6,
		ExtraSeat7,
		ExtraSeat8,
		ExtraSeat9,
		ExtraSeat10,
		ExtraSeat11,
		ExtraSeat12
	}

	public sealed class Vehicle : Entity
	{
		#region Fields
		VehicleDoorCollection _doors;
		#endregion

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
			set
			{
				API.SetVehicleBodyHealth(Handle, value);
			}
		}

		/// <summary>
		/// Gets this <see cref="Vehicle"/> engine health.
		/// </summary>
		public float EngineHealth
		{
			get
			{
				return API.GetVehicleEngineHealth(Handle);
			}
		}

		/// <summary>
		/// Gets this <see cref="Vehicle"/> petrol tank health.
		/// </summary>
		public float PetrolTankHealth
		{
			get
			{
				return API.GetVehiclePetrolTankHealth(Handle);
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Vehicle"/>s engine is running.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Vehicle"/>s engine is running; otherwise, <c>false</c>.
		/// </value>
		public bool IsEngineRunning
		{
			get
			{
				return API.GetIsVehicleEngineRunning(Handle);
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Vehicle"/>s engine is currently starting.
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
		/// Sets a value indicating whether this <see cref="Vehicle"/> has an alarm set.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has an alarm set; otherwise, <c>false</c>.
		/// </value>
		public bool IsAlarmSet
		{
			set
			{
				API.SetVehicleAlarm(Handle, value);
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Vehicle"/> has its siren turned on.
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
		/// Gets a value indicating whether this <see cref="Vehicle"/> has its lights on.
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
		/// Gets a value indicating whether this <see cref="Vehicle"/> has its high beams on.
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
		/// Gets a value indicating whether the Handbrake on this <see cref="Vehicle"/> is forced on.
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

		/// <summary>
		/// Gets or sets a value indicating whether the doors on this <see cref="Vehicle"/> are locked.
		/// </summary>
		public VehicleLockStatus LockStatus
		{
			get
			{
				return (VehicleLockStatus)API.GetVehicleDoorLockStatus(Handle);
			}
			set
			{
				API.SetVehicleDoorsLocked(Handle, (int)value);
			}
		}

		public VehicleDoorCollection Doors
		{
			get
			{
				if (_doors == null)
				{
					_doors = new VehicleDoorCollection(this);
				}

				return _doors;
			}
		}

		/// <summary>
		/// Gets a value indicating whether a specific extra on this <see cref="Vehicle"/> is on.
		/// </summary>
		/// <value>
		///   <c>true</c> if the specific extra on this <see cref="Vehicle"/> is on; otherwise, <c>false</c>.
		/// </value>
		public bool IsExtraOn(int extra)
		{
			return API.IsVehicleExtraTurnedOn(Handle, extra);
		}

		/// <summary>
		/// Cleans this <see cref="Vehicle"/>
		/// </summary>
		public void Wash()
		{
			DirtLevel = 0f;
		}

		/// <summary>
		/// Gets or sets the dirt level of this <see cref="Vehicle"/> as a <see cref="float"/>.
		/// Dirt level should be less than 15.0f
		/// </summary>
		public float DirtLevel
		{
			get
			{
				return API.GetVehicleDirtLevel(Handle);
			}
			set
			{
				API.SetVehicleDirtLevel(Handle, value);
			}
		}

		/// <summary>
		/// Creates a <see cref="Ped"/> inside this <see cref="Vehicle"/> in the provided <see cref="VehicleSeat"/>; pass model name as a <see cref="string"/>
		/// </summary>
		/// <param name="seat">The <see cref="VehicleSeat"/> to place the <see cref="Ped"/> in</param>
		/// <param name="model">The vehicle mode name as a <see cref="string"/></param>
		/// <returns></returns>
		public Ped CreatePedOnSeat(VehicleSeat seat, string model)
		{
			return new Ped(API.CreatePedInsideVehicle(Handle, 26, (uint)API.GetHashKey(model), (int)seat, true, true));
		}

		/// <summary>
		/// Determines whether this <see cref="Vehicle"/> exists.
		/// </summary>
		/// <returns><c>true</c> if this <see cref="Vehicle"/> exists; otherwise, <c>false</c></returns>
		public new bool Exists()
		{
			return base.Exists() && API.GetEntityType(Handle) == 2;
		}

		/// <summary>
		/// Determines whether the <see cref="Vehicle"/> exists.
		/// </summary>
		/// <param name="vehicle">The <see cref="Vehicle"/> to check.</param>
		/// <returns><c>true</c> if the <see cref="Vehicle"/> exists; otherwise, <c>false</c></returns>
		public static bool Exists(Vehicle vehicle)
		{
			return !ReferenceEquals(vehicle, null) && vehicle.Exists();
		}
	}
}