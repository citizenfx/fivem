using System;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	public sealed class Prop : Entity
	{
		public Prop(int handle) : base(handle)
		{
		}

		/// <summary>
		/// Determines whether this <see cref="Prop"/> exists.
		/// </summary>
		/// <returns><c>true</c> if this <see cref="Prop"/> exists; otherwise, <c>false</c></returns>
		public new bool Exists()
		{
			return Function.Call<int>(Hash.GET_ENTITY_TYPE, Handle) == 3;
		}
		/// <summary>
		/// Determines whether the <see cref="Prop"/> exists.
		/// </summary>
		/// <param name="prop">The <see cref="Prop"/> to check.</param>
		/// <returns><c>true</c> if the <see cref="Prop"/> exists; otherwise, <c>false</c></returns>
		public static bool Exists(Prop prop)
		{
			return !ReferenceEquals(prop, null) && prop.Exists();
		}
	}
}
