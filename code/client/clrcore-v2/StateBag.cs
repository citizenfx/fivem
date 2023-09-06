using System.Security;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	[SecuritySafeCritical]
	public class StateBag
	{
		private readonly CString m_bagName;

		public static StateBag Global { get; } = new StateBag("global");

		internal StateBag(string bagName)
		{
			m_bagName = (CString)bagName;
		}

		public void Set(string key, object data, bool replicate)
			=> CoreNatives.SetStateBagValue(m_bagName, key, InPacket.Serialize(data), replicate);

		public dynamic Get(string key)
			=> CoreNatives.GetStateBagValue(m_bagName, key);

		public dynamic this[string key]
		{
			get => Get(key);
			set => Set(key, value, Resource.IsServer);
		}
	}
}
