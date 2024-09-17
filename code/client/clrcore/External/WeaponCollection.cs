using System.Collections.Generic;

#if MONO_V2
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{
	public sealed class WeaponCollection
	{
		#region Fields
		Ped _owner;
		readonly Dictionary<WeaponHash, Weapon> _weapons = new Dictionary<WeaponHash, Weapon>();
		#endregion

		internal WeaponCollection(Ped owner)
		{
			_owner = owner;
		}

		public Weapon this[WeaponHash hash]
		{
			get
			{
				Weapon weapon = null;

				if (!_weapons.TryGetValue(hash, out weapon))
				{
					if (!API.HasPedGotWeapon(_owner.Handle, (uint)hash, false))
					{
						return null;
					}

					weapon = new Weapon(_owner, hash);
					_weapons.Add(hash, weapon);
				}

				return weapon;
			}
		}

		public Weapon Current
		{
			get
			{
				uint currentWeapon = 0u;
				API.GetCurrentPedWeapon(_owner.Handle, ref currentWeapon, true);

				WeaponHash hash = (WeaponHash)currentWeapon;

				if (_weapons.ContainsKey(hash))
				{
					return _weapons[hash];
				}
				else
				{
					var weapon = new Weapon(_owner, hash);
					_weapons.Add(hash, weapon);

					return weapon;
				}
			}
		}

		public Prop CurrentWeaponObject
		{
			get
			{
				if (Current.Hash == WeaponHash.Unarmed)
				{
					return null;
				}

				return new Prop(API.GetCurrentPedWeaponEntityIndex(_owner.Handle));
			}
		}
		public Weapon BestWeapon
		{
			get
			{
				WeaponHash hash = (WeaponHash)API.GetBestPedWeapon(_owner.Handle, false);

				if (_weapons.ContainsKey(hash))
				{
					return _weapons[hash];
				}
				else
				{
					var weapon = new Weapon(_owner, hash);
					_weapons.Add(hash, weapon);

					return weapon;
				}
			}
		}

		public bool HasWeapon(WeaponHash weaponHash)
		{
			return API.HasPedGotWeapon(_owner.Handle, (uint)weaponHash, false);
		}
		public bool IsWeaponValid(WeaponHash hash)
		{
			return API.IsWeaponValid((uint)hash);
		}

		public Weapon Give(WeaponHash hash, int ammoCount, bool equipNow, bool isAmmoLoaded)
		{
			Weapon weapon = null;

			if (!_weapons.TryGetValue(hash, out weapon))
			{
				weapon = new Weapon(_owner, hash);
				_weapons.Add(hash, weapon);
			}

			if (weapon.IsPresent)
			{
				Select(weapon);
			}
			else
			{
				API.GiveWeaponToPed(_owner.Handle, (uint)weapon.Hash, ammoCount, equipNow, isAmmoLoaded);
			}

			return weapon;
		}
		public bool Select(Weapon weapon)
		{
			if (!weapon.IsPresent)
			{
				return false;
			}

			API.SetCurrentPedWeapon(_owner.Handle, (uint)weapon.Hash, true);

			return true;
		}
		public bool Select(WeaponHash weaponHash)
		{
			return Select(weaponHash, true);
		}
		public bool Select(WeaponHash weaponHash, bool equipNow)
		{
			if (!API.HasPedGotWeapon(_owner.Handle, (uint)weaponHash, false))
			{
				return false;
			}

			API.SetCurrentPedWeapon(_owner.Handle, (uint)weaponHash, equipNow);

			return true;
		}

		public void Drop()
		{
			API.SetPedDropsWeapon(_owner.Handle);
		}
		public void Remove(Weapon weapon)
		{
			WeaponHash hash = weapon.Hash;

			if (_weapons.ContainsKey(hash))
			{
				_weapons.Remove(hash);
			}

			Remove(weapon.Hash);
		}
		public void Remove(WeaponHash weaponHash)
		{
			API.RemoveWeaponFromPed(_owner.Handle, (uint)weaponHash);
		}
		public void RemoveAll()
		{
			API.RemoveAllPedWeapons(_owner.Handle, true);

			_weapons.Clear();
		}
	}
}
