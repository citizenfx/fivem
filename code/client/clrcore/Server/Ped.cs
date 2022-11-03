using System;
using CitizenFX.Core.Native;
using System.Security;

#if MONO_V2
using API = CitizenFX.Server.Native.Natives;
#endif

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
	}
}
