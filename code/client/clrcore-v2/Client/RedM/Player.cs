using System;
using CitizenFX.Core;
using INativeValue = CitizenFX.Shared.Player;
using CitizenFX.RedM.Native;

namespace CitizenFX.RedM
{
	public sealed class Player : INativeValue, IEquatable<Player>
	{
		#region Fields
		Ped _ped;
		
		#endregion
		
		private static Player m_player = new Player(Natives.PlayerId());
		public static Player Local => m_player;
		
		public Player(int handle) : base((ulong)handle)
		{
		}
		
		
		/// <summary>
		/// Gets the <see cref="Player"/>s server id
		/// </summary>
		public int ServerId => Natives.GetPlayerServerId(Handle);

		/// <summary>
		/// Gets the name of this <see cref="Player"/>.
		/// </summary>
		public override string Name => Natives.GetPlayerName(Handle);
		
		/// <summary>
		/// Gets the <see cref="StateBag"/> of this <see cref="Player"/>
		/// </summary>
		public override StateBag State => new StateBag("player:" + ServerId);
		
		/// <summary>
		/// Gets the <see cref="Ped"/> this <see cref="Player"/> is controlling.
		/// </summary>
		public Ped Character
		{
			get
			{
				int handle = Natives.GetPlayerPed(Handle);

				if (ReferenceEquals(_ped, null) || handle != _ped.Handle)
				{
					_ped = new Ped(handle);
				}

				return _ped;
			}
		}
		
		public override Shared.IPed GetCharacter()
		{
			return Character;
		}
		
		public bool Equals(Player player)
		{
			return !ReferenceEquals(player, null) && Handle == player.Handle;
		}
		public override bool Equals(object obj)
		{
			return !ReferenceEquals(obj, null) && obj.GetType() == GetType() && Equals((Entity)obj);
		}

		public override int GetHashCode()
		{
			return Handle.GetHashCode();
		}

		public static bool operator ==(Player left, Player right)
		{
			return ReferenceEquals(left, null) ? ReferenceEquals(right, null) : left.Equals(right);
		}
		public static bool operator !=(Player left, Player right)
		{
			return !(left == right);
		}
	}
}
