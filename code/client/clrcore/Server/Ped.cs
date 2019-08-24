using System;
using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
	public sealed class Ped : Entity
	{
		public Ped(int handle) : base(handle)
		{
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
		/// <summary>
		/// Gets or sets how much Armor this <see cref="Ped"/> is wearing.
		/// </summary>
		public int Armor
		{
			get
			{
				return API.GetPedArmour(Handle);
			}
		}

		/// <summary>
		/// Returns the hash of the weapon/model/object that killed the <see cref="Ped"/>.
		/// </summary>
		public Hash CauseOfDeath
		{
			get
			{
				return API.GetPedCauseOfDeath(Handle);
			}
		}
	}
}
