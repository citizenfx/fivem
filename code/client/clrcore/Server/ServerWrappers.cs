using CitizenFX.Core.Native;

using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

#if MONO_V2
using CitizenFX.Core;
using static CitizenFX.Server.Native.Natives;

namespace CitizenFX.Server
{
	public class Player	: Shared.Player
	{
		internal Player(Remote remote)
		{
			m_handle = remote.GetPlayerHandle();
		}

#else
using static CitizenFX.Core.Native.API;

namespace CitizenFX.Core
{
	public class Player
	{
		private string m_handle;

		public string Handle => m_handle;
#endif

		internal Player(string sourceString)
		{
			if (sourceString.StartsWith("net:"))
			{
				sourceString = sourceString.Substring(4);
			}
#if IS_FXSERVER
			else if (sourceString.StartsWith("internal-net:"))
			{
				sourceString = sourceString.Substring(13);
			}
#endif

			m_handle = sourceString;
		}

		public int Ping => GetPlayerPing(m_handle);

		public int LastMsg => GetPlayerLastMsg(m_handle);

		public IdentifierCollection Identifiers => new IdentifierCollection(this);

		public string EndPoint => GetPlayerEndpoint(m_handle);

		public void Drop(string reason) => DropPlayer(m_handle, reason);

#if MONO_V2
		public override string Name => GetPlayerName(m_handle);

		public override StateBag State => new StateBag("player:" + m_handle);

		public Ped Character => Ped.FromPlayerHandle(m_handle);

		public override Shared.IPed GetCharacter() => Character;

		public static implicit operator Player(Remote remote) => new Player(remote.GetPlayerHandle());
#else
		public string Name => GetPlayerName(m_handle);

		public StateBag State => new StateBag("player:" + m_handle);

		public Ped Character => Ped.FromPlayerHandle(m_handle);
#endif

		public void TriggerEvent(string eventName, params object[] args)
		{
#if MONO_V2
			CoreNatives.TriggerClientEventInternal(eventName, m_handle, args);
#else
			var argsSerialized = MsgPackSerializer.Serialize(args);

			unsafe
			{
				fixed (byte* serialized = &argsSerialized[0])
				{
					Function.Call(Hash.TRIGGER_CLIENT_EVENT_INTERNAL, eventName, m_handle, serialized, argsSerialized.Length);
				}
			}
#endif
		}

		public void TriggerLatentEvent(string eventName, int bytesPerSecond, params object[] args)
		{
#if MONO_V2
			CoreNatives.TriggerLatentClientEventInternal(eventName, m_handle, args, bytesPerSecond);
#else
			var argsSerialized = MsgPackSerializer.Serialize(args);

			unsafe
			{
				fixed (byte* serialized = &argsSerialized[0])
				{
					Function.Call(Hash.TRIGGER_LATENT_CLIENT_EVENT_INTERNAL, eventName, m_handle, serialized, argsSerialized.Length, bytesPerSecond);
				}
			}
#endif
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
#if MONO_V2
			int numIndices = GetNumPlayerIdentifiers(m_player.m_handle);
			for (var i = 0; i < numIndices; i++)
			{
				yield return GetPlayerIdentifier(m_player.m_handle, i);
			}
#else
			int numIndices = GetNumPlayerIdentifiers(m_player.Handle);
			for (var i = 0; i < numIndices; i++)
			{
				yield return GetPlayerIdentifier(m_player.Handle, i);
			}
#endif
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
		public string this[string type] => this.FirstOrDefault(id => id.Split(':')[0].Equals(type, StringComparison.InvariantCultureIgnoreCase))?.Split(new char[] { ':' }, 2)?.Last();
	}

	public class PlayerList : IEnumerable<Player>
	{
		public IEnumerator<Player> GetEnumerator()
		{
			int numIndices = GetNumPlayerIndices();

			for (var i = 0; i < numIndices; i++)
			{
				yield return new Player(GetPlayerFromIndex(i));
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
