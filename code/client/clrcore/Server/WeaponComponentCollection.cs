using System.Collections.Generic;
using System.Linq;
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

		//mpgunrunning
		#region Common
		AtSights = 0x420FD713,
		AtScopeSmallMk2 = 0x3F3C8181,
		AtScopeMacroMk2 = 0x49B2945,
		AtScopeMediumMk2 = 0xC66B6542,
		AtMuzzle1 = 0xB99402D4,
		AtMuzzle2 = 0xC867A07B,
		AtMuzzle3 = 0xDE11CBCF,
		AtMuzzle4 = 0xEC9068CC,
		AtMuzzle5 = 0x2E7957A,
		AtMuzzle6 = 0x347EF8AC,
		AtMuzzle7 = 0x4DB62ABE,
		AtArAfGrip2 = 0x9D65907A,
		#endregion

		#region PistolMk2
		PistolMk2ClipNormal = 0x94F42D62,
		PistolMk2ClipExtended = 0x5ED6C128,
		PistolMk2ClipFMJ = 0x4F37DF2A,
		PistolMk2ClipHollowpoint = 0x85FEA109,
		PistolMk2ClipIncendiary = 0x2BBD7A3A,
		PistolMk2ClipTracer = 0x25CAAEAF,

		PistolMk2Scope = 0x8ED4BB70,
		PistolMk2Flash = 0x43FD595B,

		PistolMk2Compensator = 0x21E34793,

		PistolMk2CamoDigital = 0x5C6C749C,
		PistolMk2CamoBrushstroke = 0x15F7A390,
		PistolMk2CamoWoodland = 0x968E24DB,
		PistolMk2CamoSkull = 0x17BFA99,
		PistolMk2CamoSessanta = 0xF2685C72,
		PistolMk2CamoPerseus = 0xDD2231E6,
		PistolMk2CamoLeopard = 0xBB43EE76,
		PistolMk2CamoZebra = 0x4D901310,
		PistolMk2CamoGeometric = 0x5F31B653,
		PistolMk2CamoBoom = 0x697E19A0,
		PistolMk2CamoPatriotic = 0x930CB951,
		PistolMk2CamoSlideDigital = 0xB4FC92B0,
		PistolMk2CamoSlideBrushstroke = 0x1A1F1260,
		PistolMk2CamoSlideWoodland = 0xE4E00B70,
		PistolMk2CamoSlideSkull = 0x2C298B2B,
		PistolMk2CamoSlideSessanta = 0xDFB79725,
		PistolMk2CamoSlidePerseus = 0x6BD7228C,
		PistolMk2CamoSlideLeopard = 0x9DDBCF8C,
		PistolMk2CamoSlideZebra = 0xB319A52C,
		PistolMk2CamoSlideGeometric = 0xC6836E12,
		PistolMk2CamoSlideBoom = 0x43B1B173,
		PistolMk2CamoSlidePatriotic = 0x4ABDA3FA,
		#endregion

		#region AssaultRifleMk2
		AssaultRifleMk2ClipNormal = 0x8610343F,
		AssaultRifleMk2ClipExtended = 0xD12ACA6F,
		AssaultRifleMk2ClipArmorPiercing = 0xA7DD1E58,
		AssaultRifleMk2ClipFMJ = 0x63E0A098,
		AssaultRifleMk2ClipIncendiary = 0xFB70D853,
		AssaultRifleMk2ClipTracer = 0xEF2C78C1,

		AssaultRifleMk2BarrelNormal = 0x43A49D26,
		AssaultRifleMk2BarrelHeavy = 0x5646C26A,

		AssaultRifleMk2CamoDigital = 0x911B24AF,
		AssaultRifleMk2CamoBrushstroke = 0x37E5444B,
		AssaultRifleMk2CamoWoodland = 0x538B7B97,
		AssaultRifleMk2CamoSkull = 0x25789F72,
		AssaultRifleMk2CamoSessanta = 0xC5495F2D,
		AssaultRifleMk2CamoPerseus = 0xCF8B73B1,
		AssaultRifleMk2CamoLeopard = 0xA9BB2811,
		AssaultRifleMk2CamoZebra = 0xFC674D54,
		AssaultRifleMk2CamoGeometric = 0x7C7FCD9B,
		AssaultRifleMk2CamoBoom = 0xA5C38392,
		AssaultRifleMk2CamoPatriotic = 0xB9B15DB0,
		#endregion

		#region CarbineRifleMk2
		CarbineRifleMk2ClipNormal = 0x4C7A391E,
		CarbineRifleMk2ClipExtended = 0x5DD5DBD5,
		CarbineRifleMk2ClipArmorPiercing = 0x255D5D57,
		CarbineRifleMk2ClipFMJ = 0x44032F11,
		CarbineRifleMk2ClipIncendiary = 0x3D25C2A7,
		CarbineRifleMk2ClipTracer = 0x1757F566,

		CarbineRifleMk2BarrelNormal = 0x833637FF,
		CarbineRifleMk2BarrelHeavy = 0x8B3C480B,

		CarbineRifleMk2CamoDigital = 0x4BDD6F16,
		CarbineRifleMk2CamoBrushstroke = 0x406A7908,
		CarbineRifleMk2CamoWoodland = 0x2F3856A4,
		CarbineRifleMk2CamoSkull = 0xE50C424D,
		CarbineRifleMk2CamoSessanta = 0xD37D1F2F,
		CarbineRifleMk2CamoPerseus = 0x86268483,
		CarbineRifleMk2CamoLeopard = 0xF420E076,
		CarbineRifleMk2CamoZebra = 0xAAE14DF8,
		CarbineRifleMk2CamoGeometric = 0x9893A95D,
		CarbineRifleMk2CamoBoom = 0x6B13CD3E,
		CarbineRifleMk2CamoPatriotic = 0xDA55CD3F,
		#endregion

		#region CombatMGMk2
		CombatMGMk2ClipNormal = 0x492B257C,
		CombatMGMk2ClipExtended = 0x17DF42E9,
		CombatMGMk2ClipArmorPiercing = 0x29882423,
		CombatMGMk2ClipFMJ = 0x57EF1CC8,
		CombatMGMk2ClipIncendiary = 0xC326BDBA,
		CombatMGMk2ClipTracer = 0xF6649745,

		CombatMGMk2BarrelNormal = 0xC34EF234,
		CombatMGMk2BarrelHeavy = 0xB5E2575B,

		CombatMGMk2CamoDigital = 0x4A768CB5,
		CombatMGMk2CamoBrushstroke = 0xCCE06BBD,
		CombatMGMk2CamoWoodland = 0xBE94CF26,
		CombatMGMk2CamoSkull = 0x7609BE11,
		CombatMGMk2CamoSessanta = 0x48AF6351,
		CombatMGMk2CamoPerseus = 0x9186750A,
		CombatMGMk2CamoLeopard = 0x84555AA8,
		CombatMGMk2CamoZebra = 0x1B4C088B,
		CombatMGMk2CamoGeometric = 0xE046DFC,
		CombatMGMk2CamoBoom = 0x28B536E,
		CombatMGMk2CamoPatriotic = 0xD703C94D,
		#endregion

		#region HeavySniperMk2
		HeavySniperMk2ClipNormal = 0xFA1E1A28,
		HeavySniperMk2ClipExtended = 0x2CD8FF9D,
		HeavySniperMk2ClipArmorPiercing = 0xF835D6D4,
		HeavySniperMk2ClipExplosive = 0x89EBDAA7,
		HeavySniperMk2ClipFMJ = 0x3BE948F6,
		HeavySniperMk2ClipIncendiary = 0xEC0F617,

		HeavySniperMk2ScopeLarge = 0x82C10383,
		HeavySniperMk2ScopeNightvision = 0xB68010B0,
		HeavySniperMk2ScopeThermal = 0x2E43DA41,

		HeavySniperMk2Suppressor = 0xAC42DF71,
		HeavySniperMk2Muzzle8 = 0x5F7DCE4D,
		HeavySniperMk2Muzzle9 = 0x6927E1A1,

		HeavySniperMk2BarrelNormal = 0x909630B7,
		HeavySniperMk2BarrelHeavy = 0x108AB09E,

		HeavySniperMk2CamoDigital = 0xF8337D02,
		HeavySniperMk2CamoBrushstroke = 0xC5BEDD65,
		HeavySniperMk2CamoWoodland = 0xE9712475,
		HeavySniperMk2CamoSkull = 0x13AA78E7,
		HeavySniperMk2CamoSessanta = 0x26591E50,
		HeavySniperMk2CamoPerseus = 0x302731EC,
		HeavySniperMk2CamoLeopard = 0xAC722A78,
		HeavySniperMk2CamoZebra = 0xBEA4CEDD,
		HeavySniperMk2CamoGeometric = 0xCD776C82,
		HeavySniperMk2CamoBoom = 0xABC5ACC7,
		HeavySniperMk2CamoPatriotic = 0x6C32D2EB,
		#endregion

		#region SMGMk2
		SMGMk2ClipNormal = 0x4C24806E,
		SMGMk2ClipExtended = 0xB9835B2E,
		SMGMk2ClipFMJ = 0xB5A715F,
		SMGMk2ClipHollowpoint = 0x3A1BD6FA,
		SMGMk2ClipIncendiary = 0xD99222E5,
		SMGMk2ClipTracer = 0x7FEA36EC,

		SMGMk2Sights = 0x9FDB5652,
		SMGMk2ScopeMacro = 0xE502AB6B,
		SMGMk2ScopeSmall = 0x3DECC7DA,

		SMGMk2BarrelNormal = 0xD9103EE1,
		SMGMk2BarrelHeavy = 0xA564D78B,

		SMGMk2CamoDigital = 0xC4979067,
		SMGMk2CamoBrushstroke = 0x3815A945,
		SMGMk2CamoWoodland = 0x4B4B4FB0,
		SMGMk2CamoSkull = 0xEC729200,
		SMGMk2CamoSessanta = 0x48F64B22,
		SMGMk2CamoPerseus = 0x35992468,
		SMGMk2CamoLeopard = 0x24B782A5,
		SMGMk2CamoZebra = 0xA2E67F01,
		SMGMk2CamoGeometric = 0x2218FD68,
		SMGMk2CamoBoom = 0x45C5C3C5,
		SMGMk2CamoPatriotic = 0x399D558F,
		#endregion

		Invalid = 4294967295u,
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
		public WeaponComponent GetMk2CamoComponent(int index)
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.GunRoot)
				{
					if (index-- == 0)
					{
						return component;
					}
				}
			}
			return _invalidComponent;
		}

		public WeaponComponent GetMk2BarrelComponent(int index)
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.Barrel)
				{
					if (index-- == 0)
					{
						return component;
					}
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

				// GUNRUNNING
				case WeaponHash.PistolMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.PistolMk2ClipNormal,
						WeaponComponentHash.PistolMk2ClipExtended,
						WeaponComponentHash.PistolMk2ClipFMJ,
						WeaponComponentHash.PistolMk2ClipHollowpoint,
						WeaponComponentHash.PistolMk2ClipIncendiary,
						WeaponComponentHash.PistolMk2ClipTracer,
						WeaponComponentHash.PistolMk2Scope,
						WeaponComponentHash.PistolMk2Flash,
						WeaponComponentHash.AtPiSupp02,
						WeaponComponentHash.PistolMk2Compensator,
						WeaponComponentHash.PistolMk2CamoDigital,
						WeaponComponentHash.PistolMk2CamoBrushstroke,
						WeaponComponentHash.PistolMk2CamoWoodland,
						WeaponComponentHash.PistolMk2CamoSkull,
						WeaponComponentHash.PistolMk2CamoSessanta,
						WeaponComponentHash.PistolMk2CamoPerseus,
						WeaponComponentHash.PistolMk2CamoLeopard,
						WeaponComponentHash.PistolMk2CamoZebra,
						WeaponComponentHash.PistolMk2CamoGeometric,
						WeaponComponentHash.PistolMk2CamoBoom,
						WeaponComponentHash.PistolMk2CamoPatriotic,
						WeaponComponentHash.PistolMk2CamoSlideDigital,
						WeaponComponentHash.PistolMk2CamoSlideBrushstroke,
						WeaponComponentHash.PistolMk2CamoSlideWoodland,
						WeaponComponentHash.PistolMk2CamoSlideSkull,
						WeaponComponentHash.PistolMk2CamoSlideSessanta,
						WeaponComponentHash.PistolMk2CamoSlidePerseus,
						WeaponComponentHash.PistolMk2CamoSlideLeopard,
						WeaponComponentHash.PistolMk2CamoSlideZebra,
						WeaponComponentHash.PistolMk2CamoSlideGeometric,
						WeaponComponentHash.PistolMk2CamoSlideBoom,
						WeaponComponentHash.PistolMk2CamoSlidePatriotic,
					};
				case WeaponHash.AssaultRifleMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AssaultRifleMk2ClipNormal,
						WeaponComponentHash.AssaultRifleMk2ClipExtended,
						WeaponComponentHash.AssaultRifleMk2ClipArmorPiercing,
						WeaponComponentHash.AssaultRifleMk2ClipFMJ,
						WeaponComponentHash.AssaultRifleMk2ClipIncendiary,
						WeaponComponentHash.AssaultRifleMk2ClipTracer,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtSights,
						WeaponComponentHash.AtScopeMacroMk2,
						WeaponComponentHash.AtScopeMediumMk2,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AtMuzzle1,
						WeaponComponentHash.AtMuzzle2,
						WeaponComponentHash.AtMuzzle3,
						WeaponComponentHash.AtMuzzle4,
						WeaponComponentHash.AtMuzzle5,
						WeaponComponentHash.AtMuzzle6,
						WeaponComponentHash.AtMuzzle7,
						WeaponComponentHash.AtArAfGrip2,
						WeaponComponentHash.AssaultRifleMk2BarrelNormal,
						WeaponComponentHash.AssaultRifleMk2BarrelHeavy,
						WeaponComponentHash.AssaultRifleMk2CamoDigital,
						WeaponComponentHash.AssaultRifleMk2CamoBrushstroke,
						WeaponComponentHash.AssaultRifleMk2CamoWoodland,
						WeaponComponentHash.AssaultRifleMk2CamoSkull,
						WeaponComponentHash.AssaultRifleMk2CamoSessanta,
						WeaponComponentHash.AssaultRifleMk2CamoPerseus,
						WeaponComponentHash.AssaultRifleMk2CamoLeopard,
						WeaponComponentHash.AssaultRifleMk2CamoZebra,
						WeaponComponentHash.AssaultRifleMk2CamoGeometric,
						WeaponComponentHash.AssaultRifleMk2CamoBoom,
						WeaponComponentHash.AssaultRifleMk2CamoPatriotic,
					};
				case WeaponHash.CarbineRifleMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CarbineRifleMk2ClipNormal,
						WeaponComponentHash.CarbineRifleMk2ClipExtended,
						WeaponComponentHash.CarbineRifleMk2ClipArmorPiercing,
						WeaponComponentHash.CarbineRifleMk2ClipFMJ,
						WeaponComponentHash.CarbineRifleMk2ClipIncendiary,
						WeaponComponentHash.CarbineRifleMk2ClipTracer,
						WeaponComponentHash.AtSights,
						WeaponComponentHash.AtScopeMacroMk2,
						WeaponComponentHash.AtScopeMediumMk2,
						WeaponComponentHash.AtArAfGrip2,
						WeaponComponentHash.CarbineRifleMk2BarrelNormal,
						WeaponComponentHash.CarbineRifleMk2BarrelHeavy,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.AtMuzzle1,
						WeaponComponentHash.AtMuzzle2,
						WeaponComponentHash.AtMuzzle3,
						WeaponComponentHash.AtMuzzle4,
						WeaponComponentHash.AtMuzzle5,
						WeaponComponentHash.AtMuzzle6,
						WeaponComponentHash.AtMuzzle7,
						WeaponComponentHash.CarbineRifleMk2CamoDigital,
						WeaponComponentHash.CarbineRifleMk2CamoBrushstroke,
						WeaponComponentHash.CarbineRifleMk2CamoWoodland,
						WeaponComponentHash.CarbineRifleMk2CamoSkull,
						WeaponComponentHash.CarbineRifleMk2CamoSessanta,
						WeaponComponentHash.CarbineRifleMk2CamoPerseus,
						WeaponComponentHash.CarbineRifleMk2CamoLeopard,
						WeaponComponentHash.CarbineRifleMk2CamoZebra,
						WeaponComponentHash.CarbineRifleMk2CamoGeometric,
						WeaponComponentHash.CarbineRifleMk2CamoBoom,
						WeaponComponentHash.CarbineRifleMk2CamoPatriotic,
					};
				case WeaponHash.CombatMGMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CombatMGMk2ClipNormal,
						WeaponComponentHash.CombatMGMk2ClipExtended,
						WeaponComponentHash.CombatMGMk2ClipArmorPiercing,
						WeaponComponentHash.CombatMGMk2ClipFMJ,
						WeaponComponentHash.CombatMGMk2ClipIncendiary,
						WeaponComponentHash.CombatMGMk2ClipTracer,
						WeaponComponentHash.AtSights,
						WeaponComponentHash.AtScopeSmallMk2,
						WeaponComponentHash.AtScopeMediumMk2,
						WeaponComponentHash.AtMuzzle1,
						WeaponComponentHash.AtMuzzle2,
						WeaponComponentHash.AtMuzzle3,
						WeaponComponentHash.AtMuzzle4,
						WeaponComponentHash.AtMuzzle5,
						WeaponComponentHash.AtMuzzle6,
						WeaponComponentHash.AtMuzzle7,
						WeaponComponentHash.AtArAfGrip2,
						WeaponComponentHash.CombatMGMk2BarrelNormal,
						WeaponComponentHash.CombatMGMk2BarrelHeavy,
						WeaponComponentHash.CombatMGMk2CamoDigital,
						WeaponComponentHash.CombatMGMk2CamoBrushstroke,
						WeaponComponentHash.CombatMGMk2CamoWoodland,
						WeaponComponentHash.CombatMGMk2CamoSkull,
						WeaponComponentHash.CombatMGMk2CamoSessanta,
						WeaponComponentHash.CombatMGMk2CamoPerseus,
						WeaponComponentHash.CombatMGMk2CamoLeopard,
						WeaponComponentHash.CombatMGMk2CamoZebra,
						WeaponComponentHash.CombatMGMk2CamoGeometric,
						WeaponComponentHash.CombatMGMk2CamoBoom,
						WeaponComponentHash.CombatMGMk2CamoPatriotic,
					};
				case WeaponHash.HeavySniperMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.HeavySniperMk2ClipNormal,
						WeaponComponentHash.HeavySniperMk2ClipExtended,
						WeaponComponentHash.HeavySniperMk2ClipArmorPiercing,
						WeaponComponentHash.HeavySniperMk2ClipExplosive,
						WeaponComponentHash.HeavySniperMk2ClipFMJ,
						WeaponComponentHash.HeavySniperMk2ClipIncendiary,
						WeaponComponentHash.HeavySniperMk2ScopeLarge,
						WeaponComponentHash.AtScopeMax,
						WeaponComponentHash.HeavySniperMk2ScopeNightvision,
						WeaponComponentHash.HeavySniperMk2ScopeThermal,
						WeaponComponentHash.HeavySniperMk2Suppressor,
						WeaponComponentHash.HeavySniperMk2Muzzle8,
						WeaponComponentHash.HeavySniperMk2Muzzle9,
						WeaponComponentHash.HeavySniperMk2BarrelNormal,
						WeaponComponentHash.HeavySniperMk2BarrelHeavy,
						WeaponComponentHash.HeavySniperMk2CamoDigital,
						WeaponComponentHash.HeavySniperMk2CamoBrushstroke,
						WeaponComponentHash.HeavySniperMk2CamoWoodland,
						WeaponComponentHash.HeavySniperMk2CamoSkull,
						WeaponComponentHash.HeavySniperMk2CamoSessanta,
						WeaponComponentHash.HeavySniperMk2CamoPerseus,
						WeaponComponentHash.HeavySniperMk2CamoLeopard,
						WeaponComponentHash.HeavySniperMk2CamoZebra,
						WeaponComponentHash.HeavySniperMk2CamoGeometric,
						WeaponComponentHash.HeavySniperMk2CamoBoom,
						WeaponComponentHash.HeavySniperMk2CamoPatriotic,
					};
				case WeaponHash.SMGMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SMGMk2ClipNormal,
						WeaponComponentHash.SMGMk2ClipExtended,
						WeaponComponentHash.SMGMk2ClipFMJ,
						WeaponComponentHash.SMGMk2ClipHollowpoint,
						WeaponComponentHash.SMGMk2ClipIncendiary,
						WeaponComponentHash.SMGMk2ClipTracer,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.SMGMk2Sights,
						WeaponComponentHash.SMGMk2ScopeMacro,
						WeaponComponentHash.SMGMk2ScopeSmall,
						WeaponComponentHash.AtPiSupp,
						WeaponComponentHash.SMGMk2BarrelNormal,
						WeaponComponentHash.SMGMk2BarrelHeavy,
						WeaponComponentHash.AtMuzzle1,
						WeaponComponentHash.AtMuzzle2,
						WeaponComponentHash.AtMuzzle3,
						WeaponComponentHash.AtMuzzle4,
						WeaponComponentHash.AtMuzzle5,
						WeaponComponentHash.AtMuzzle6,
						WeaponComponentHash.AtMuzzle7,
						WeaponComponentHash.SMGMk2CamoDigital,
						WeaponComponentHash.SMGMk2CamoBrushstroke,
						WeaponComponentHash.SMGMk2CamoWoodland,
						WeaponComponentHash.SMGMk2CamoSkull,
						WeaponComponentHash.SMGMk2CamoSessanta,
						WeaponComponentHash.SMGMk2CamoPerseus,
						WeaponComponentHash.SMGMk2CamoLeopard,
						WeaponComponentHash.SMGMk2CamoZebra,
						WeaponComponentHash.SMGMk2CamoGeometric,
						WeaponComponentHash.SMGMk2CamoBoom,
						WeaponComponentHash.SMGMk2CamoPatriotic,
					};
			}

			WeaponComponentHash[] result = null;

			if (result == null)
			{
				result = new WeaponComponentHash[0];
			}

			return result;
		}
	}
}