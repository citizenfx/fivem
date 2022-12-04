using System;
using System.Security;

#if MONO_V2
using API = CitizenFX.FiveM.Native.Natives;
using Function = CitizenFX.FiveM.Native.Natives;
using compat_i32_u32 = System.UInt32;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;
using compat_i32_u32 = System.Int32;

namespace CitizenFX.Core
#endif
{
	public enum WeaponTint
	{
		Mk2ClassicBlack,
		Mk2ClassicGray,
		Mk2ClassicTwoTone,
		Mk2ClassicWhite,
		Mk2ClassicBeige,
		Mk2ClassicGreen,
		Mk2ClassicBlue,
		Mk2ClassicEarth,
		Mk2ClassicBrownAndBlack,
		Mk2RedContrast,
		Mk2BlueContrast,
		Mk2YellowContrast,
		Mk2OrangeContrast,
		Mk2BoldPink,
		Mk2BoldPurpleAndYellow,
		Mk2BoldOrange,
		Mk2BoldGreenAndPurple,
		Mk2BoldRedFeatures,
		Mk2BoldGreenFeatures,
		Mk2BoldCyanFeatures,
		Mk2BoldYellowFeatures,
		Mk2BoldRedAndWhite,
		Mk2BoldBlueAndWhite,
		Mk2MetallicGold,
		Mk2MetallicPlatinum,
		Mk2MetallicGrayAndLilac,
		Mk2MetallicPurpleAndLime,
		Mk2MetallicRed,
		Mk2MetallicGreen,
		Mk2MetallicBlue,
		Mk2MetallicWhiteAndAqua,
		Mk2MetallicRedAndYellow,
		Normal = 0,
		Green = 1,
		Gold = 2,
		Pink = 3,
		Army = 4,
		LSPD = 5,
		Orange = 6,
		Platinum = 7,
	}
	public enum WeaponGroup : uint
	{
		Unarmed = 2685387236u,
		Melee = 3566412244u,
		Pistol = 416676503u,
		SMG = 3337201093u,
		AssaultRifle = 970310034u,
		DigiScanner = 3539449195u,
		FireExtinguisher = 4257178988u,
		MG = 1159398588u,
		NightVision = 3493187224u,
		Parachute = 431593103u,
		Shotgun = 860033945u,
		Sniper = 3082541095u,
		Stungun = 690389602u,
		Heavy = 2725924767u,
		Thrown = 1548507267u,
		PetrolCan = 1595662460u
	}

	public enum WeaponLivery
	{
		Digital,
		Brushstroke,
		Woodland,
		Skull,
		Sessanta,
		Perseus,
		Leopard,
		Zebra,
		Geometric,
		Boom,
		Patriotic,
	}
	public enum WeaponLiveryColor
	{
		Gray,
		DarkGray,
		Black,
		White,
		Blue,
		Cyan,
		Aqua,
		CoolBlue,
		DarkBlue,
		RoyalBlue,
		Plum,
		DarkPurple,
		Purple,
		Red,
		WineRed,
		Magenta,
		Pink,
		Salmon,
		HotPink,
		RustOrange,
		Brown,
		Earth,
		Orange,
		LightOrange,
		DarkYellow,
		Yellow,
		LightBrown,
		LimeGreen,
		Olive,
		Moss,
		Turquoise,
		DarkGreen
	}

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

		public bool IsPresent
		{
			get
			{
				if (Hash == WeaponHash.Unarmed)
				{
					return true;
				}

				return API.HasPedGotWeapon(_owner.Handle, (uint)Hash, false);
			}
		}
		public string DisplayName
		{
			get
			{
				return GetDisplayNameFromHash(Hash);
			}
		}
		public string LocalizedName
		{
			get
			{
				return Game.GetGXTEntry(DisplayName);
			}
		}
		public Model Model
		{
			get
			{
				return new Model(API.GetWeapontypeModel((uint)Hash));
			}
		}
		public WeaponTint Tint
		{
			get
			{
				return (WeaponTint)API.GetPedWeaponTintIndex(_owner.Handle, (uint)Hash);
			}
			set
			{
				API.SetPedWeaponTintIndex(_owner.Handle, (uint)Hash, (int)value);
			}
		}
		public WeaponGroup Group
		{
			get
			{
				return (WeaponGroup)API.GetWeapontypeGroup((uint)Hash);
			}
		}

		public AmmoType AmmoType => (AmmoType)API.GetPedAmmoTypeFromWeapon(_owner.Handle, (uint)Hash);

		public int Ammo
		{
			get
			{
				if (Hash == WeaponHash.Unarmed)
				{
					return 1;
				}

				if (!IsPresent)
				{
					return API.GetPedAmmoByType(_owner.Handle, (compat_i32_u32)AmmoType);
				}

				return API.GetAmmoInPedWeapon(_owner.Handle, (uint)Hash);
			}
			set
			{
				if (Hash == WeaponHash.Unarmed)
				{
					return;
				}

				if (IsPresent)
				{
					API.SetPedAmmo(_owner.Handle, (uint)Hash, value);
				}
				else
				{
					API.GiveWeaponToPed(_owner.Handle, (uint)Hash, value, false, true);
				}
			}
		}
		public int AmmoInClip
		{
			get
			{
				if (Hash == WeaponHash.Unarmed)
				{
					return 1;
				}

				if (!IsPresent)
				{
					return 0;
				}

				int ammoInClip = 0;
				API.GetAmmoInClip(_owner.Handle, (uint)Hash, ref ammoInClip);

				return ammoInClip;
			}
			set
			{
				if (Hash == WeaponHash.Unarmed)
				{
					return;
				}

				if (IsPresent)
				{
					API.SetAmmoInClip(_owner.Handle, (uint)Hash, value);
				}
				else
				{
					API.GiveWeaponToPed(_owner.Handle, (uint)Hash, value, true, false);
				}
			}
		}
		public int MaxAmmo
		{
			get
			{
				if (Hash == WeaponHash.Unarmed)
				{
					return 1;
				}

				int maxAmmo = 0;
				API.GetMaxAmmo(_owner.Handle, (uint)Hash, ref maxAmmo);

				return maxAmmo;
			}
		}
		public int MaxAmmoInClip
		{
			get
			{
				if (Hash == WeaponHash.Unarmed)
				{
					return 1;
				}

				if (!IsPresent)
				{
					return 0;
				}

				return API.GetMaxAmmoInClip(_owner.Handle, (uint)Hash, true);
			}
		}
		public int DefaultClipSize
		{
			get
			{
				return API.GetWeaponClipSize((uint)Hash);
			}
		}
		public bool InfiniteAmmo
		{
			set
			{
				if (Hash == WeaponHash.Unarmed)
				{
					return;
				}

				API.SetPedInfiniteAmmo(_owner.Handle, value, (uint)Hash);
			}
		}
		public bool InfiniteAmmoClip
		{
			set
			{
				API.SetPedInfiniteAmmoClip(_owner.Handle, value);
			}
		}

		public bool CanUseOnParachute
		{
			get
			{
				return API.CanUseWeaponOnParachute((uint)Hash);
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

		public void SetLivery(WeaponLivery liveryID, WeaponLiveryColor colorID)
		{
			if (IsMk2)
			{
				WeaponComponent comp = Components.GetMk2CamoComponent((int)liveryID);
				comp.Active = true;
				API.N_0x9fe5633880ecd8ed(Game.PlayerPed.Handle, (compat_i32_u32)Hash, (compat_i32_u32)comp.ComponentHash, (int)colorID);
			}
			else
			{
				throw new InvalidOperationException("You can't set the livery of a non-Mk2 weapon.");
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
			DlcWeaponData data;
			for (int i = 0, count = API.GetNumDlcWeapons(); i < count; i++)
			{
				unsafe
				{
					// struct native, can't be converted.
					if (Function.Call<bool>(Native.Hash.GET_DLC_WEAPON_DATA, i, &data))
					{
						if (data.Hash == hash)
						{
							return data.DisplayName;
						}
					}
				}
			}
			return "WT_INVALID";
		}

		/// <summary>
		/// Gets the <see cref="Game.WeaponHudStats"/> data from this <see cref="Weapon"/>.
		/// </summary>
		public Game.WeaponHudStats HudStats
		{
			get
			{
				Game.WeaponHudStats stats = new Game.WeaponHudStats();
				if (Game.GetWeaponHudStats((uint)this.Hash, ref stats))
				{
					return stats;
				}
				return new Game.WeaponHudStats();
			}
		}
	}
}
