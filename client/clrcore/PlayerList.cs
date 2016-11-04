using System;
using System.Collections;
using System.Collections.Generic;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
    public class PlayerList : IEnumerable<Player>
    {
        public const int MaxPlayers = 32;

        public IEnumerator<Player> GetEnumerator()
        {
            for (int i = 0; i < MaxPlayers; i++)
            {
                if (Function.Call<bool>(Hash.NETWORK_IS_PLAYER_ACTIVE, i))
                {
                    yield return new Player(i);
                }
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
        
        public Player this[int netId]
        {
            get
            {
                foreach (var player in this)
                {
                    if (player.ServerId == netId)
                    {
                        return player;
                    }
                }

                return null;
            }
        }

        public Player this[string name]
        {
            get
            {
                foreach (var player in this)
                {
                    if (player.Name.Equals(name, StringComparison.InvariantCultureIgnoreCase))
                    {
                        return player;
                    }
                }

                return null;
            }
        }
    }
}
