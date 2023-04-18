using System;

#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{
	public class AIBlip : PoolObject, IEquatable<AIBlip>
	{
		/// <summary>
		/// Creates a <see cref="AIBlip"/>. This blip will be handled by the game itself and are used for peds (guards, allies, etc). 
		/// </summary>
		/// <param name="handle">Handle of the ped that will have the blip</param>
		/// <param name="sprite">Sprite of the <see cref="AIBlip"/></param>
		public AIBlip(int handle, int sprite) : base(handle)
		{
			API.SetPedHasAiBlip(handle, true);
			API.SetPedAiBlipSprite(handle, sprite);
		}

		/// <summary>
		/// Creates a <see cref="AIBlip"/>. This blip will be handled by the game itself and are used for peds (guards, allies, etc). 
		/// </summary>
		/// <param name="handle">Handle of the ped that will have the blip</param>
		/// <param name="sprite">Sprite of the <see cref="AIBlip"/></param>
		/// <param name="color">Color of the blip (see <see href="https://docs.fivem.net/docs/game-references/blips/#blip-colors">the docs</see> (blip colors))</param>
		public AIBlip(int handle, int sprite, int color) : base(handle)
		{
			API.SetPedHasAiBlipWithColor(handle, true, color);
			API.SetPedAiBlipSprite(handle, sprite);
		}

		/// <summary>
		/// Sets the status of the vision cone of this <see cref="AIBlip"/>
		/// </summary>
		public bool ConeEnabled
		{
			set
			{
				API.SetPedAiBlipHasCone(Handle, value);
			}
		}

		/// <summary>
		/// Sets the status if this <see cref="AIBlip"/> is forced on the map
		/// </summary>
		public bool ForcedOn
		{
			set
			{
				API.SetPedAiBlipForcedOn(Handle, value);
			}
		}

		/// <summary>
		/// Sets the gang id
		/// </summary>
		public int GangId
		{
			set
			{
				API.SetPedAiBlipGangId(Handle, value);
			}
		}

		/// <summary>
		/// Sets the sprite of this <see cref="AIBlip"/>
		/// </summary>
		public int Sprite
		{
			set
			{
				API.SetPedAiBlipSprite(Handle, value);
			}
		}

		/// <summary>
		/// Sets the range of this <see cref="AIBlip"/>
		/// </summary>
		public float Range
		{
			set
			{
				API.SetPedAiBlipNoticeRange(Handle, value);
			}
		}

		/// <summary>
		/// Returns the blip handle of this <see cref="AIBlip"/>
		/// </summary>
		/// <returns>Blip Handle</returns>
		public int GetBlipHandle()
		{
			return API.GetAiBlip(Handle);
		}

		/// <summary>
		/// Returns the blip as a blip object from this <see cref="AIBlip"/>
		/// </summary>
		/// <returns>A <see cref="Blip"/> object</returns>
		public Blip GetBlipObject()
		{
			return new Blip(API.GetAiBlip(Handle));
		}

		/// <summary>
		/// Disables this <see cref="AIBlip"/>
		/// </summary>
		public override void Delete()
		{
			if (API.DoesPedHaveAiBlip(Handle))
			{
				API.SetPedHasAiBlip(Handle, false);
			}
		}

		public override bool Exists()
		{
			return API.DoesPedHaveAiBlip(Handle);
		}

		public static bool Exists(AIBlip aiBlip)
		{
			return !ReferenceEquals(aiBlip, null) && aiBlip.Exists();
		}

		public bool Equals(AIBlip aiBlip)
		{
			return !ReferenceEquals(aiBlip, null) && Handle == aiBlip.Handle;
		}

		public override bool Equals(object obj)
		{
			return !ReferenceEquals(obj, null) && obj is AIBlip blip && Equals(blip);
		}

		public sealed override int GetHashCode()
		{
			return Handle.GetHashCode();
		}

		public static bool operator ==(AIBlip left, AIBlip right)
		{
			return ReferenceEquals(left, null) ? ReferenceEquals(right, null) : left.Equals(right);
		}

		public static bool operator !=(AIBlip left, AIBlip right)
		{
			return !(left == right);
		}
	}
}
