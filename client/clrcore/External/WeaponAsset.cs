using System;
using CitizenFX.Core.Native;
using System.Threading.Tasks;
using CitizenFX.Core;

namespace CitizenFX.Core
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
				return Function.Call<bool>(Native.Hash.IS_WEAPON_VALID, Hash);
			}
		}
		public bool IsLoaded
		{
			get
			{
				return Function.Call<bool>(Native.Hash.HAS_WEAPON_ASSET_LOADED, Hash);
			}
		}

		public void Request()
		{
			Function.Call(Native.Hash.REQUEST_WEAPON_ASSET, Hash, 31, 0);
		}
		public async Task<bool> Request(int timeout)
		{
			Request();

			DateTime endtime = timeout >= 0 ? DateTime.UtcNow + new TimeSpan(0, 0, 0, 0, timeout) : DateTime.MaxValue;

			while (!IsLoaded)
			{
                await BaseScript.Delay(0);
                Request();

				if (DateTime.UtcNow >= endtime)
				{
					return false;
				}
			}

			return true;
		}
		public void Dismiss()
		{
			Function.Call(Native.Hash.REMOVE_WEAPON_ASSET, Hash);
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
