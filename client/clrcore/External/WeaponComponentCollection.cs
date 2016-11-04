using System;
using System.Collections.Generic;
using System.Linq;
using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
	public enum WeaponComponentHash : uint
	{
		AdvancedRifleClip01 = 4203716879u,
		AdvancedRifleClip02 = 2395064697u,
		AdvancedRifleVarmodLuxe = 930927479u,
		APPistolClip01 = 834974250u,
		APPistolClip02 = 614078421u,
		APPistolVarmodLuxe = 2608252716u,
		AssaultRifleClip01 = 3193891350u,
		AssaultRifleClip02 = 2971750299u,
		AssaultRifleClip03 = 3689981245u,
		AssaultRifleVarmodLuxe = 1319990579u,
		AssaultSMGClip01 = 2366834608u,
		AssaultSMGClip02 = 3141985303u,
		AssaultSMGVarmodLowrider = 663517359u,
		AssaultShotgunClip01 = 2498239431u,
		AssaultShotgunClip02 = 2260565874u,
		AtArAfGrip = 202788691u,
		AtArFlsh = 2076495324u,
		AtArSupp = 2205435306u,
		AtArSupp02 = 2805810788u,
		AtPiFlsh = 899381934u,
		AtPiSupp = 3271853210u,
		AtPiSupp02 = 1709866683u,
		AtRailCover01 = 1967214384u,
		AtScopeLarge = 3527687644u,
		AtScopeLargeFixedZoom = 471997210u,
		AtScopeMacro = 2637152041u,
		AtScopeMacro02 = 1019656791u,
		AtScopeMax = 3159677559u,
		AtScopeMedium = 2698550338u,
		AtScopeSmall = 2855028148u,
		AtScopeSmall02 = 1006677997u,
		AtSrSupp = 3859329886u,
		BullpupRifleClip01 = 3315675008u,
		BullpupRifleClip02 = 3009973007u,
		BullpupRifleVarmodLow = 2824322168u,
		BullpupShotgunClip01 = 3377353998u,
		CarbineRifleClip01 = 2680042476u,
		CarbineRifleClip02 = 2433783441u,
		CarbineRifleClip03 = 3127044405u,
		CarbineRifleVarmodLuxe = 3634075224u,
		CombatMGClip01 = 3791631178u,
		CombatMGClip02 = 3603274966u,
		CombatMGVarmodLowrider = 2466172125u,
		CombatPDWClip01 = 1125642654u,
		CombatPDWClip02 = 860508675u,
		CombatPDWClip03 = 1857603803u,
		CombatPistolClip01 = 119648377u,
		CombatPistolClip02 = 3598405421u,
		CombatPistolVarmodLowrider = 3328527730u,
		CompactRifleClip01 = 1363085923u,
		CompactRifleClip02 = 1509923832u,
		CompactRifleClip03 = 3322377230u,
		DBShotgunClip01 = 703231006u,
		FireworkClip01 = 3840197261u,
		FlareGunClip01 = 2481569177u,
		FlashlightLight = 3719772431u,
		GrenadeLauncherClip01 = 296639639u,
		GusenbergClip01 = 484812453u,
		GusenbergClip02 = 3939025520u,
		HeavyPistolClip01 = 222992026u,
		HeavyPistolClip02 = 1694090795u,
		HeavyPistolVarmodLuxe = 2053798779u,
		HeavyShotgunClip01 = 844049759u,
		HeavyShotgunClip02 = 2535257853u,
		HeavyShotgunClip03 = 2294798931u,
		HeavySniperClip01 = 1198478068u,
		HomingLauncherClip01 = 4162006335u,
		KnuckleVarmodBallas = 4007263587u,
		KnuckleVarmodBase = 4081463091u,
		KnuckleVarmodDiamond = 2539772380u,
		KnuckleVarmodDollar = 1351683121u,
		KnuckleVarmodHate = 2112683568u,
		KnuckleVarmodKing = 3800804335u,
		KnuckleVarmodLove = 1062111910u,
		KnuckleVarmodPimp = 3323197061u,
		KnuckleVarmodPlayer = 146278587u,
		KnuckleVarmodVagos = 2062808965u,
		MGClip01 = 4097109892u,
		MGClip02 = 2182449991u,
		MGVarmodLowrider = 3604658878u,
		MachinePistolClip01 = 1198425599u,
		MachinePistolClip02 = 3106695545u,
		MachinePistolClip03 = 2850671348u,
		MarksmanPistolClip01 = 3416146413u,
		MarksmanRifleClip01 = 3627761985u,
		MarksmanRifleClip02 = 3439143621u,
		MarksmanRifleVarmodLuxe = 371102273u,
		MicroSMGClip01 = 3410538224u,
		MicroSMGClip02 = 283556395u,
		MicroSMGVarmodLuxe = 1215999497u,
		MinigunClip01 = 3370020614u,
		MusketClip01 = 1322387263u,
		Pistol50Clip01 = 580369945u,
		Pistol50Clip02 = 3654528146u,
		Pistol50VarmodLuxe = 2008591151u,
		PistolClip01 = 4275109233u,
		PistolClip02 = 3978713628u,
		PistolVarmodLuxe = 3610841222u,
		PoliceTorchFlashlight = 3315797997u,
		PumpShotgunClip01 = 3513717816u,
		PumpShotgunVarmodLowrider = 2732039643u,
		RPGClip01 = 1319465907u,
		RailgunClip01 = 59044840u,
		RevolverClip01 = 3917905123u,
		RevolverVarmodBoss = 384708672u,
		RevolverVarmodGoon = 2492708877u,
		SMGClip01 = 643254679u,
		SMGClip02 = 889808635u,
		SMGClip03 = 2043113590u,
		SMGVarmodLuxe = 663170192u,
		SNSPistolClip01 = 4169150169u,
		SNSPistolClip02 = 2063610803u,
		SNSPistolVarmodLowrider = 2150886575u,
		SawnoffShotgunClip01 = 3352699429u,
		SawnoffShotgunVarmodLuxe = 2242268665u,
		SniperRifleClip01 = 2613461129u,
		SniperRifleVarmodLuxe = 1077065191u,
		SpecialCarbineClip01 = 3334989185u,
		SpecialCarbineClip02 = 2089537806u,
		SpecialCarbineClip03 = 1801039530u,
		SpecialCarbineVarmodLowrider = 1929467122u,
		SwitchbladeVarmodBase = 2436343040u,
		SwitchbladeVarmodVar1 = 1530822070u,
		SwitchbladeVarmodVar2 = 3885209186u,
		VintagePistolClip01 = 1168357051u,
		VintagePistolClip02 = 867832552u,
		Invalid = 4294967295u
	}

	public class WeaponComponentCollection
	{
		#region Fields

		readonly Ped _owner;
		readonly Weapon _weapon;

		readonly Dictionary<WeaponComponentHash, WeaponComponent> _weaponComponents =
			new Dictionary<WeaponComponentHash, WeaponComponent>();

		readonly WeaponComponentHash[] _components;
		readonly static InvalidWeaponComponent _invalidComponent = new InvalidWeaponComponent();

		#endregion

		internal WeaponComponentCollection(Ped owner, Weapon weapon)
		{
			_owner = owner;
			_weapon = weapon;
			_components = GetComponentsFromHash(weapon.Hash);
		}

		public WeaponComponent this[WeaponComponentHash componentHash]
		{
			get
			{
				if (_components.Contains(componentHash))
				{
					WeaponComponent component = null;
					if (!_weaponComponents.TryGetValue(componentHash, out component))
					{
						component = new WeaponComponent(_owner, _weapon, componentHash);
						_weaponComponents.Add(componentHash, component);
					}

					return component;
				}
				else
				{
					return _invalidComponent;
				}
			}
		}

		public WeaponComponent this[int index]
		{
			get
			{
				WeaponComponent component = null;
				if (index >= 0 && index < Count)
				{
					WeaponComponentHash componentHash = _components[index];

					if (!_weaponComponents.TryGetValue(componentHash, out component))
					{
						component = new WeaponComponent(_owner, _weapon, componentHash);
						_weaponComponents.Add(componentHash, component);
					}
					return component;
				}
				else
				{
					return _invalidComponent;
				}
			}
		}

		public int Count
		{
			get { return _components.Length; }
		}

		public IEnumerator<WeaponComponent> GetEnumerator()
		{
			WeaponComponent[] AllComponents = new WeaponComponent[Count];
			for (int i = 0; i < Count; i++)
			{
				AllComponents[i] = this[_components[i]];
			}
			return (AllComponents as IEnumerable<WeaponComponent>).GetEnumerator();
		}

		public WeaponComponent GetClipComponent(int index)
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.Clip ||
				    component.AttachmentPoint == ComponentAttachmentPoint.Clip2)
				{
					if (index-- == 0)
					{
						return component;
					}
				}
			}
			return _invalidComponent;
		}

		public int ClipVariationsCount
		{
			get
			{
				int count = 0;
				foreach (var component in this)
				{
					if (component.AttachmentPoint == ComponentAttachmentPoint.Clip ||
					component.AttachmentPoint == ComponentAttachmentPoint.Clip2)
					{
						count++;
					}
				}
				return count;
			}
		}

		public WeaponComponent GetScopeComponent(int index)
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.Scope ||
					component.AttachmentPoint == ComponentAttachmentPoint.Scope2)
				{
					if (index-- == 0)
					{
						return component;
					}
				}
			}
			return _invalidComponent;
		}

		public int ScopeVariationsCount
		{
			get
			{
				int count = 0;
				foreach (var component in this)
				{
					if (component.AttachmentPoint == ComponentAttachmentPoint.Scope ||
					component.AttachmentPoint == ComponentAttachmentPoint.Scope2)
					{
						count++;
					}
				}
				return count;
			}
		}

		public WeaponComponent GetSuppressorComponent()
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.Supp ||
					component.AttachmentPoint == ComponentAttachmentPoint.Supp2)
				{
					return component;
				}
			}
			return _invalidComponent;
		}

		public WeaponComponent GetFlashLightComponent()
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.FlashLaser ||
					component.AttachmentPoint == ComponentAttachmentPoint.FlashLaser2)
				{
					return component;
				}
			}
			return _invalidComponent;
		}

		public WeaponComponent GetLuxuryFinishComponent()
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.GunRoot)
				{
					return component;
				}
			}
			return _invalidComponent;
		}

        [SecuritySafeCritical]
        public static WeaponComponentHash[] GetComponentsFromHash(WeaponHash hash)
        {
            return _GetComponentsFromHash(hash);
        }

        [SecurityCritical]

        private static WeaponComponentHash[] _GetComponentsFromHash(WeaponHash hash)
		{
			switch (hash)
			{
				case WeaponHash.Pistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.PistolClip01,
						WeaponComponentHash.PistolClip02,
						WeaponComponentHash.AtPiFlsh,
						WeaponComponentHash.AtPiSupp02,
						WeaponComponentHash.PistolVarmodLuxe,
					};
				case WeaponHash.CombatPistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CombatPistolClip01,
						WeaponComponentHash.CombatPistolClip02,
						WeaponComponentHash.AtPiFlsh,
						WeaponComponentHash.AtPiSupp,
						WeaponComponentHash.CombatPistolVarmodLowrider,
					};
				case WeaponHash.APPistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.APPistolClip01,
						WeaponComponentHash.APPistolClip02,
						WeaponComponentHash.AtPiFlsh,
						WeaponComponentHash.AtPiSupp,
						WeaponComponentHash.APPistolVarmodLuxe,
					};
				case WeaponHash.MicroSMG:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MicroSMGClip01,
						WeaponComponentHash.MicroSMGClip02,
						WeaponComponentHash.AtPiFlsh,
						WeaponComponentHash.AtScopeMacro,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.MicroSMGVarmodLuxe,
					};
				case WeaponHash.SMG:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SMGClip01,
						WeaponComponentHash.SMGClip02,
						WeaponComponentHash.SMGClip03,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtPiSupp,
						WeaponComponentHash.AtScopeMacro02,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.SMGVarmodLuxe,
					};
				case WeaponHash.AssaultRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AssaultRifleClip01,
						WeaponComponentHash.AssaultRifleClip02,
						WeaponComponentHash.AssaultRifleClip03,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeMacro,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AssaultRifleVarmodLuxe,
					};
				case WeaponHash.CarbineRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CarbineRifleClip01,
						WeaponComponentHash.CarbineRifleClip02,
						WeaponComponentHash.CarbineRifleClip03,
						WeaponComponentHash.AtRailCover01,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeMedium,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.CarbineRifleVarmodLuxe,
					};
				case WeaponHash.AdvancedRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AdvancedRifleClip01,
						WeaponComponentHash.AdvancedRifleClip02,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeSmall,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.AdvancedRifleVarmodLuxe,
					};
				case WeaponHash.MG:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MGClip01,
						WeaponComponentHash.MGClip02,
						WeaponComponentHash.AtScopeSmall02,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.MGVarmodLowrider,
					};
				case WeaponHash.CombatMG:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CombatMGClip01,
						WeaponComponentHash.CombatMGClip02,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.AtScopeMedium,
						WeaponComponentHash.CombatMGVarmodLowrider,
					};
				case WeaponHash.PumpShotgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AtSrSupp,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.PumpShotgunVarmodLowrider,
					};
				case WeaponHash.AssaultShotgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AssaultShotgunClip01,
						WeaponComponentHash.AssaultShotgunClip02,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtArSupp,
					};
				case WeaponHash.SniperRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SniperRifleClip01,
						WeaponComponentHash.AtScopeLarge,
						WeaponComponentHash.AtScopeMax,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.SniperRifleVarmodLuxe,
					};
				case WeaponHash.HeavySniper:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.HeavySniperClip01,
						WeaponComponentHash.AtScopeLarge,
						WeaponComponentHash.AtScopeMax,
					};
				case WeaponHash.GrenadeLauncher:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeSmall,
					};
				case WeaponHash.Minigun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MinigunClip01,
					};
				case WeaponHash.AssaultSMG:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AssaultSMGClip01,
						WeaponComponentHash.AssaultSMGClip02,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeMacro,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AssaultSMGVarmodLowrider,
					};
				case WeaponHash.BullpupShotgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtArSupp02,
					};
				case WeaponHash.Pistol50:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.Pistol50Clip01,
						WeaponComponentHash.Pistol50Clip02,
						WeaponComponentHash.AtPiFlsh,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.Pistol50VarmodLuxe,
					};
				case WeaponHash.CombatPDW:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CombatPDWClip01,
						WeaponComponentHash.CombatPDWClip02,
						WeaponComponentHash.CombatPDWClip03,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeSmall,
						WeaponComponentHash.AtArAfGrip,
					};
				case WeaponHash.SawnOffShotgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SawnoffShotgunVarmodLuxe,
					};
				case WeaponHash.BullpupRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.BullpupRifleClip01,
						WeaponComponentHash.BullpupRifleClip02,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeSmall,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.BullpupRifleVarmodLow,
					};
				case WeaponHash.SNSPistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SNSPistolClip01,
						WeaponComponentHash.SNSPistolClip02,
						WeaponComponentHash.SNSPistolVarmodLowrider,
					};
				case WeaponHash.SpecialCarbine:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SpecialCarbineClip01,
						WeaponComponentHash.SpecialCarbineClip02,
						WeaponComponentHash.SpecialCarbineClip03,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeMedium,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.SpecialCarbineVarmodLowrider,
					};
				case WeaponHash.KnuckleDuster:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.KnuckleVarmodPimp,
						WeaponComponentHash.KnuckleVarmodBallas,
						WeaponComponentHash.KnuckleVarmodDollar,
						WeaponComponentHash.KnuckleVarmodDiamond,
						WeaponComponentHash.KnuckleVarmodHate,
						WeaponComponentHash.KnuckleVarmodLove,
						WeaponComponentHash.KnuckleVarmodPlayer,
						WeaponComponentHash.KnuckleVarmodKing,
						WeaponComponentHash.KnuckleVarmodVagos,
					};
				case WeaponHash.MachinePistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MachinePistolClip01,
						WeaponComponentHash.MachinePistolClip02,
						WeaponComponentHash.MachinePistolClip03,
						WeaponComponentHash.AtPiSupp,
					};
				case WeaponHash.SwitchBlade:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SwitchbladeVarmodVar1,
						WeaponComponentHash.SwitchbladeVarmodVar2,
					};
				case WeaponHash.Revolver:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.RevolverClip01,
						WeaponComponentHash.RevolverVarmodBoss,
						WeaponComponentHash.RevolverVarmodGoon,
					};
			}
			
			WeaponComponentHash[] result = null;

			for (int i = 0, count = Function.Call<int>(Native.Hash.GET_NUM_DLC_WEAPONS); i < count; i++)
			{
				unsafe
				{
					DlcWeaponData weaponData;
					if (Function.Call<bool>(Native.Hash.GET_DLC_WEAPON_DATA, i, &weaponData))
					{
						if (weaponData.Hash == hash)
						{
							result = new WeaponComponentHash[Function.Call<int>(Native.Hash.GET_NUM_DLC_WEAPON_COMPONENTS, i)];
							
							for (int j = 0; j < result.Length; j++)
							{
								DlcWeaponComponentData componentData;
								if (Function.Call<bool>(Native.Hash.GET_DLC_WEAPON_COMPONENT_DATA, i, j, &componentData))
								{
									result[j] = componentData.Hash;
								}
								else
								{
									result[j] = WeaponComponentHash.Invalid;
								}
							}
							return result;
						}
					}
				}
			}
			

			if (result == null)
			{
				result = new WeaponComponentHash[0];
			}

			return result;
		}
	}
}