using CitizenFX.Core.Native;
using System;
using System.Collections.Generic;
using System.Security;

namespace CitizenFX.Core
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
					if (!Function.Call<bool>(Hash.HAS_PED_GOT_WEAPON, _owner.Handle, hash, 0))
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
            [SecuritySafeCritical]
			get
			{
			    int currentWeapon = _GetCurrentWeapon();

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

        [SecurityCritical]
        private int _GetCurrentWeapon()
        {
            int currentWeapon;

            unsafe
            {
                Function.Call(Hash.GET_CURRENT_PED_WEAPON, _owner.Handle, &currentWeapon, true);
            }

            return currentWeapon;
        }

		public Prop CurrentWeaponObject
		{
			get
			{
				if (Current.Hash == WeaponHash.Unarmed)
				{
					return null;
				}

				return new Prop(Function.Call<int>(Hash.GET_CURRENT_PED_WEAPON_ENTITY_INDEX, _owner.Handle));
			}
		}
		public Weapon BestWeapon
		{
			get
			{
				WeaponHash hash = Function.Call<WeaponHash>(Hash.GET_BEST_PED_WEAPON, _owner.Handle, 0);

				if (_weapons.ContainsKey(hash))
				{
					return _weapons[hash];
				}
				else
				{
					var weapon = new Weapon(_owner, (WeaponHash)hash);
					_weapons.Add(hash, weapon);

					return weapon;
				}
			}
		}

		public bool HasWeapon(WeaponHash weaponHash)
		{
			return Function.Call<bool>(Hash.HAS_PED_GOT_WEAPON, _owner.Handle, weaponHash);
		}
		public bool IsWeaponValid(WeaponHash hash)
		{
			return Function.Call<bool>(Hash.IS_WEAPON_VALID, hash);
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
				Function.Call(Hash.GIVE_WEAPON_TO_PED, _owner.Handle, weapon.Hash, ammoCount, equipNow, isAmmoLoaded);
			}

			return weapon;
		}
		public bool Select(Weapon weapon)
		{
			if (!weapon.IsPresent)
			{
				return false;
			}

			Function.Call(Hash.SET_CURRENT_PED_WEAPON, _owner.Handle, weapon.Hash, true);

			return true;
		}
		public bool Select(WeaponHash weaponHash)
		{
			return Select(weaponHash, true);
		}
		public bool Select(WeaponHash weaponHash, bool equipNow)
		{
			if (!Function.Call<bool>(Hash.HAS_PED_GOT_WEAPON, _owner.Handle, weaponHash))
			{
				return false;
			}

			Function.Call(Hash.SET_CURRENT_PED_WEAPON, _owner.Handle, weaponHash, equipNow);

			return true;
		}

		public void Drop()
		{
			Function.Call(Hash.SET_PED_DROPS_WEAPON, _owner.Handle);
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
			Function.Call(Hash.REMOVE_WEAPON_FROM_PED, _owner.Handle, weaponHash);
		}
		public void RemoveAll()
		{
			Function.Call(Hash.REMOVE_ALL_PED_WEAPONS, _owner.Handle, true);

			_weapons.Clear();
		}
	}
}
