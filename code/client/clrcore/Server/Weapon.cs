using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
	public sealed class Weapon
	{
		#region Fields
		Ped _owner;
		WeaponComponentCollection _components;
		#endregion

		internal Weapon()
		{
			Hash = WeaponHash.Unarmed;
		}
		internal Weapon(Ped owner, WeaponHash hash)
		{
			_owner = owner;
			Hash = hash;
		}

		public WeaponHash Hash { get; private set; }
		public static implicit operator WeaponHash(Weapon weapon)
		{
			return weapon.Hash;
		}

		public string DisplayName
		{
			get
			{
				return GetDisplayNameFromHash(Hash);
			}
		}

		public int Ammo
		{
			set
			{
				if (Hash == WeaponHash.Unarmed)
				{
					return;
				}

				API.SetPedAmmo(_owner.Handle, (uint)Hash, value);
			}
		}

		public bool IsMk2
		{
			get
			{
				switch (Hash)
				{
					case WeaponHash.PistolMk2:
					case WeaponHash.AssaultRifleMk2:
					case WeaponHash.CarbineRifleMk2:
					case WeaponHash.CombatMGMk2:
					case WeaponHash.HeavySniperMk2:
					case WeaponHash.SMGMk2:
						return true;
					default:
						return false;
				}
			}
		}

		public WeaponComponentCollection Components
		{
			get
			{
				if (_components == null)
				{
					_components = new WeaponComponentCollection(_owner, this);
				}
				return _components;
			}
		}

		[SecuritySafeCritical]
		public static string GetDisplayNameFromHash(WeaponHash hash)
		{
			return GetDisplayNameFromHashInternal(hash);
		}

		[SecurityCritical]
		private static string GetDisplayNameFromHashInternal(WeaponHash hash)
		{
			switch (hash)
			{
				case WeaponHash.Pistol:
					return "WT_PIST";
				case WeaponHash.CombatPistol:
					return "WT_PIST_CBT";
				case WeaponHash.APPistol:
					return "WT_PIST_AP";
				case WeaponHash.SMG:
					return "WT_SMG";
				case WeaponHash.MicroSMG:
					return "WT_SMG_MCR";
				case WeaponHash.AssaultRifle:
					return "WT_RIFLE_ASL";
				case WeaponHash.CarbineRifle:
					return "WT_RIFLE_CBN";
				case WeaponHash.AdvancedRifle:
					return "WT_RIFLE_ADV";
				case WeaponHash.MG:
					return "WT_MG";
				case WeaponHash.CombatMG:
					return "WT_MG_CBT";
				case WeaponHash.PumpShotgun:
					return "WT_SG_PMP";
				case WeaponHash.SawnOffShotgun:
					return "WT_SG_SOF";
				case WeaponHash.AssaultShotgun:
					return "WT_SG_ASL";
				case WeaponHash.HeavySniper:
					return "WT_SNIP_HVY";
				case WeaponHash.SniperRifle:
					return "WT_SNIP_RIF";
				case WeaponHash.GrenadeLauncher:
					return "WT_GL";
				case WeaponHash.RPG:
					return "WT_RPG";
				case WeaponHash.Minigun:
					return "WT_MINIGUN";
				case WeaponHash.AssaultSMG:
					return "WT_SMG_ASL";
				case WeaponHash.BullpupShotgun:
					return "WT_SG_BLP";
				case WeaponHash.Pistol50:
					return "WT_PIST_50";
				case WeaponHash.Bottle:
					return "WT_BOTTLE";
				case WeaponHash.Gusenberg:
					return "WT_GUSENBERG";
				case WeaponHash.SNSPistol:
					return "WT_SNSPISTOL";
				case WeaponHash.VintagePistol:
					return "TT_VPISTOL";
				case WeaponHash.Dagger:
					return "WT_DAGGER";
				case WeaponHash.FlareGun:
					return "WT_FLAREGUN";
				case WeaponHash.Musket:
					return "WT_MUSKET";
				case WeaponHash.Firework:
					return "WT_FWRKLNCHR";
				case WeaponHash.MarksmanRifle:
					return "WT_HMKRIFLE";
				case WeaponHash.HeavyShotgun:
					return "WT_HVYSHOT";
				case WeaponHash.ProximityMine:
					return "WT_PRXMINE";
				case WeaponHash.HomingLauncher:
					return "WT_HOMLNCH";
				case WeaponHash.CombatPDW:
					return "WT_COMBATPDW";
				case WeaponHash.KnuckleDuster:
					return "WT_KNUCKLE";
				case WeaponHash.MarksmanPistol:
					return "WT_MKPISTOL";
				case WeaponHash.Machete:
					return "WT_MACHETE";
				case WeaponHash.MachinePistol:
					return "WT_MCHPIST";
				case WeaponHash.Flashlight:
					return "WT_FLASHLIGHT";
				case WeaponHash.DoubleBarrelShotgun:
					return "WT_DBSHGN";
				case WeaponHash.CompactRifle:
					return "WT_CMPRIFLE";
				case WeaponHash.SwitchBlade:
					return "WT_SWBLADE";
				case WeaponHash.Revolver:
					return "WT_REVOLVER";
				//mpgunrunning
				case WeaponHash.PistolMk2:
					return "WT_PIST2";
				case WeaponHash.AssaultRifleMk2:
					return "WT_RIFLE_ASL2";
				case WeaponHash.CarbineRifleMk2:
					return "WT_RIFLE_CBN2";
				case WeaponHash.CombatMGMk2:
					return "WT_MG_CBT2";
				case WeaponHash.HeavySniperMk2:
					return "WT_SNIP_HVY2";
				case WeaponHash.SMGMk2:
					return "WT_SMG2";
			}

			return "WT_INVALID";
		}
	}
}