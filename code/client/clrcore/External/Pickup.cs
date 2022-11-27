using System;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	public enum PickupType : uint
	{
		WeaponBullpupShotgun = 1850631618u,
		WeaponAssaultSMG = 1948018762u,
		VehicleWeaponAssaultSMG = 1751145014u,
		WeaponPistol50 = 1817941018u,
		VehicleWeaponPistol50 = 3550712678u,
		AmmoBulletMP = 1426343849u,
		AmmoMissileMP = 4187887056u,
		AmmoGrenadeLauncherMP = 2753668402u,
		WeaponAssaultRifle = 4080829360u,
		WeaponCarbineRifle = 3748731225u,
		WeaponAdvancedRifle = 2998219358u,
		WeaponMG = 2244651441u,
		WeaponCombatMG = 2995980820u,
		WeaponSniperRifle = 4264178988u,
		WeaponHeavySniper = 1765114797u,
		WeaponMicroSMG = 496339155u,
		WeaponSMG = 978070226u,
		Armour = 1274757841u,
		WeaponRPG = 1295434569u,
		WeaponMinigun = 792114228u,
		Health = 2406513688u,
		WeaponPumpShotgun = 2838846925u,
		WeaponSawnoffShotgun = 2528383651u,
		WeaponAssaultShotgun = 2459552091u,
		WeaponGrenade = 1577485217u,
		WeaponMolotov = 768803961u,
		WeaponSmokeGrenade = 483787975u,
		WeaponStickyBomb = 2081529176u,
		WeaponPistol = 4189041807u,
		WeaponCombatPistol = 2305275123u,
		WeaponAPPistol = 996550793u,
		WeaponGrenadeLauncher = 779501861u,
		MoneyVariable = 4263048111u,
		GangAttackMoney = 3782592152u,
		WeaponStungun = 4246083230u,
		WeaponPetrolcan = 3332236287u,
		WeaponKnife = 663586612u,
		WeaponNightstick = 1587637620u,
		WeaponHammer = 693539241u,
		WeaponBat = 2179883038u,
		WeaponGolfclub = 2297080999u,
		WeaponCrowbar = 2267924616u,
		CustomScript = 738282662u,
		Camera = 3812460080u,
		PortablePackage = 2158727964u,
		PortableCrateUnfixed = 1852930709u,
		PortablePackageLargeRadius = 1651898027u,
		PortableCrateUnfixedInCar = 1263688126u,
		PortableCrateUnfixedInAirVehicleWithPassengers = 2431639355u,
		PortableCrateUnfixedInAirVehicleWithPassengersUpright = 68603185u,
		PortableCrateUnfixedInCarWithPassengers = 79909481u,
		PortableCrateFixedInCarWithPassengers = 2689501965u,
		PortableCrateFixedInCarSmall = 2817147086u,
		PortableCrateUnfixedInCarSmall = 3285027633u,
		PortableCrateUnfixedLowGlow = 2499414878u,
		MoneyCase = 3463437675u,
		MoneyWallet = 1575005502u,
		MoneyPurse = 513448440u,
		MoneyDepBag = 545862290u,
		MoneyMedBag = 341217064u,
		MoneyPaperBag = 1897726628u,
		MoneySecurityCase = 3732468094u,
		VehicleWeaponCombatPistol = 3500855031u,
		VehicleWeaponAPPistol = 3431676165u,
		VehicleWeaponPistol = 2773149623u,
		VehicleWeaponGrenade = 2803366040u,
		VehicleWeaponMolotov = 2228647636u,
		VehicleWeaponSmokeGrenade = 1705498857u,
		VehicleWeaponStickyBomb = 746606563u,
		VehicleHealth = 160266735u,
		VehicleHealthLowGlow = 4260266856u,
		VehicleArmour = 1125567497u,
		VehicleWeaponMicroSMG = 3094015579u,
		VehicleWeaponSMG = 3430731035u,
		VehicleWeaponSawnoffShotgun = 772217690u,
		VehicleCustomScript = 2780351145u,
		VehicleCustomScriptNoRotate = 83435908u,
		VehicleCustomScriptLowGlow = 1104334678u,
		VehicleMoneyVariable = 1704231442u,
		Submarine = 3889104844u,
		HealthSnack = 483577702u,
		Parachute = 1735599485u,
		AmmoPistol = 544828034u,
		AmmoSMG = 292537574u,
		AmmoRifle = 3837603782u,
		AmmoMG = 3730366643u,
		AmmoShotgun = 2012476125u,
		AmmoSniper = 3224170789u,
		AmmoGrenadeLauncher = 2283450536u,
		AmmoRPG = 2223210455u,
		AmmoMinigun = 4065984953u,
		WeaponBottle = 4199656437u,
		WeaponSNSPistol = 3317114643u,
		WeaponHeavyPistol = 2633054488u,
		WeaponSpecialCarbine = 157823901u,
		WeaponBullpupRifle = 2170382056u,
		WeaponRayPistol = 3812817136u,
		WeaponRayCarbine = 1959050722u,
		WeaponRayMinigun = 1000920287u,
		WeaponBullpupRifleMK2 = 2349845267u,
		WeaponDoubleAction = 990867623u,
		WeaponMarksmanRifleMK2 = 2673201481u,
		WeaponPumpShotgunMK2 = 1572258186u,
		WeaponRevolverMK2 = 1835046764u,
		WeaponSNSPistolMK2 = 1038697149u,
		WeaponSpecialCarbineMK2 = 94531552u,
		WeaponProxmine = 1649373715u,
		WeaponHomingLauncher = 3223238264u,
		AmmoHomingLauncher = 1548844439u,
		WeaponGusenberg = 1393009900u,
		WeaponDagger = 3220073531u,
		WeaponVintagePistol = 3958938975u,
		WeaponFirework = 582047296u,
		WeaponMusket = 1983869217u,
		AmmoFirework = 4180625516u,
		AmmoFireworkMP = 1613316560u,
		PortableDLCVehiclePackage = 837436873u,
		WeaponHatchet = 1311775952u,
		WeaponRailgun = 3832418740u,
		WeaponHeavyShotgun = 3201593029u,
		WeaponMarksmanRifle = 127042729u,
		WeaponCeramicPistol = 1601729296u,
		WeaponHazardcan = 2045070941u,
		WeaponNavyRevolver = 3392027813u,
		WeaponCombatShotgun = 2074855423u,
		WeaponGadgetPistol = 2010690963u,
		WeaponMilitaryRifle = 884272848u,
		WeaponFlaregun = 3175998018u,
		AmmoFlaregun = 3759398940u,
		WeaponKnuckle = 4254904030u,
		WeaponMarksmanPistol = 2329799797u,
		WeaponCombatPDW = 2023061218u,
		PortableCrateFixedInCar = 3993904883u,
		WeaponCompactRifle = 266812085u,
		WeaponDBShotgun = 4192395039u,
		WeaponMachete = 3626334911u,
		WeaponMachinePistol = 4123384540u,
		WeaponFlashlight = 3182886821u,
		WeaponRevolver = 1632369836u,
		WeaponSwitchblade = 3722713114u,
		WeaponAutoShotgun = 3167076850u,
		WeaponBattleaxe = 158843122u,
		WeaponCompactLauncher = 4041868857u,
		WeaponMiniSMG = 3547474523u,
		WeaponPipeBomb = 2942905513u,
		WeaponPoolcue = 155106086u,
		WeaponWrench = 3843167081u,
		WeaponAssaultRifleMK2 = 2173116527u,
		WeaponCarbineRifleMk2 = 3185079484u,
		WeaponCombatmgMK2 = 2837437579u,
		WeaponHeavySniperMK2 = 4278878871u,
		WeaponPistolMK2 = 1234831722u,
		WeaponSMGMK2 = 4012602256u,
		WeaponStoneHatchet = 3432031091u,
		WeaponMetalDetector = 2226947771u,
		WeaponTacticalRifle = 2316705120u,
		WeaponPrecisionRifle = 2821026276u,
		WeaponEMPLauncher = 4284229131u,
		AmmoEMPLauncher = 2308161313u,
		WeaponHeavyRifle = 1491498856u,
		WeaponPetrolcanSmallRadius = 3279969783u,
		WeaponFertilizerCan = 3708929359u,
		WeaponStungunMP = 3025681922u,
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
