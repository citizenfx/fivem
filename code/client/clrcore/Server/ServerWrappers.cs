using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	public class Player
	{
		private string m_handle;

		public string Handle => m_handle;

		internal Player(string sourceString)
		{
			if (sourceString.StartsWith("net:"))
			{
				sourceString = sourceString.Substring(4);
			}

			m_handle = sourceString;
		}

		public string Name => API.GetPlayerName(m_handle);

		public int Ping => API.GetPlayerPing(m_handle);

		public int LastMsg => API.GetPlayerLastMsg(m_handle);

		public IdentifierCollection Identifiers => new IdentifierCollection(this);

		public string EndPoint => API.GetPlayerEndpoint(m_handle);

		public Ped Character => Ped.FromPlayerHandle(m_handle);

		public void Drop(string reason) => API.DropPlayer(m_handle, reason);

		public void TriggerEvent(string eventName, params object[] args)
		{
			var argsSerialized = MsgPackSerializer.Serialize(args);

			unsafe
			{
				fixed (byte* serialized = &argsSerialized[0])
				{
					API.TriggerClientEventInternal(eventName, m_handle, serialized, argsSerialized.Length);
				}
			}
		}

		protected bool Equals(Player other) => string.Equals(Handle, other.Handle);

		/// <inheritdoc />
		public override bool Equals(object obj)
		{
			if (ReferenceEquals(null, obj))
			{
				return false;
			}

			if (ReferenceEquals(this, obj))
			{
				return true;
			}

			if (obj.GetType() != this.GetType())
			{
				return false;
			}

			return Equals((Player) obj);
		}

		/// <inheritdoc />
		public override int GetHashCode() => (m_handle != null ? m_handle.GetHashCode() : 0);

		public static bool operator ==(Player left, Player right) => Equals(left, right);

		public static bool operator !=(Player left, Player right) => !Equals(left, right);
	}

	public class IdentifierCollection : IEnumerable<string>
	{
		private Player m_player;

		internal IdentifierCollection(Player player)
		{
			m_player = player;
		}

		public IEnumerator<string> GetEnumerator()
		{
			int numIndices = API.GetNumPlayerIdentifiers(m_player.Handle);

			for (var i = 0; i < numIndices; i++)
			{
				yield return API.GetPlayerIdentifier(m_player.Handle, i);
			}
		}

		IEnumerator IEnumerable.GetEnumerator()
		{
			return GetEnumerator();
		}

		/// <summary>
		/// Gets the identifier value of a particular type.
		/// </summary>
		/// <example>
		/// string steamId = player.Identifiers["steam"];
		/// </example>
		/// <param name="type">The identifier type to return.</param>
		/// <returns>The identifier value (without prefix), or null if it could not be found.</returns>
		public string this[string type] => this.FirstOrDefault(id => id.Split(':')[0].Equals(type, StringComparison.InvariantCultureIgnoreCase))?.Split(':')?.Last();
	}

	public class PlayerList : IEnumerable<Player>
	{
		public IEnumerator<Player> GetEnumerator()
		{
			int numIndices = API.GetNumPlayerIndices();

			for (var i = 0; i < numIndices; i++)
			{
				yield return new Player(API.GetPlayerFromIndex(i));
			}
		}

		IEnumerator IEnumerable.GetEnumerator()
		{
			return GetEnumerator();
		}

		public Player this[int netId] => new Player(netId.ToString());

		public Player this[string name] => this.FirstOrDefault(player => player.Name.Equals(name, StringComparison.InvariantCultureIgnoreCase));
	}
}
