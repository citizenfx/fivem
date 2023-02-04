using System;

#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{
	public enum PickupType : uint
	{
		CustomScript = 738282662u,
		VehicleCustomScript = 2780351145u,
		Parachute = 1735599485u,
		PortablePackage = 2158727964u,
		PortableCrateUnfixed = 1852930709u,
		Health = 2406513688u,
		HealthSnack = 483577702u,
		Armour = 1274757841u,
		MoneyCase = 3463437675u,
		MoneySecurityCase = 3732468094u,
		MoneyVariable = 4263048111u,
		MoneyMedBag = 341217064u,
		MoneyPurse = 513448440u,
		MoneyDepBag = 545862290u,
		MoneyWallet = 1575005502u,
		MoneyPaperBag = 1897726628u,
		WeaponPistol = 4189041807u,
		WeaponPistolMK2 = 1234831722u,
		WeaponCombatPistol = 2305275123u,
		WeaponAPPistol = 996550793u,
		WeaponSNSPistol = 3317114643u,
		WeaponSNSPistolMK2 = 1038697149u,
		WeaponHeavyPistol = 2633054488u,
		WeaponPistol50 = 1817941018u,
		WeaponVintagePistol = 3958938975u,
		WeaponCeramicPistol = 1601729296u,
		WeaponGadgetPistol = 2010690963u,
		WeaponMarksmanPistol = 2329799797u,
		WeaponMachinePistol = 4123384540u,
		WeaponPistolXM3 = 3063083075u,
		WeaponNavyRevolver = 3392027813u,
		WeaponRevolver = 1632369836u,
		WeaponRevolverMK2 = 1835046764u,
		WeaponMiniSMG = 3547474523u,
		WeaponMicroSMG = 496339155u,
		WeaponSMG = 978070226u,
		WeaponSMGMK2 = 4012602256u,
		WeaponAssaultSMG = 1948018762u,
		WeaponMG = 2244651441u,
		WeaponCombatMG = 2995980820u,
		WeaponCombatMGMK2 = 2837437579u,
		WeaponGusenberg = 1393009900u,
		WeaponAssaultRifle = 4080829360u,
		WeaponAssaultRifleMK2 = 2173116527u,
		WeaponCarbineRifle = 3748731225u,
		WeaponCarbineRifleMk2 = 3185079484u,
		WeaponAdvancedRifle = 2998219358u,
		WeaponSpecialCarbine = 157823901u,
		WeaponSpecialCarbineMK2 = 94531552u,
		WeaponBullpupRifle = 2170382056u,
		WeaponBullpupRifleMK2 = 2349845267u,
		WeaponMarksmanRifle = 127042729u,
		WeaponMarksmanRifleMK2 = 2673201481u,
		WeaponMilitaryRifle = 884272848u,
		WeaponCompactRifle = 266812085u,
		WeaponTacticalRifle = 2316705120u,
		WeaponPrecisionRifle = 2821026276u,
		WeaponHeavyRifle = 1491498856u,
		WeaponCombatPDW = 2023061218u,
		WeaponPumpShotgun = 2838846925u,
		WeaponPumpShotgunMK2 = 1572258186u,
		WeaponSawnoffShotgun = 2528383651u,
		WeaponAssaultShotgun = 2459552091u,
		WeaponBullpupShotgun = 1850631618u,
		WeaponHeavyShotgun = 3201593029u,
		WeaponCombatShotgun = 2074855423u,
		WeaponDBShotgun = 4192395039u,
		WeaponAutoShotgun = 3167076850u,
		WeaponDoubleAction = 990867623u,
		WeaponSniperRifle = 4264178988u,
		WeaponHeavySniper = 1765114797u,
		WeaponHeavySniperMK2 = 4278878871u,
		WeaponMusket = 1983869217u,
		WeaponGrenadeLauncher = 779501861u,
		WeaponCompactLauncher = 4041868857u,
		WeaponEMPLauncher = 4284229131u,
		WeaponRPG = 1295434569u,
		WeaponHomingLauncher = 3223238264u,
		WeaponFirework = 582047296u,
		WeaponMinigun = 792114228u,
		WeaponGrenade = 1577485217u,
		WeaponStickyBomb = 2081529176u,
		WeaponSmokeGrenade = 483787975u,
		WeaponMolotov = 768803961u,
		WeaponProxmine = 1649373715u,
		WeaponPipeBomb = 2942905513u,
		WeaponPetrolCan = 3332236287u,
		WeaponPetrolCanSmallRadius = 3279969783u,
		WeaponFertilizerCan = 3708929359u,
		WeaponHazardCan = 2045070941u,
		WeaponStungun = 4246083230u,
		WeaponStungunMP = 3025681922u,
		WeaponFlaregun = 3175998018u,
		WeaponRayPistol = 3812817136u,
		WeaponRayCarbine = 1959050722u,
		WeaponRayMinigun = 1000920287u,
		WeaponRailgun = 3832418740u,
		WeaponRailgunXM3 = 4109932467u,
		WeaponKnife = 663586612u,
		WeaponSwitchblade = 3722713114u,
		WeaponDagger = 3220073531u,
		WeaponMachete = 3626334911u,
		WeaponNightstick = 1587637620u,
		WeaponHammer = 693539241u,
		WeaponHatchet = 1311775952u,
		WeaponBat = 2179883038u,
		WeaponBattleAxe = 158843122u,
		WeaponStoneHatchet = 3432031091u,
		WeaponCrowbar = 2267924616u,
		WeaponGolfclub = 2297080999u,
		WeaponPoolcue = 155106086u,
		WeaponWrench = 3843167081u,
		WeaponBottle = 4199656437u,
		WeaponKnuckle = 4254904030u,
		WeaponFlashlight = 3182886821u,
		WeaponMetalDetector = 2226947771u,
		WeaponCandyCane = 1337246736u,
		VehicleWeaponPistol = 2773149623u,
		VehicleWeaponCombatPistol = 3500855031u,
		VehicleWeaponAPPistol = 3431676165u,
		VehicleWeaponPistol50 = 3550712678u,
		VehicleWeaponMicroSMG = 3094015579u,
		VehicleWeaponSMG = 3430731035u,
		VehicleWeaponAssaultSMG = 1751145014u,
		VehicleWeaponSawnoffShotgun = 772217690u,
		VehicleWeaponGrenade = 2803366040u,
		VehicleWeaponSmokeGrenade = 1705498857u,
		VehicleWeaponStickyBomb = 746606563u,
		VehicleWeaponMolotov = 2228647636u,
		VehicleHealth = 160266735u,
		VehicleHealthLowGlow = 4260266856u,
		VehicleArmour = 1125567497u,
		VehicleCustomScriptNoRotate = 83435908u,
		VehicleCustomScriptLowGlow = 1104334678u,
		VehicleMoneyVariable = 1704231442u,
		AmmoPistol = 544828034u,
		AmmoSMG = 292537574u,
		AmmoMG = 3730366643u,
		AmmoRifle = 3837603782u,
		AmmoShotgun = 2012476125u,
		AmmoSniper = 3224170789u,
		AmmoGrenadeLauncher = 2283450536u,
		AmmoRPG = 2223210455u,
		AmmoMinigun = 4065984953u,
		AmmoMissileMP = 4187887056u,
		AmmoBulletMP = 1426343849u,
		AmmoGrenadeLauncherMP = 2753668402u,
		AmmoHomingLauncher = 1548844439u,
		AmmoFirework = 4180625516u,
		AmmoFireworkMP = 1613316560u,
		AmmoFlaregun = 3759398940u,
		AmmoEMPLauncher = 2308161313u,
		GangAttackMoney = 3782592152u,
		Submarine = 3889104844u,
		Camera = 3812460080u,
		PortablePackageLargeRadius = 1651898027u,
		PortableCrateUnfixedInCar = 1263688126u,
		PortableCrateUnfixedInAirVehicleWithPassengers = 2431639355u,
		PortableCrateUnfixedInAirVehicleWithPassengersUpright = 68603185u,
		PortableCrateUnfixedInCarWithPassengers = 79909481u,
		PortableCrateFixedInCar = 3993904883u,
		PortableCrateFixedInCarWithPassengers = 2689501965u,
		PortableCrateFixedInCarSmall = 2817147086u,
		PortableCrateUnfixedInCarSmall = 3285027633u,
		PortableCrateUnfixedLowGlow = 2499414878u,
		PortableDLCVehiclePackage = 837436873u,
		PortableFMContentMissionEntitySmall = 1610516839u
	}

	public sealed class Pickup : PoolObject, IEquatable<Pickup>
	{
		public Pickup(int handle) : base(handle)
		{
		}

		public Vector3 Position
		{
			get
			{
				return API.GetPickupCoords(Handle);
			}
		}

		public bool IsCollected
		{
			get
			{
				return API.HasPickupBeenCollected(Handle);
			}
		}

		public override void Delete()
		{
			API.RemovePickup(Handle);
		}

		public override bool Exists()
		{
			return API.DoesPickupExist(Handle);
		}
		public static bool Exists(Pickup pickup)
		{
			return !ReferenceEquals(pickup, null) && pickup.Exists();
		}
		public bool ObjectExists()
		{
			return API.DoesPickupObjectExist(Handle);
		}

		public bool Equals(Pickup pickup)
		{
			return !ReferenceEquals(pickup, null) && Handle == pickup.Handle;
		}
		public sealed override bool Equals(object obj)
		{
			return !ReferenceEquals(obj, null) && obj.GetType() == GetType() && Equals((Pickup)obj);
		}

		public sealed override int GetHashCode()
		{
			return Handle;
		}

		public static bool operator ==(Pickup left, Pickup right)
		{
			return ReferenceEquals(left, null) ? ReferenceEquals(right, null) : left.Equals(right);
		}
		public static bool operator !=(Pickup left, Pickup right)
		{
			return !(left == right);
		}
	}
}
