using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
#if !IS_FXSERVER && !IS_RDR3
	public class PlayerList : IEnumerable<Player>
	{
		public const int MaxPlayers = 256;

		public IEnumerator<Player> GetEnumerator()
		{
			for (var i = 0; i < MaxPlayers; i++)
			{
				if (API.NetworkIsPlayerActive(i))
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
#endif
}
