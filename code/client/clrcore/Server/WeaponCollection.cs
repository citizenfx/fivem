using CitizenFX.Core.Native;
using System.Collections.Generic;

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

		public Weapon Give(WeaponHash hash, int ammoCount, bool equipNow, bool isAmmoLoaded)
		{
			Weapon weapon = null;

			if (!_weapons.TryGetValue(hash, out weapon))
			{
				weapon = new Weapon(_owner, hash);
				_weapons.Add(hash, weapon);
			}

			Select(weapon);

			return weapon;
		}
		public bool Select(Weapon weapon)
		{
			API.SetCurrentPedWeapon(_owner.Handle, (uint)weapon.Hash, true);

			return true;
		}
		public bool Select(WeaponHash weaponHash)
		{
			return Select(weaponHash, true);
		}
		public bool Select(WeaponHash weaponHash, bool equipNow)
		{
			API.SetCurrentPedWeapon(_owner.Handle, (uint)weaponHash, equipNow);

			return true;
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