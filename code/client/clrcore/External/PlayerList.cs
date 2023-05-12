using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

#if MONO_V2
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{
#if !IS_FXSERVER && !IS_RDR3 && !GTA_NY
	public class PlayerList : IEnumerable<Player>
	{
		public const int MaxPlayers = 256;

		public static PlayerList Players { get; } = new PlayerList();

		public IEnumerator<Player> GetEnumerator()
		{
			var list = (IList<object>)(object)API.GetActivePlayers();
			foreach (var p in list)
			{
				yield return new Player(Convert.ToInt32(p));
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
