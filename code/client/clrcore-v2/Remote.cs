
namespace CitizenFX.Core
{
	public struct Remote
	{
#if IS_FXSERVER
		internal string m_playerId;

		internal Remote(string playerId)
		{
			if (!string.IsNullOrEmpty(playerId))
			{
				if (playerId.StartsWith("net:"))
				{
					playerId = playerId.Substring(4);
				}
				else if (playerId.StartsWith("internal-net:"))
				{
					playerId = playerId.Substring(13);
				}

				m_playerId = playerId;
			}
			else
				m_playerId = null;
		}

		internal Remote(Binding binding, string playerId) : this(playerId) { }
		internal static Remote Create(ushort remote) => new Remote(remote.ToString());

		internal static bool IsRemoteInternal(Remote remote) => remote.m_playerId != null;

		internal string GetPlayerHandle() => m_playerId;

		public override string ToString() => $"Remote({m_playerId})";
#else
		internal bool m_isServer;

		internal Remote(Binding binding, string playerId) => m_isServer = binding == Binding.Remote;
		internal Remote(bool isServer) => m_isServer = isServer;
		internal Remote(ushort remote) => m_isServer = remote != ushort.MaxValue; // TODO: what's server id?

		internal static Remote Create(ushort remote) => new Remote(remote);

		internal static bool IsRemoteInternal(Remote remote) => remote.m_isServer;

		public override string ToString() => $"Remote({m_isServer})";
#endif
		public bool IsRemote => IsRemoteInternal(this);

		public static implicit operator bool(Remote remote) => IsRemoteInternal(remote);
	}
}
