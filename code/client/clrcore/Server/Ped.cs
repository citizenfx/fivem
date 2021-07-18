using System;
using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
	public enum RagdollType
	{
		Normal = 0,
		StiffLegs = 1,
		NarrowLegs = 2,
		WideLegs = 3,
	}

	public sealed class Ped : Entity
	{
		#region Fields
		Tasks _tasks;
		WeaponCollection _weapons;
		Style _style;
		#endregion

		public Ped(int handle) : base(handle)
		{
		}

		/// <summary>
		/// Gets or sets how much Armor this <see cref="Ped"/> is wearing.
		/// </summary>
		/// <remarks>if you need to get or set the value strictly, use <see cref="ArmorFloat"/> instead.</remarks>
		public int Armor
		{
			get
			{
				return API.GetPedArmour(Handle);
			}
			set
			{
				API.SetPedArmour(Handle, value);
			}
		}

		/// <summary>
		/// Opens a list of <see cref="Tasks"/> that this <see cref="Ped"/> can carry out.
		/// </summary>
		public Tasks Task
		{
			get
			{
				if (ReferenceEquals(_tasks, null))
				{
					_tasks = new Tasks(this);
				}
				return _tasks;
			}
		}

		/// <summary>
		/// Gets a collection of all this <see cref="Ped"/>s <see cref="Weapon"/>s.
		/// </summary>
		public WeaponCollection Weapons
		{
			get
			{
				if (ReferenceEquals(_weapons, null))
				{
					_weapons = new WeaponCollection(this);
				}
				return _weapons;
			}
		}

		/// <summary>
		/// Opens a list of clothing and prop configurations that this <see cref="Ped"/> can wear.
		/// </summary>
		public Style Style
		{
			get
			{
				if (ReferenceEquals(_style, null))
				{
					_style = new Style(this);
				}
				return _style;
			}
		}

		/// <summary>
		/// Gets the last <see cref="Vehicle"/> this <see cref="Ped"/> used.
		/// </summary>
		/// <remarks>returns <c>null</c> if the last vehicle doesn't exist.</remarks>
		public Vehicle LastVehicle
		{
			get
			{
				Vehicle veh = new Vehicle(API.GetVehiclePedIsIn(Handle, true));
				return veh.Exists() ? veh : null;
			}
		}

		/// <summary>
		/// Gets the current <see cref="Vehicle"/> this <see cref="Ped"/> is using.
		/// </summary>
		/// <remarks>returns <c>null</c> if this <see cref="Ped"/> isn't in a <see cref="Vehicle"/>.</remarks>
		public Vehicle CurrentVehicle
		{
			get
			{
				Vehicle veh = new Vehicle(API.GetVehiclePedIsIn(Handle, false));
				return veh.Exists() ? veh : null;
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Ped"/> is <see cref="Player"/>.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Ped"/> is a <see cref="Player"/>; otherwise, <c>false</c>.
		/// </value>
		public bool IsPlayer
		{
			get
			{
				return API.IsPedAPlayer(Handle);
			}
		}

		/// <summary>
		/// Sets if this <see cref="Ped"/> can ragdoll or not
		/// </summary>
		public bool CanRagdoll
		{
			set
			{
				API.SetPedCanRagdoll(Handle, value);
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Ped"/> is in any <see cref="Vehicle"/>.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Ped"/> is in any <see cref="Vehicle"/>; otherwise, <c>false</c>.
		/// </value>
		public bool IsInVehicle()
		{
			//TODO: Replace with IsPedInAnyVehicle if it becomes available server side
			return API.GetVehiclePedIsIn(Handle, false) != 0;
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Ped"/> is in a specific <see cref="Vehicle"/>.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Ped"/> is in a specific <see cref="Vehicle"/>; otherwise, <c>false</c>.
		/// </value>
		public bool IsInVehicle(Vehicle vehicle)
		{
			//TODO: Replace with IsPedInVehicle if it becomes available server side
			return API.GetVehiclePedIsIn(Handle, false) == vehicle.Handle;
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Ped"/> is sitting in any <see cref="Vehicle"/>.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Ped"/> is sitting in any <see cref="Vehicle"/>; otherwise, <c>false</c>.
		/// </value>
		public bool IsSittingInVehicle()
		{
			//TODO: Replace with IsPedSittingInAnyVehicle if it becomes available server side
			return IsInVehicle();
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Ped"/> is sitting in a specific <see cref="Vehicle"/>.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Ped"/> is sitting in a specific <see cref="Vehicle"/>; otherwise, <c>false</c>.
		/// </value>
		public bool IsSittingInVehicle(Vehicle vehicle)
		{
			//TODO: Replace with IsPedSittingInVehicle if it becomes available server side
			return IsInVehicle(vehicle);
		}

		/// <summary>
		/// Places a <see cref="Ped"/> into a specific <see cref="Vehicle"/>
		/// </summary>
		/// <param name="vehicle">The <see cref="Vehicle"/> to place the <see cref="Ped"/> in</param>
		/// <param name="seat">The <see cref="VehicleSeat"/> of the seat</param>
		public void SetIntoVehicle(Vehicle vehicle, VehicleSeat seat)
		{
			API.SetPedIntoVehicle(Handle, vehicle.Handle, (int)seat);
		}

		/// <summary>
		/// Makes a <see cref="Ped"/> ragdoll for the specified time, or -1 indefinitely
		/// </summary>
		/// <param name="duration">Time in ms as <see cref="int"/> to ragdoll</param>
		/// <param name="ragdollType"><see cref="RagdollType"/> to ragdoll</param>
		public void Ragdoll(int duration = -1, RagdollType ragdollType = RagdollType.Normal)
		{
			CanRagdoll = true;
			API.SetPedToRagdoll(Handle, duration, duration, (int)ragdollType, false, false, false);
		}

		/// <summary>
		/// Stops a ped ragdolling
		/// </summary>
		public void CancelRagdoll()
		{
			API.SetPedToRagdoll(Handle, 1, 1, 1, false, false, false);
		}

		/// <summary>
		/// Determines whether this <see cref="Ped"/> exists.
		/// </summary>
		/// <returns><c>true</c> if this <see cref="Ped"/> exists; otherwise, <c>false</c></returns>
		public new bool Exists()
		{
			return base.Exists() && API.GetEntityType(Handle) == 1;
		}

		/// Determines whether the <see cref="Ped"/> exists.
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to check.</param>
		/// <returns><c>true</c> if the <see cref="Ped"/> exists; otherwise, <c>false</c></returns>
		public static bool Exists(Ped ped)
		{
			return !ReferenceEquals(ped, null) && ped.Exists();
		}

		/// <summary>
		/// Creates a new instance of an <see cref="Ped"/> from the given player handle.
		/// </summary>
		/// <param name="handle">The players handle.</param>
		/// <returns>Returns the <see cref="Ped"/> of the player.
		/// Returns <c>null</c> if no <see cref="Ped"/> exists for the specified player</returns>
		public static Ped FromPlayerHandle(string handle)
		{
			int entityHandle = API.GetPlayerPed(handle);

			if (API.GetEntityType(entityHandle) == 1)
			{
				return new Ped(entityHandle);
			}

			return null;
		}
	}
}
