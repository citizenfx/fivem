using System;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
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
		WeaponCombatPistol = 2305275123u,
		WeaponAPPistol = 996550793u,
		WeaponSNSPistol = 3317114643u,
		WeaponHeavyPistol = 2633054488u,
		WeaponMicroSMG = 496339155u,
		WeaponSMG = 978070226u,
		WeaponMG = 2244651441u,
		WeaponCombatMG = 2995980820u,
		WeaponAssaultRifle = 4080829360u,
		WeaponCarbineRifle = 3748731225u,
		WeaponAdvancedRifle = 2998219358u,
		WeaponSpecialCarbine = 157823901u,
		WeaponBullpupRifle = 2170382056u,
		WeaponPumpShotgun = 2838846925u,
		WeaponSawnoffShotgun = 2528383651u,
		WeaponAssaultShotgun = 2459552091u,
		WeaponSniperRifle = 4264178988u,
		WeaponHeavySniper = 1765114797u,
		WeaponGrenadeLauncher = 779501861u,
		WeaponRPG = 1295434569u,
		WeaponMinigun = 792114228u,
		WeaponGrenade = 1577485217u,
		WeaponStickyBomb = 2081529176u,
		WeaponSmokeGrenade = 483787975u,
		WeaponMolotov = 768803961u,
		WeaponPetrolCan = 3332236287u,
		WeaponKnife = 663586612u,
		WeaponNightstick = 1587637620u,
		WeaponBat = 2179883038u,
		WeaponCrowbar = 2267924616u,
		WeaponGolfclub = 2297080999u,
		WeaponBottle = 4199656437u,
		VehicleWeaponPistol = 2773149623u,
		VehicleWeaponCombatPistol = 3500855031u,
		VehicleWeaponAPPistol = 3431676165u,
		VehicleWeaponMicroSMG = 3094015579u,
		VehicleWeaponSawnoffShotgun = 772217690u,
		VehicleWeaponGrenade = 2803366040u,
		VehicleWeaponSmokeGrenade = 1705498857u,
		VehicleWeaponStickyBomb = 746606563u,
		VehicleWeaponMolotov = 2228647636u,
		VehicleHealth = 160266735u,
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
		AmmoGrenadeLauncherMP = 2753668402u
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
				return Function.Call<Vector3>(Hash.GET_PICKUP_COORDS, Handle);
			}
		}

		public bool IsCollected
		{
			get
			{
				return Function.Call<bool>(Hash.HAS_PICKUP_BEEN_COLLECTED, Handle);
			}
		}

		public override void Delete()
		{
			Function.Call(Hash.REMOVE_PICKUP, Handle);
		}

		public override bool Exists()
		{
			return Function.Call<bool>(Hash.DOES_PICKUP_EXIST, Handle);
		}
		public static bool Exists(Pickup pickup)
		{
			return !ReferenceEquals(pickup, null) && pickup.Exists();
		}
		public bool ObjectExists()
		{
			return Function.Call<bool>(Hash.DOES_PICKUP_OBJECT_EXIST, Handle);
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
