using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
    public class PlayerList : IEnumerable<Player>
    {
        public const int MaxPlayers = 32;

        public IEnumerator<Player> GetEnumerator()
        {
            for (var i = 0; i < MaxPlayers; i++)
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
        
        public Player this[int netId] => this.FirstOrDefault(player => player.ServerId == netId);

	    public Player this[string name] => this.FirstOrDefault(player => player.Name.Equals(name, StringComparison.InvariantCultureIgnoreCase));
    }
}
