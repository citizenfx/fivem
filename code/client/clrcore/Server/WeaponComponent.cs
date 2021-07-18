using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
	public enum ComponentAttachmentPoint : uint
	{
		Invalid = 4294967295u,
		Clip = 3723347892u,
		Clip2 = 291640902u,
		FlashLaser = 679107254u,
		FlashLaser2 = 2722126698u,
		Supp = 1863181664u,
		Supp2 = 945598191u,
		GunRoot = 962500902u,
		Scope = 196630833u,
		Scope2 = 1684637069u,
		Grip = 2972950469u,
		Grip2 = 3748215485u,
		TorchBulb = 421673795u,
		Rail = 2451679629u,
		Rail2 = 497110245u,
		Barrel = 2982890265u
	}

	public class WeaponComponent
	{
		#region Fields
		protected readonly Ped _owner;
		protected readonly Weapon _weapon;
		protected readonly WeaponComponentHash _component;
		#endregion

		internal WeaponComponent(Ped owner, Weapon weapon, WeaponComponentHash component)
		{
			_owner = owner;
			_weapon = weapon;
			_component = component;
		}

		public WeaponComponentHash ComponentHash
		{
			get
			{
				return _component;
			}
		}

		public static implicit operator WeaponComponentHash(WeaponComponent weaponComponent)
		{
			return weaponComponent.ComponentHash;
		}

		public virtual bool Active
		{
			set
			{
				if (value)
				{
					API.GiveWeaponComponentToPed(_owner.Handle, (uint)_weapon.Hash, (uint)_component);
				}
				else
				{
					API.RemoveWeaponComponentFromPed(_owner.Handle, (uint)_weapon.Hash, (uint)_component);
				}
			}
		}

		public virtual string DisplayName
		{
			get
			{
				return GetComponentDisplayNameFromHash(_weapon.Hash, _component);
			}
		}

		public virtual ComponentAttachmentPoint AttachmentPoint
		{
			get { return GetAttachmentPoint(_weapon.Hash, _component); }
		}

		[SecuritySafeCritical]
		public static string GetComponentDisplayNameFromHash(WeaponHash hash, WeaponComponentHash component)
		{
			return _GetComponentDisplayNameFromHash(hash, component);
		}

		[SecurityCritical]
		private static string _GetComponentDisplayNameFromHash(WeaponHash hash, WeaponComponentHash component)
		{
			if (hash == WeaponHash.KnuckleDuster)
			{
				switch (component)
				{
					case WeaponComponentHash.KnuckleVarmodBase:
						return "WT_KNUCKLE";
					case WeaponComponentHash.KnuckleVarmodPimp:
						return "WCT_KNUCK_02";
					case WeaponComponentHash.KnuckleVarmodBallas:
						return "WCT_KNUCK_BG";
					case WeaponComponentHash.KnuckleVarmodDollar:
						return "WCT_KNUCK_DLR";
					case WeaponComponentHash.KnuckleVarmodDiamond:
						return "WCT_KNUCK_DMD";
					case WeaponComponentHash.KnuckleVarmodHate:
						return "WCT_KNUCK_HT";
					case WeaponComponentHash.KnuckleVarmodLove:
						return "WCT_KNUCK_LV";
					case WeaponComponentHash.KnuckleVarmodPlayer:
						return "WCT_KNUCK_PC";
					case WeaponComponentHash.KnuckleVarmodKing:
						return "WCT_KNUCK_SLG";
					case WeaponComponentHash.KnuckleVarmodVagos:
						return "WCT_KNUCK_VG";
				}
			}
			else
			{
				switch (component)
				{
					case WeaponComponentHash.Invalid:
						return "WCT_INVALID";
					case WeaponComponentHash.AtRailCover01:
						return "WCT_RAIL";
					case WeaponComponentHash.AtArAfGrip:
					case WeaponComponentHash.AtArAfGrip2:
						return "WCT_GRIP";
					case WeaponComponentHash.AtPiFlsh:
					case WeaponComponentHash.AtArFlsh:
					case WeaponComponentHash.PistolMk2Flash:
						return "WCT_FLASH";
					case WeaponComponentHash.AtScopeMacro:
						return "WCT_SCOPE_MAC";
					case WeaponComponentHash.AtScopeMacro02:
						return "WCT_SCOPE_MAC";
					case WeaponComponentHash.AtScopeSmall:
						return "WCT_SCOPE_SML";
					case WeaponComponentHash.AtScopeSmall02:
						return "WCT_SCOPE_SML";
					case WeaponComponentHash.AtScopeMedium:
						return "WCT_SCOPE_MED";
					case WeaponComponentHash.AtScopeLarge:
						return "WCT_SCOPE_LRG";
					case WeaponComponentHash.AtScopeMax:
						return "WCT_SCOPE_MAX";
					case WeaponComponentHash.AtPiSupp:
					case WeaponComponentHash.AtArSupp:
					case WeaponComponentHash.AtArSupp02:
					case WeaponComponentHash.AtSrSupp:
					case WeaponComponentHash.HeavySniperMk2Suppressor:
					case WeaponComponentHash.AtPiSupp02:
						return "WCT_SUPP";
					case WeaponComponentHash.PistolClip01:
					case WeaponComponentHash.CombatPistolClip01:
					case WeaponComponentHash.APPistolClip01:
					case WeaponComponentHash.MicroSMGClip01:
					case WeaponComponentHash.AssaultRifleClip01:
					case WeaponComponentHash.CarbineRifleClip01:
					case WeaponComponentHash.AdvancedRifleClip01:
					case WeaponComponentHash.MGClip01:
					case WeaponComponentHash.CombatMGClip01:
					case WeaponComponentHash.AssaultShotgunClip01:
					case WeaponComponentHash.SniperRifleClip01:
					case WeaponComponentHash.HeavySniperClip01:
					case WeaponComponentHash.AssaultSMGClip01:
					case WeaponComponentHash.Pistol50Clip01:
					case (WeaponComponentHash)0x0BAAB157:
					case (WeaponComponentHash)0x5AF49386:
					case (WeaponComponentHash)0xCAEBD246:
					case (WeaponComponentHash)0xF8955D89:
					case WeaponComponentHash.SNSPistolClip01:
					case WeaponComponentHash.VintagePistolClip01:
					case WeaponComponentHash.HeavyShotgunClip01:
					case WeaponComponentHash.MarksmanRifleClip01:
					case WeaponComponentHash.CombatPDWClip01:
					case WeaponComponentHash.MarksmanPistolClip01:
					case WeaponComponentHash.MachinePistolClip01:
					case WeaponComponentHash.PistolMk2ClipNormal:
					case WeaponComponentHash.AssaultRifleMk2ClipNormal:
					case WeaponComponentHash.CarbineRifleMk2ClipNormal:
					case WeaponComponentHash.CombatMGMk2ClipNormal:
					case WeaponComponentHash.HeavySniperMk2ClipNormal:
					case WeaponComponentHash.SMGMk2ClipNormal:
						return "WCT_CLIP1";
					case WeaponComponentHash.PistolClip02:
					case WeaponComponentHash.CombatPistolClip02:
					case WeaponComponentHash.APPistolClip02:
					case WeaponComponentHash.MicroSMGClip02:
					case WeaponComponentHash.SMGClip02:
					case WeaponComponentHash.AssaultRifleClip02:
					case WeaponComponentHash.CarbineRifleClip02:
					case WeaponComponentHash.AdvancedRifleClip02:
					case WeaponComponentHash.MGClip02:
					case WeaponComponentHash.CombatMGClip02:
					case WeaponComponentHash.AssaultShotgunClip02:
					case WeaponComponentHash.MinigunClip01:
					case WeaponComponentHash.AssaultSMGClip02:
					case WeaponComponentHash.Pistol50Clip02:
					case (WeaponComponentHash)0x6CBF371B:
					case (WeaponComponentHash)0xE1C5FFFA:
					case (WeaponComponentHash)0x3E7E6956:
					case WeaponComponentHash.SNSPistolClip02:
					case WeaponComponentHash.VintagePistolClip02:
					case WeaponComponentHash.HeavyShotgunClip02:
					case WeaponComponentHash.MarksmanRifleClip02:
					case WeaponComponentHash.CombatPDWClip02:
					case WeaponComponentHash.MachinePistolClip02:
					case WeaponComponentHash.PistolMk2ClipExtended:
					case WeaponComponentHash.AssaultRifleMk2ClipExtended:
					case WeaponComponentHash.CarbineRifleMk2ClipExtended:
					case WeaponComponentHash.CombatMGMk2ClipExtended:
					case WeaponComponentHash.HeavySniperMk2ClipExtended:
					case WeaponComponentHash.SMGMk2ClipExtended:
						return "WCT_CLIP2";
					case WeaponComponentHash.AtScopeLargeFixedZoom:
						return "WCT_SCOPE_LRG";
					case WeaponComponentHash.AssaultRifleVarmodLuxe:
					case WeaponComponentHash.CarbineRifleVarmodLuxe:
					case WeaponComponentHash.PistolVarmodLuxe:
					case WeaponComponentHash.SMGVarmodLuxe:
					case WeaponComponentHash.MicroSMGVarmodLuxe:
					case (WeaponComponentHash)0x161E9241:
					case WeaponComponentHash.AssaultSMGVarmodLowrider:
					case WeaponComponentHash.CombatPistolVarmodLowrider:
					case WeaponComponentHash.MGVarmodLowrider:
					case WeaponComponentHash.PumpShotgunVarmodLowrider:
						return "WCT_VAR_GOLD";
					case WeaponComponentHash.AdvancedRifleVarmodLuxe:
					case WeaponComponentHash.APPistolVarmodLuxe:
					case WeaponComponentHash.SawnoffShotgunVarmodLuxe:
					case WeaponComponentHash.BullpupRifleVarmodLow:
						return "WCT_VAR_METAL";
					case WeaponComponentHash.Pistol50VarmodLuxe:
						return "WCT_VAR_SIL";
					case WeaponComponentHash.HeavyPistolVarmodLuxe:
					case WeaponComponentHash.SniperRifleVarmodLuxe:
					case WeaponComponentHash.SNSPistolVarmodLowrider:
						return "WCT_VAR_WOOD";
					case WeaponComponentHash.CombatMGVarmodLowrider:
					case WeaponComponentHash.SpecialCarbineVarmodLowrider:
						return "WCT_VAR_ETCHM";
					case WeaponComponentHash.SwitchbladeVarmodBase:
						return "WCT_SB_BASE";
					case WeaponComponentHash.SwitchbladeVarmodVar1:
						return "WCT_SB_VAR1";
					case WeaponComponentHash.SwitchbladeVarmodVar2:
						return "WCT_SB_VAR2";
					case WeaponComponentHash.RevolverClip01:
						return "WCT_CLIP1";
					case WeaponComponentHash.RevolverVarmodBoss:
						return "WCT_REV_VARB";
					case WeaponComponentHash.RevolverVarmodGoon:
						return "WCT_REV_VARG";
					case WeaponComponentHash.SMGClip03:
					case WeaponComponentHash.AssaultRifleClip03:
					case WeaponComponentHash.HeavyShotgunClip03:
						return "WCT_CLIP_DRM";
					case WeaponComponentHash.CarbineRifleClip03:
						return "WCT_CLIP_BOX";
					case WeaponComponentHash.AssaultRifleMk2ClipArmorPiercing:
					case WeaponComponentHash.CarbineRifleMk2ClipArmorPiercing:
					case WeaponComponentHash.CombatMGMk2ClipArmorPiercing:
					case WeaponComponentHash.HeavySniperMk2ClipArmorPiercing:
						return "WCT_CLIP_AP";
					case WeaponComponentHash.PistolMk2ClipFMJ:
					case WeaponComponentHash.AssaultRifleMk2ClipFMJ:
					case WeaponComponentHash.CarbineRifleMk2ClipFMJ:
					case WeaponComponentHash.CombatMGMk2ClipFMJ:
					case WeaponComponentHash.HeavySniperMk2ClipFMJ:
					case WeaponComponentHash.SMGMk2ClipFMJ:
						return "WCT_CLIP_FMJ";
					case WeaponComponentHash.PistolMk2ClipIncendiary:
					case WeaponComponentHash.AssaultRifleMk2ClipIncendiary:
					case WeaponComponentHash.CarbineRifleMk2ClipIncendiary:
					case WeaponComponentHash.CombatMGMk2ClipIncendiary:
					case WeaponComponentHash.HeavySniperMk2ClipIncendiary:
					case WeaponComponentHash.SMGMk2ClipIncendiary:
						return "WCT_CLIP_INC";
					case WeaponComponentHash.PistolMk2ClipTracer:
					case WeaponComponentHash.AssaultRifleMk2ClipTracer:
					case WeaponComponentHash.CarbineRifleMk2ClipTracer:
					case WeaponComponentHash.CombatMGMk2ClipTracer:
					case WeaponComponentHash.SMGMk2ClipTracer:
						return "WCT_CLIP_TR";
					case WeaponComponentHash.HeavySniperMk2ClipExplosive:
						return "WCT_CLIP_EX";
					case WeaponComponentHash.PistolMk2ClipHollowpoint:
					case WeaponComponentHash.SMGMk2ClipHollowpoint:
						return "WCT_CLIP_HP";
					case WeaponComponentHash.AssaultRifleMk2BarrelNormal:
					case WeaponComponentHash.CarbineRifleMk2BarrelNormal:
					case WeaponComponentHash.CombatMGMk2BarrelNormal:
					case WeaponComponentHash.SMGMk2BarrelNormal:
						return "WCT_BARR";
					case WeaponComponentHash.AssaultRifleMk2BarrelHeavy:
					case WeaponComponentHash.CarbineRifleMk2BarrelHeavy:
					case WeaponComponentHash.CombatMGMk2BarrelHeavy:
					case WeaponComponentHash.SMGMk2BarrelHeavy:
						return "WCT_BARR2";
					case WeaponComponentHash.PistolMk2CamoDigital:
					case WeaponComponentHash.AssaultRifleMk2CamoDigital:
					case WeaponComponentHash.CarbineRifleMk2CamoDigital:
					case WeaponComponentHash.CombatMGMk2CamoDigital:
					case WeaponComponentHash.HeavySniperMk2CamoDigital:
					case WeaponComponentHash.SMGMk2CamoDigital:
					case WeaponComponentHash.PistolMk2CamoSlideDigital:
						return "WCT_CAMO_1";
					case WeaponComponentHash.PistolMk2CamoBrushstroke:
					case WeaponComponentHash.AssaultRifleMk2CamoBrushstroke:
					case WeaponComponentHash.CarbineRifleMk2CamoBrushstroke:
					case WeaponComponentHash.CombatMGMk2CamoBrushstroke:
					case WeaponComponentHash.HeavySniperMk2CamoBrushstroke:
					case WeaponComponentHash.SMGMk2CamoBrushstroke:
					case WeaponComponentHash.PistolMk2CamoSlideBrushstroke:
						return "WCT_CAMO_2";
					case WeaponComponentHash.PistolMk2CamoWoodland:
					case WeaponComponentHash.AssaultRifleMk2CamoWoodland:
					case WeaponComponentHash.CarbineRifleMk2CamoWoodland:
					case WeaponComponentHash.CombatMGMk2CamoWoodland:
					case WeaponComponentHash.HeavySniperMk2CamoWoodland:
					case WeaponComponentHash.SMGMk2CamoWoodland:
					case WeaponComponentHash.PistolMk2CamoSlideWoodland:
						return "WCT_CAMO_3";
					case WeaponComponentHash.PistolMk2CamoSkull:
					case WeaponComponentHash.AssaultRifleMk2CamoSkull:
					case WeaponComponentHash.CarbineRifleMk2CamoSkull:
					case WeaponComponentHash.CombatMGMk2CamoSkull:
					case WeaponComponentHash.HeavySniperMk2CamoSkull:
					case WeaponComponentHash.SMGMk2CamoSkull:
					case WeaponComponentHash.PistolMk2CamoSlideSkull:
						return "WCT_CAMO_4";
					case WeaponComponentHash.PistolMk2CamoSessanta:
					case WeaponComponentHash.AssaultRifleMk2CamoSessanta:
					case WeaponComponentHash.CarbineRifleMk2CamoSessanta:
					case WeaponComponentHash.CombatMGMk2CamoSessanta:
					case WeaponComponentHash.HeavySniperMk2CamoSessanta:
					case WeaponComponentHash.SMGMk2CamoSessanta:
					case WeaponComponentHash.PistolMk2CamoSlideSessanta:
						return "WCT_CAMO_5";
					case WeaponComponentHash.PistolMk2CamoPerseus:
					case WeaponComponentHash.AssaultRifleMk2CamoPerseus:
					case WeaponComponentHash.CarbineRifleMk2CamoPerseus:
					case WeaponComponentHash.CombatMGMk2CamoPerseus:
					case WeaponComponentHash.HeavySniperMk2CamoPerseus:
					case WeaponComponentHash.SMGMk2CamoPerseus:
					case WeaponComponentHash.PistolMk2CamoSlidePerseus:
						return "WCT_CAMO_6";
					case WeaponComponentHash.PistolMk2CamoLeopard:
					case WeaponComponentHash.AssaultRifleMk2CamoLeopard:
					case WeaponComponentHash.CarbineRifleMk2CamoLeopard:
					case WeaponComponentHash.CombatMGMk2CamoLeopard:
					case WeaponComponentHash.HeavySniperMk2CamoLeopard:
					case WeaponComponentHash.SMGMk2CamoLeopard:
					case WeaponComponentHash.PistolMk2CamoSlideLeopard:
						return "WCT_CAMO_7";
					case WeaponComponentHash.PistolMk2CamoZebra:
					case WeaponComponentHash.AssaultRifleMk2CamoZebra:
					case WeaponComponentHash.CarbineRifleMk2CamoZebra:
					case WeaponComponentHash.CombatMGMk2CamoZebra:
					case WeaponComponentHash.HeavySniperMk2CamoZebra:
					case WeaponComponentHash.SMGMk2CamoZebra:
					case WeaponComponentHash.PistolMk2CamoSlideZebra:
						return "WCT_CAMO_8";
					case WeaponComponentHash.PistolMk2CamoGeometric:
					case WeaponComponentHash.AssaultRifleMk2CamoGeometric:
					case WeaponComponentHash.CarbineRifleMk2CamoGeometric:
					case WeaponComponentHash.CombatMGMk2CamoGeometric:
					case WeaponComponentHash.HeavySniperMk2CamoGeometric:
					case WeaponComponentHash.SMGMk2CamoGeometric:
					case WeaponComponentHash.PistolMk2CamoSlideGeometric:
						return "WCT_CAMO_9";
					case WeaponComponentHash.PistolMk2CamoBoom:
					case WeaponComponentHash.AssaultRifleMk2CamoBoom:
					case WeaponComponentHash.CarbineRifleMk2CamoBoom:
					case WeaponComponentHash.CombatMGMk2CamoBoom:
					case WeaponComponentHash.HeavySniperMk2CamoBoom:
					case WeaponComponentHash.SMGMk2CamoBoom:
					case WeaponComponentHash.PistolMk2CamoSlideBoom:
						return "WCT_CAMO_10";
					case WeaponComponentHash.PistolMk2CamoPatriotic:
					case WeaponComponentHash.AssaultRifleMk2CamoPatriotic:
					case WeaponComponentHash.CarbineRifleMk2CamoPatriotic:
					case WeaponComponentHash.CombatMGMk2CamoPatriotic:
					case WeaponComponentHash.HeavySniperMk2CamoPatriotic:
					case WeaponComponentHash.SMGMk2CamoPatriotic:
					case WeaponComponentHash.PistolMk2CamoSlidePatriotic:
						return "WCT_CAMO_IND";
					case WeaponComponentHash.AtSights:
						return "WCT_HOLO";
					case WeaponComponentHash.AtScopeSmallMk2:
						return "WCT_SCOPE_SML2";
					case WeaponComponentHash.AtScopeMacroMk2:
						return "WCT_SCOPE_MAC2";
					case WeaponComponentHash.AtScopeMediumMk2:
						return "WCT_SCOPE_MED2";
					case WeaponComponentHash.AtMuzzle1:
					case WeaponComponentHash.AtMuzzle2:
					case WeaponComponentHash.AtMuzzle3:
					case WeaponComponentHash.AtMuzzle4:
					case WeaponComponentHash.AtMuzzle5:
					case WeaponComponentHash.AtMuzzle7:
					case WeaponComponentHash.HeavySniperMk2Muzzle8:
					case WeaponComponentHash.HeavySniperMk2Muzzle9:
						return "WCT_MUZZ";
					case WeaponComponentHash.PistolMk2Scope:
						return "WCT_SCOPE_PI";
					case WeaponComponentHash.PistolMk2Compensator:
						return "WCT_COMP";
					case WeaponComponentHash.HeavySniperMk2ScopeLarge:
						return "WCT_SCOPE_LRG2";
				}
			}
			string result = "WCT_INVALID";

			return result;
		}

		[SecuritySafeCritical]
		public static ComponentAttachmentPoint GetAttachmentPoint(WeaponHash hash, WeaponComponentHash componentHash)
		{
			return _GetAttachmentPoint(hash, componentHash);
		}

		[SecurityCritical]
		private static ComponentAttachmentPoint _GetAttachmentPoint(WeaponHash hash, WeaponComponentHash componentHash)
		{
			switch (hash)
			{
				case WeaponHash.Pistol:
					switch (componentHash)
					{
						case WeaponComponentHash.PistolClip01:
						case WeaponComponentHash.PistolClip02:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtPiFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtPiSupp02:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.PistolVarmodLuxe:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.CombatPistol:
					switch (componentHash)
					{
						case WeaponComponentHash.CombatPistolClip01:
						case WeaponComponentHash.CombatPistolClip02:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtPiFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtPiSupp:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.CombatPistolVarmodLowrider:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.APPistol:
					switch (componentHash)
					{
						case WeaponComponentHash.APPistolClip01:
						case WeaponComponentHash.APPistolClip02:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtPiFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtPiSupp:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.APPistolVarmodLuxe:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.MicroSMG:
					switch (componentHash)
					{
						case WeaponComponentHash.MicroSMGClip01:
						case WeaponComponentHash.MicroSMGClip02:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtPiFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtScopeMacro:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.AtArSupp02:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.MicroSMGVarmodLuxe:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.SMG:
					switch (componentHash)
					{
						case WeaponComponentHash.SMGClip01:
						case WeaponComponentHash.SMGClip02:
						case WeaponComponentHash.SMGClip03:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtArFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtScopeMacro02:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.AtPiSupp:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.SMGVarmodLuxe:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.AssaultRifle:
					switch (componentHash)
					{
						case WeaponComponentHash.AssaultRifleClip01:
						case WeaponComponentHash.AssaultRifleClip02:
						case WeaponComponentHash.AssaultRifleClip03:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtArAfGrip:
							return ComponentAttachmentPoint.Grip;

						case WeaponComponentHash.AtArFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtScopeMacro:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.AtArSupp02:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.AssaultRifleVarmodLuxe:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.CarbineRifle:
					switch (componentHash)
					{
						case WeaponComponentHash.CarbineRifleClip01:
						case WeaponComponentHash.CarbineRifleClip02:
						case WeaponComponentHash.CarbineRifleClip03:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtRailCover01:
							return ComponentAttachmentPoint.Rail;

						case WeaponComponentHash.AtArAfGrip:
							return ComponentAttachmentPoint.Grip;

						case WeaponComponentHash.AtArFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtScopeMedium:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.AtArSupp:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.CarbineRifleVarmodLuxe:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.AdvancedRifle:
					switch (componentHash)
					{
						case WeaponComponentHash.AdvancedRifleClip01:
						case WeaponComponentHash.AdvancedRifleClip02:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtArFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtScopeSmall:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.AtArSupp:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.AdvancedRifleVarmodLuxe:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}

				case WeaponHash.MG:
					switch (componentHash)
					{
						case WeaponComponentHash.MGClip01:
						case WeaponComponentHash.MGClip02:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtScopeSmall02:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.MGVarmodLowrider:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.CombatMG:
					switch (componentHash)
					{
						case WeaponComponentHash.CombatMGClip01:
						case WeaponComponentHash.CombatMGClip02:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtArAfGrip:
							return ComponentAttachmentPoint.Grip;

						case WeaponComponentHash.AtScopeMedium:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.CombatMGVarmodLowrider:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.PumpShotgun:
					switch (componentHash)
					{
						case WeaponComponentHash.PumpShotgunClip01:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtArFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtArSupp:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.PumpShotgunVarmodLowrider:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.AssaultShotgun:
					switch (componentHash)
					{
						case WeaponComponentHash.AssaultShotgunClip01:
						case WeaponComponentHash.AssaultShotgunClip02:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtArAfGrip:
							return ComponentAttachmentPoint.Grip;

						case WeaponComponentHash.AtArFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtArSupp:
							return ComponentAttachmentPoint.Supp;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.SniperRifle:
					switch (componentHash)
					{
						case WeaponComponentHash.SniperRifleClip01:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtArSupp02:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.AtScopeLarge:
						case WeaponComponentHash.AtScopeMax:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.SniperRifleVarmodLuxe:
							return ComponentAttachmentPoint.GunRoot;
						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.HeavySniper:
					switch (componentHash)
					{
						case WeaponComponentHash.HeavySniperClip01:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtScopeLarge:
						case WeaponComponentHash.AtScopeMax:
							return ComponentAttachmentPoint.Scope;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.GrenadeLauncher:
					switch (componentHash)
					{
						case WeaponComponentHash.GrenadeLauncherClip01:
							return ComponentAttachmentPoint.Grip;

						case WeaponComponentHash.AtArAfGrip:
							return ComponentAttachmentPoint.Grip;

						case WeaponComponentHash.AtArFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtScopeSmall:
							return ComponentAttachmentPoint.Scope;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.Minigun:
					switch (componentHash)
					{
						case WeaponComponentHash.MinigunClip01:
							return ComponentAttachmentPoint.Clip;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.AssaultSMG:
					switch (componentHash)
					{
						case WeaponComponentHash.AssaultSMGClip01:
						case WeaponComponentHash.AssaultSMGClip02:
							return ComponentAttachmentPoint.Clip;


						case WeaponComponentHash.AtArFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtScopeMacro:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.AtArSupp02:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.AssaultSMGVarmodLowrider:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.BullpupShotgun:
					switch (componentHash)
					{
						case WeaponComponentHash.BullpupShotgunClip01:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtArAfGrip:
							return ComponentAttachmentPoint.Grip;

						case WeaponComponentHash.AtArFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtArSupp02:
							return ComponentAttachmentPoint.Supp;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.Pistol50:
					switch (componentHash)
					{
						case WeaponComponentHash.Pistol50Clip01:
						case WeaponComponentHash.Pistol50Clip02:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtPiFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtArSupp02:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.Pistol50VarmodLuxe:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.SawnOffShotgun:
					switch (componentHash)
					{
						case WeaponComponentHash.SawnoffShotgunClip01:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.SawnoffShotgunVarmodLuxe:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.PistolMk2:
					switch (componentHash)
					{
						case WeaponComponentHash.PistolMk2ClipNormal:
						case WeaponComponentHash.PistolMk2ClipExtended:
						case WeaponComponentHash.PistolMk2ClipFMJ:
						case WeaponComponentHash.PistolMk2ClipHollowpoint:
						case WeaponComponentHash.PistolMk2ClipIncendiary:
						case WeaponComponentHash.PistolMk2ClipTracer:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.PistolMk2Scope:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.PistolMk2Flash:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.PistolMk2Compensator:
						case WeaponComponentHash.AtPiSupp02:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.PistolMk2CamoDigital:
						case WeaponComponentHash.PistolMk2CamoBrushstroke:
						case WeaponComponentHash.PistolMk2CamoWoodland:
						case WeaponComponentHash.PistolMk2CamoSkull:
						case WeaponComponentHash.PistolMk2CamoSessanta:
						case WeaponComponentHash.PistolMk2CamoPerseus:
						case WeaponComponentHash.PistolMk2CamoLeopard:
						case WeaponComponentHash.PistolMk2CamoZebra:
						case WeaponComponentHash.PistolMk2CamoGeometric:
						case WeaponComponentHash.PistolMk2CamoBoom:
						case WeaponComponentHash.PistolMk2CamoPatriotic:
							return ComponentAttachmentPoint.GunRoot;

						case WeaponComponentHash.PistolMk2CamoSlideDigital:
						case WeaponComponentHash.PistolMk2CamoSlideBrushstroke:
						case WeaponComponentHash.PistolMk2CamoSlideWoodland:
						case WeaponComponentHash.PistolMk2CamoSlideSkull:
						case WeaponComponentHash.PistolMk2CamoSlideSessanta:
						case WeaponComponentHash.PistolMk2CamoSlidePerseus:
						case WeaponComponentHash.PistolMk2CamoSlideLeopard:
						case WeaponComponentHash.PistolMk2CamoSlideZebra:
						case WeaponComponentHash.PistolMk2CamoSlideGeometric:
						case WeaponComponentHash.PistolMk2CamoSlideBoom:
						case WeaponComponentHash.PistolMk2CamoSlidePatriotic:
							return ComponentAttachmentPoint.Scope2;

						default:
							return ComponentAttachmentPoint.Invalid;
					}
				case WeaponHash.AssaultRifleMk2:
					switch (componentHash)
					{
						case WeaponComponentHash.AssaultRifleMk2ClipNormal:
						case WeaponComponentHash.AssaultRifleMk2ClipExtended:
						case WeaponComponentHash.AssaultRifleMk2ClipFMJ:
						case WeaponComponentHash.AssaultRifleMk2ClipArmorPiercing:
						case WeaponComponentHash.AssaultRifleMk2ClipIncendiary:
						case WeaponComponentHash.AssaultRifleMk2ClipTracer:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtArFlsh:
							return ComponentAttachmentPoint.FlashLaser2;

						case WeaponComponentHash.AtSights:
						case WeaponComponentHash.AtScopeMacroMk2:
						case WeaponComponentHash.AtScopeMediumMk2:
							return ComponentAttachmentPoint.Scope2;

						case WeaponComponentHash.AtArSupp02:
						case WeaponComponentHash.AtMuzzle1:
						case WeaponComponentHash.AtMuzzle2:
						case WeaponComponentHash.AtMuzzle3:
						case WeaponComponentHash.AtMuzzle4:
						case WeaponComponentHash.AtMuzzle5:
						case WeaponComponentHash.AtMuzzle6:
						case WeaponComponentHash.AtMuzzle7:
							return ComponentAttachmentPoint.Supp2;

						case WeaponComponentHash.AtArAfGrip2:
							return ComponentAttachmentPoint.Grip;

						case WeaponComponentHash.AssaultRifleMk2BarrelNormal:
						case WeaponComponentHash.AssaultRifleMk2BarrelHeavy:
							return ComponentAttachmentPoint.Barrel;

						case WeaponComponentHash.AssaultRifleMk2CamoDigital:
						case WeaponComponentHash.AssaultRifleMk2CamoBrushstroke:
						case WeaponComponentHash.AssaultRifleMk2CamoWoodland:
						case WeaponComponentHash.AssaultRifleMk2CamoSkull:
						case WeaponComponentHash.AssaultRifleMk2CamoSessanta:
						case WeaponComponentHash.AssaultRifleMk2CamoPerseus:
						case WeaponComponentHash.AssaultRifleMk2CamoLeopard:
						case WeaponComponentHash.AssaultRifleMk2CamoZebra:
						case WeaponComponentHash.AssaultRifleMk2CamoGeometric:
						case WeaponComponentHash.AssaultRifleMk2CamoBoom:
						case WeaponComponentHash.AssaultRifleMk2CamoPatriotic:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}

				case WeaponHash.CarbineRifleMk2:
					switch (componentHash)
					{
						case WeaponComponentHash.CarbineRifleMk2ClipNormal:
						case WeaponComponentHash.CarbineRifleMk2ClipExtended:
						case WeaponComponentHash.CarbineRifleMk2ClipFMJ:
						case WeaponComponentHash.CarbineRifleMk2ClipArmorPiercing:
						case WeaponComponentHash.CarbineRifleMk2ClipIncendiary:
						case WeaponComponentHash.CarbineRifleMk2ClipTracer:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtSights:
						case WeaponComponentHash.AtScopeSmallMk2:
						case WeaponComponentHash.AtScopeMediumMk2:
							return ComponentAttachmentPoint.Scope2;

						case WeaponComponentHash.AtMuzzle1:
						case WeaponComponentHash.AtMuzzle2:
						case WeaponComponentHash.AtMuzzle3:
						case WeaponComponentHash.AtMuzzle4:
						case WeaponComponentHash.AtMuzzle5:
						case WeaponComponentHash.AtMuzzle6:
						case WeaponComponentHash.AtMuzzle7:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.AtArAfGrip2:
							return ComponentAttachmentPoint.Grip2;

						case WeaponComponentHash.CarbineRifleMk2BarrelNormal:
						case WeaponComponentHash.CarbineRifleMk2BarrelHeavy:
							return ComponentAttachmentPoint.Barrel;

						case WeaponComponentHash.CarbineRifleMk2CamoDigital:
						case WeaponComponentHash.CarbineRifleMk2CamoBrushstroke:
						case WeaponComponentHash.CarbineRifleMk2CamoWoodland:
						case WeaponComponentHash.CarbineRifleMk2CamoSkull:
						case WeaponComponentHash.CarbineRifleMk2CamoSessanta:
						case WeaponComponentHash.CarbineRifleMk2CamoPerseus:
						case WeaponComponentHash.CarbineRifleMk2CamoLeopard:
						case WeaponComponentHash.CarbineRifleMk2CamoZebra:
						case WeaponComponentHash.CarbineRifleMk2CamoGeometric:
						case WeaponComponentHash.CarbineRifleMk2CamoBoom:
						case WeaponComponentHash.CarbineRifleMk2CamoPatriotic:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}

				case WeaponHash.CombatMGMk2:
					switch (componentHash)
					{
						case WeaponComponentHash.CombatMGMk2ClipNormal:
						case WeaponComponentHash.CombatMGMk2ClipExtended:
						case WeaponComponentHash.CombatMGMk2ClipFMJ:
						case WeaponComponentHash.CombatMGMk2ClipArmorPiercing:
						case WeaponComponentHash.CombatMGMk2ClipIncendiary:
						case WeaponComponentHash.CombatMGMk2ClipTracer:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtArFlsh:
							return ComponentAttachmentPoint.FlashLaser;

						case WeaponComponentHash.AtSights:
						case WeaponComponentHash.AtScopeMacroMk2:
						case WeaponComponentHash.AtScopeMediumMk2:
							return ComponentAttachmentPoint.Scope2;

						case WeaponComponentHash.AtArSupp:
						case WeaponComponentHash.AtMuzzle1:
						case WeaponComponentHash.AtMuzzle2:
						case WeaponComponentHash.AtMuzzle3:
						case WeaponComponentHash.AtMuzzle4:
						case WeaponComponentHash.AtMuzzle5:
						case WeaponComponentHash.AtMuzzle6:
						case WeaponComponentHash.AtMuzzle7:
							return ComponentAttachmentPoint.Supp2;

						case WeaponComponentHash.AtArAfGrip2:
							return ComponentAttachmentPoint.Grip2;

						case WeaponComponentHash.CombatMGMk2BarrelNormal:
						case WeaponComponentHash.CombatMGMk2BarrelHeavy:
							return ComponentAttachmentPoint.Barrel;

						case WeaponComponentHash.CombatMGMk2CamoDigital:
						case WeaponComponentHash.CombatMGMk2CamoBrushstroke:
						case WeaponComponentHash.CombatMGMk2CamoWoodland:
						case WeaponComponentHash.CombatMGMk2CamoSkull:
						case WeaponComponentHash.CombatMGMk2CamoSessanta:
						case WeaponComponentHash.CombatMGMk2CamoPerseus:
						case WeaponComponentHash.CombatMGMk2CamoLeopard:
						case WeaponComponentHash.CombatMGMk2CamoZebra:
						case WeaponComponentHash.CombatMGMk2CamoGeometric:
						case WeaponComponentHash.CombatMGMk2CamoBoom:
						case WeaponComponentHash.CombatMGMk2CamoPatriotic:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}

				case WeaponHash.HeavySniperMk2:
					switch (componentHash)
					{
						case WeaponComponentHash.HeavySniperMk2ClipNormal:
						case WeaponComponentHash.HeavySniperMk2ClipExtended:
						case WeaponComponentHash.HeavySniperMk2ClipFMJ:
						case WeaponComponentHash.HeavySniperMk2ClipArmorPiercing:
						case WeaponComponentHash.HeavySniperMk2ClipIncendiary:
						case WeaponComponentHash.HeavySniperMk2ClipExplosive:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.HeavySniperMk2ScopeLarge:
						case WeaponComponentHash.AtScopeMax:
						case WeaponComponentHash.HeavySniperMk2ScopeNightvision:
						case WeaponComponentHash.HeavySniperMk2ScopeThermal:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.HeavySniperMk2Suppressor:
						case WeaponComponentHash.HeavySniperMk2Muzzle8:
						case WeaponComponentHash.HeavySniperMk2Muzzle9:
							return ComponentAttachmentPoint.Supp;

						case WeaponComponentHash.HeavySniperMk2BarrelNormal:
						case WeaponComponentHash.HeavySniperMk2BarrelHeavy:
							return ComponentAttachmentPoint.Barrel;

						case WeaponComponentHash.HeavySniperMk2CamoDigital:
						case WeaponComponentHash.HeavySniperMk2CamoBrushstroke:
						case WeaponComponentHash.HeavySniperMk2CamoWoodland:
						case WeaponComponentHash.HeavySniperMk2CamoSkull:
						case WeaponComponentHash.HeavySniperMk2CamoSessanta:
						case WeaponComponentHash.HeavySniperMk2CamoPerseus:
						case WeaponComponentHash.HeavySniperMk2CamoLeopard:
						case WeaponComponentHash.HeavySniperMk2CamoZebra:
						case WeaponComponentHash.HeavySniperMk2CamoGeometric:
						case WeaponComponentHash.HeavySniperMk2CamoBoom:
						case WeaponComponentHash.HeavySniperMk2CamoPatriotic:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}

				case WeaponHash.SMGMk2:
					switch (componentHash)
					{
						case WeaponComponentHash.SMGMk2ClipNormal:
						case WeaponComponentHash.SMGMk2ClipExtended:
						case WeaponComponentHash.SMGMk2ClipFMJ:
						case WeaponComponentHash.SMGMk2ClipHollowpoint:
						case WeaponComponentHash.SMGMk2ClipIncendiary:
						case WeaponComponentHash.SMGMk2ClipTracer:
							return ComponentAttachmentPoint.Clip;

						case WeaponComponentHash.AtArFlsh:
							return ComponentAttachmentPoint.FlashLaser2;

						case WeaponComponentHash.SMGMk2Sights:
						case WeaponComponentHash.SMGMk2ScopeMacro:
						case WeaponComponentHash.SMGMk2ScopeSmall:
							return ComponentAttachmentPoint.Scope;

						case WeaponComponentHash.AtPiSupp:
						case WeaponComponentHash.AtMuzzle1:
						case WeaponComponentHash.AtMuzzle2:
						case WeaponComponentHash.AtMuzzle3:
						case WeaponComponentHash.AtMuzzle4:
						case WeaponComponentHash.AtMuzzle5:
						case WeaponComponentHash.AtMuzzle6:
						case WeaponComponentHash.AtMuzzle7:
							return ComponentAttachmentPoint.Supp2;

						case WeaponComponentHash.AtArAfGrip2:
							return ComponentAttachmentPoint.Grip2;

						case WeaponComponentHash.SMGMk2BarrelNormal:
						case WeaponComponentHash.SMGMk2BarrelHeavy:
							return ComponentAttachmentPoint.Barrel;

						case WeaponComponentHash.SMGMk2CamoDigital:
						case WeaponComponentHash.SMGMk2CamoBrushstroke:
						case WeaponComponentHash.SMGMk2CamoWoodland:
						case WeaponComponentHash.SMGMk2CamoSkull:
						case WeaponComponentHash.SMGMk2CamoSessanta:
						case WeaponComponentHash.SMGMk2CamoPerseus:
						case WeaponComponentHash.SMGMk2CamoLeopard:
						case WeaponComponentHash.SMGMk2CamoZebra:
						case WeaponComponentHash.SMGMk2CamoGeometric:
						case WeaponComponentHash.SMGMk2CamoBoom:
						case WeaponComponentHash.SMGMk2CamoPatriotic:
							return ComponentAttachmentPoint.GunRoot;

						default:
							return ComponentAttachmentPoint.Invalid;
					}

			}

			return ComponentAttachmentPoint.Invalid;
		}
	}

	public class InvalidWeaponComponent : WeaponComponent
	{
		internal InvalidWeaponComponent() : base(null, null, WeaponComponentHash.Invalid)
		{
		}

		public override bool Active
		{
			set
			{
				//Setter doesn't need to do anything for the invalid component
			}
		}

		public override string DisplayName
		{
			get
			{
				return "WCT_INVALID";
			}
		}

		public override ComponentAttachmentPoint AttachmentPoint
		{
			get { return ComponentAttachmentPoint.Invalid; }
		}
	}
}
