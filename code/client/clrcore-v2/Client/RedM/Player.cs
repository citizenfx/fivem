using System;
using CitizenFX.Core;
using INativeValue = CitizenFX.Shared.Player;
using CitizenFX.RedM.Native;

namespace CitizenFX.RedM
{
    public sealed class Player : INativeValue, IEquatable<Player>
    {
        #region Fields
        private Ped m_ped;

        #endregion

        public static Player Local { get; } = new Player(Natives.PlayerId());

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
        public override StateBag State =>  new StateBag($"player:{ServerId}");

        public override Shared.IPed GetCharacter()
        {
	        return Character;
        }

        /// <summary>
        /// Gets the <see cref="Ped"/> this <see cref="Player"/> is controlling.
        /// </summary>
        public Ped Character
        {
            get
            {
                int handle = Natives.GetPlayerPed(Handle);

                if (ReferenceEquals(m_ped, null) || handle != m_ped.Handle)
                {
                    m_ped = new Ped(handle);
                }

                return m_ped;
            }
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
            return left?.Equals(right) ?? right is null;
        }
        public static bool operator !=(Player left, Player right)
        {
            return !(left == right);
        }
    }
}
