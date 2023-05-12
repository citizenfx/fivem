using System;

#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;
using INativeValue = CitizenFX.Core.Native.Input.Primitive;
using TaskBool = CitizenFX.Core.Coroutine<bool>;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;
using System.Threading.Tasks;
using TaskBool = System.Threading.Tasks.Task<bool>;

namespace CitizenFX.Core
#endif
{
	public class WeaponAsset : INativeValue, IEquatable<WeaponAsset>
	{
		public WeaponAsset()
		{

		}
		public WeaponAsset(int hash) : this()
		{
			Hash = hash;
		}
		public WeaponAsset(uint hash) : this((int)hash)
		{
		}
		public WeaponAsset(WeaponHash hash) : this((int)hash)
		{
		}

		public int Hash { get; private set; }
		public override ulong NativeValue
		{
			get { return (ulong)Hash; }
			set { Hash = unchecked((int)value); }
		}

		public bool IsValid
		{
			get
			{
				return API.IsWeaponValid((uint)Hash);
			}
		}
		public bool IsLoaded
		{
			get
			{
				return API.HasWeaponAssetLoaded((uint)Hash);
			}
		}

		public void Request()
		{
			API.RequestWeaponAsset((uint)Hash, 31, 0);
		}
		public async TaskBool Request(int timeout)
		{
			Request();

			DateTime endtime = timeout >= 0 ? DateTime.UtcNow + new TimeSpan(0, 0, 0, 0, timeout) : DateTime.MaxValue;

			while (!IsLoaded)
			{
				await BaseScript.Delay(0);

				if (DateTime.UtcNow >= endtime)
				{
					return false;
				}
			}

			return true;
		}
		public void Dismiss()
		{
			API.RemoveWeaponAsset((uint)Hash);
		}

		public bool Equals(WeaponAsset weaponAsset)
		{
			return Hash == weaponAsset.Hash;
		}
		public override bool Equals(object obj)
		{
			return obj != null && obj.GetType() == GetType() && Equals((WeaponAsset)obj);
		}

		public override int GetHashCode()
		{
			return Hash;
		}

		public string DisplayName
		{
			get { return Weapon.GetDisplayNameFromHash((WeaponHash)Hash); }
		}

		public string LocalizedName
		{
			get { return Game.GetGXTEntry(Weapon.GetDisplayNameFromHash((WeaponHash)Hash)); }
		}

		public override string ToString()
		{
			return "0x" + Hash.ToString("X");
		}

		public static implicit operator WeaponAsset(int hash)
		{
			return new WeaponAsset(hash);
		}
		public static implicit operator WeaponAsset(uint hash)
		{
			return new WeaponAsset(hash);
		}
		public static implicit operator WeaponAsset(WeaponHash hash)
		{
			return new WeaponAsset(hash);
		}

		public static bool operator ==(WeaponAsset left, WeaponAsset right)
		{
			return left.Equals(right);
		}
		public static bool operator !=(WeaponAsset left, WeaponAsset right)
		{
			return !left.Equals(right);
		}
	}
}
