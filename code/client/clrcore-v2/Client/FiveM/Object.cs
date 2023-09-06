using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
{
	public sealed class Object : Entity, Shared.IObject
	{
		public Object(int handle) : base(handle)
		{
		}

		/// <summary>
		/// Determines whether this game object exists.
		/// </summary>
		/// <returns><c>true</c> if this game object exists; otherwise, <c>false</c></returns>
		public new bool Exists() => base.Exists() && API.GetEntityType(Handle) == 3;

		/// <summary>
		/// Determines whether the game object exists.
		/// </summary>
		/// <param name="prop">The game object to check.</param>
		/// <returns><c>true</c> if the game object exists; otherwise, <c>false</c></returns>
		public static bool Exists(Object prop) => !ReferenceEquals(prop, null) && prop.Exists();
	}
}
