using System;
using System.Security;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
{

	public class StateBag
	{
		public string BagName { get; }

		internal StateBag(string bagName)
		{
			BagName = bagName;
		}

		[SecurityCritical]
		private void SetInternal(string key, byte[] dataSerialized, bool replicated)
		{
			unsafe
			{
				fixed (byte* serialized = &dataSerialized[0])
				{
					Function.Call(Hash.SET_STATE_BAG_VALUE, BagName, key, serialized, dataSerialized.Length, replicated);
				}
			}
		}

		[SecuritySafeCritical]
		public void Set(string key, object data, bool replicated)
		{
			var dataSerialized = MsgPackSerializer.Serialize(data);

			SetInternal(key, dataSerialized, replicated);
		}


		public dynamic Get(string key)
		{
			return API.GetStateBagValue(BagName, key);
		}

		public dynamic this[string key]
		{
			get => Get(key);
			set => Set(key, value, API.IsDuplicityVersion());
		}
	}
}
