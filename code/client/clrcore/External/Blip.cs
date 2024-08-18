using System;
using System.Security;

#if MONO_V2
using CitizenFX.Core;
using CitizenFX.FiveM.Native;
using API = CitizenFX.FiveM.Native.Natives;
using Function = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{
	public enum BlipColor
	{
		White,
		Red,
		Green,
		Blue,
		MichaelBlue = 42,
		FranklinGreen,
		TrevorOrange,
		Yellow = 66
	}
	public enum BlipSprite
	{
		Standard = 1,
		BigBlip,
		PoliceOfficer,
		PoliceArea,
		Square,
		Player,
		North,
		Waypoint,
		BigCircle,
		BigCircleOutline,
		ArrowUpOutlined,
		ArrowDownOutlined,
		ArrowUp,
		ArrowDown,
		PoliceHelicopterAnimated,
		Jet,
		Number1,
		Number2,
		Number3,
		Number4,
		Number5,
		Number6,
		Number7,
		Number8,
		Number9,
		Number10,
		GTAOCrew,
		GTAOFriendly,
		Lift = 36,
		RaceFinish = 38,
		Safehouse = 40,
		PoliceOfficer2,
		PoliceCarDot,
		PoliceHelicopter,
		ChatBubble = 47,
		Garage2 = 50,
		Drugs,
		Store,
		PoliceCar = 56,
		PolicePlayer = 58,
		PoliceStation = 60,
		Hospital,
		Helicopter = 64,
		StrangersAndFreaks,
		ArmoredTruck,
		TowTruck = 68,
		Barber = 71,
		LosSantosCustoms,
		Clothes,
		TattooParlor = 75,
		Simeon,
		Lester,
		Michael,
		Trevor,
		Rampage = 84,
		VinewoodTours,
		Lamar,
		Franklin = 88,
		Chinese,
		Airport,
		Bar = 93,
		BaseJump,
		CarWash = 100,
		ComedyClub = 102,
		Dart,
		FIB = 106,
		DollarSign = 108,
		Golf,
		AmmuNation,
		Exile = 112,
		ShootingRange = 119,
		Solomon,
		StripClub,
		Tennis,
		Triathlon = 126,
		OffRoadRaceFinish,
		Key = 134,
		MovieTheater,
		Music,
		Marijuana = 140,
		Hunting,
		ArmsTraffickingGround = 147,
		Nigel = 149,
		AssaultRifle,
		Bat,
		Grenade,
		Health,
		Knife,
		Molotov,
		Pistol,
		RPG,
		Shotgun,
		SMG,
		Sniper,
		SonicWave,
		PointOfInterest,
		GTAOPassive,
		GTAOUsingMenu,
		Link = 171,
		Minigun = 173,
		GrenadeLauncher,
		Armor,
		Castle,
		Camera = 184,
		Handcuffs = 188,
		Yoga = 197,
		Cab,
		Number11,
		Number12,
		Number13,
		Number14,
		Number15,
		Number16,
		Shrink,
		Epsilon,
		PersonalVehicleCar = 225,
		PersonalVehicleBike,
		Custody = 237,
		ArmsTraffickingAir = 251,
		Fairground = 266,
		PropertyManagement,
		Altruist = 269,
		Enemy,
		Chop = 273,
		Dead,
		Hooker = 279,
		Friend,
		BountyHit = 303,
		GTAOMission,
		GTAOSurvival,
		CrateDrop,
		PlaneDrop,
		Sub,
		Race,
		Deathmatch,
		ArmWrestling,
		AmmuNationShootingRange = 313,
		RaceAir,
		RaceCar,
		RaceSea,
		GarbageTruck = 318,
		SafehouseForSale = 350,
		Package,
		MartinMadrazo,
		EnemyHelicopter,
		Boost,
		Devin,
		Marina,
		Garage,
		GolfFlag,
		Hangar,
		Helipad,
		JerryCan,
		Masks,
		HeistSetup,
		Incapacitated,
		PickupSpawn,
		BoilerSuit,
		Completed,
		Rockets,
		GarageForSale,
		HelipadForSale,
		MarinaForSale,
		HangarForSale,
		Business = 374,
		BusinessForSale,
		RaceBike,
		Parachute,
		TeamDeathmatch,
		RaceFoot,
		VehicleDeathmatch,
		Barry,
		Dom,
		MaryAnn,
		Cletus,
		Josh,
		Minute,
		Omega,
		Tonya,
		Paparazzo,
		Crosshair,
		Creator = 398,
		CreatorDirection,
		Abigail,
		Blimp,
		Repair,
		Testosterone,
		Dinghy,
		Fanatic,
		Information = 407,
		CaptureBriefcase,
		LastTeamStanding,
		Boat,
		CaptureHouse,
		JerryCan2 = 415,
		RP,
		GTAOPlayerSafehouse,
		GTAOPlayerSafehouseDead,
		CaptureAmericanFlag,
		CaptureFlag,
		Tank,
		HelicopterAnimated,
		Plane,
		PlayerNoColor = 425,
		GunCar,
		Speedboat,
		Heist,
		Stopwatch = 430,
		DollarSignCircled,
		Crosshair2,
		DollarSignSquared = 434
	}

	public sealed class Blip : PoolObject, IEquatable<Blip>
	{
		public Blip(int handle) : base(handle)
		{
		}

		/// <summary>
		/// Gets or sets the position of this <see cref="Blip"/>.
		/// </summary>
		public Vector3 Position
		{
			get
			{
				return API.GetBlipInfoIdCoord(Handle);
			}
			set
			{
				API.SetBlipCoords(Handle, value.X, value.Y, value.Z);
			}
		}
		/// <summary>
		/// Sets the rotation of this <see cref="Blip"/> on the map.
		/// </summary>
		public int Rotation
		{
			set
			{
				API.SetBlipRotation(Handle, value);
			}
		}
		/// <summary>
		/// Sets the scale of this <see cref="Blip"/> on the map.
		/// </summary>
		public float Scale
		{
			set
			{
				API.SetBlipScale(Handle, value);
			}
		}

		/// <summary>
		/// Gets the type of this <see cref="Blip"/>.
		/// </summary>
		public int Type
		{
			get
			{
				return API.GetBlipInfoIdType(Handle);
			}
		}
		/// <summary>
		/// Gets or sets the alpha of this <see cref="Blip"/> on the map.
		/// </summary>
		public int Alpha
		{
			get
			{
				return API.GetBlipAlpha(Handle);
			}
			set
			{
				API.SetBlipAlpha(Handle, value);
			}
		}
		/// <summary>
		/// Sets the priority of this <see cref="Blip"/>.
		/// </summary>
		public int Priority
		{
			set
			{
				API.SetBlipPriority(Handle, value);
			}
		}
		/// <summary>
		/// Sets this <see cref="Blip"/>s label to the given number.
		/// </summary>
		public int NumberLabel
		{
			set
			{
				API.ShowNumberOnBlip(Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets the color of this <see cref="Blip"/>.
		/// </summary>
		public BlipColor Color
		{
			get
			{
				return (BlipColor)API.GetBlipColour(Handle);
			}
			set
			{
				API.SetBlipColour(Handle, (int)value);
			}
		}
		/// <summary>
		/// Gets or sets the sprite of this <see cref="Blip"/>.
		/// </summary>
		public BlipSprite Sprite
		{
			get
			{
				return (BlipSprite)API.GetBlipSprite(Handle);
			}
			set
			{
				API.SetBlipSprite(Handle, (int)value);
			}
		}
		/// <summary>
		/// Sets this <see cref="Blip"/>s label to the given string.
		/// </summary>
		public string Name
		{
			set
			{
				API.BeginTextCommandSetBlipName("STRING");
				API.AddTextComponentSubstringPlayerName(value);
				API.EndTextCommandSetBlipName(Handle);
			}
		}
		/// <summary>
		/// Gets the <see cref="Entity"/> this <see cref="Blip"/> is attached to.
		/// </summary>
		public Entity Entity
		{
			get
			{
				return Entity.FromHandle(API.GetBlipInfoIdEntityIndex(Handle));
			}
		}

		/// <summary>
		/// Sets a value indicating whether the route to this <see cref="Blip"/> should be shown on the map.
		/// </summary>
		/// <value>
		///   <c>true</c> to show the route; otherwise, <c>false</c>.
		/// </value>
		public bool ShowRoute
		{
			set
			{
				API.SetBlipRoute(Handle, value);
			}
		}
		/// <summary>
		/// Sets a value indicating whether this <see cref="Blip"/> is friendly.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Blip"/> is friendly; otherwise, <c>false</c>.
		/// </value>
		public bool IsFriendly
		{
			set
			{
				API.SetBlipAsFriendly(Handle, value);
			}
		}
		/// <summary>
		/// Sets a value indicating whether this (Player) <see cref="Blip"/> is a friend. Toggles a half cyan circle on the right side.
		/// </summary>
		/// <value>
		/// <c>true</c> if this (Player) <see cref="Blip"/> is a friend; otherwise, <c>false</c>.
		/// </value>
		public bool IsFriend
		{
			set
			{
				API.SetBlipFriend(Handle, value);
			}
		}
		/// <summary>
		/// Sets a value indicating whether this (Player) <see cref="Blip"/> is a CREW member. Toggles a half cyan circle on the left side.
		/// </summary>
		/// <value>
		/// <c>true</c> if this (Player) <see cref="Blip"/> is a CREW member; otherwise, <c>false</c>.
		/// </value>
		public bool IsCrew
		{
			set
			{
				API.SetBlipCrew(Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Blip"/> is flashing.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Blip"/> is flashing; otherwise, <c>false</c>.
		/// </value>
		public bool IsFlashing
		{
			get
			{
				return API.IsBlipFlashing(Handle);
			}
			set
			{
				API.SetBlipFlashes(Handle, value);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Blip"/> is on minimap.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Blip"/> is on minimap; otherwise, <c>false</c>.
		/// </value>
		public bool IsOnMinimap
		{
			get
			{
				return API.IsBlipOnMinimap(Handle);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Blip"/> is short range.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Blip"/> is short range; otherwise, <c>false</c>.
		/// </value>
		public bool IsShortRange
		{
			get
			{
				return API.IsBlipShortRange(Handle);
			}
			set
			{
				API.SetBlipAsShortRange(Handle, value);
			}
		}

		/// <summary>
		/// Removes the number label for this <see cref="Blip"/>.
		/// </summary>
		public void RemoveNumberLabel()
		{
			API.HideNumberOnBlip(Handle);
		}

		/// <summary>
		/// Adds a cone for this <see cref="Blip"/>
		/// </summary>
		/// <param name="halfAngle">'Width' of the cone (0.0f - pi)</param>
		/// <param name="distance">Distance of the cone</param>
		/// <param name="heading">Heading of the cone in radians (degrees * pi / 180)</param>
		/// <param name="color">Color of the cone (see <see href="https://docs.fivem.net/docs/game-references/hud-colors/">hud colors</see>/></param>
		/// <param name="unk0">Unknown at this time</param>
		/// <param name="unk1">Unknown at this time</param>
		/// <param name="unk2">Unknown at this time</param>
		/// <param name="unk3">Unknown at this time</param>
		public void ShowCone(float halfAngle, float distance, float heading, int color, float unk0 = -1.0f, float unk1 = 1.0f, float unk2 = 1.0f, int unk3 = 1)
		{
			// SETUP_FAKE_CONE_DATA
			Function.Call((Hash)0xF83D0FEBE75E62C9, Handle, unk0, unk1, halfAngle, unk2, distance, heading, unk3, color);
			API.SetBlipShowCone(Handle, true);
		}

		/// <summary>
		/// Removes the cone for this <see cref="Blip"/>
		/// </summary>
		public void HideCone()
		{
			// REMOVE_FAKE_CONE_DATA
			Function.Call((Hash)0x35A3CD97B2C0A6D2, Handle);
			API.SetBlipShowCone(Handle, false);
		}

		/// <summary>
		/// Removes this <see cref="Blip"/>.
		/// </summary>
		[SecuritySafeCritical]
		public override void Delete()
		{
			_Delete();
		}

		[SecuritySafeCritical]
		private void _Delete()
		{
			int handle = Handle;

			// prevent the game from crashing when an invalid blip handle was somehow set.
			if (API.DoesBlipExist(handle))
			{
				unsafe
				{
					API.RemoveBlip(ref handle);
				}
				Handle = handle;
			}
		}

		public override bool Exists()
		{
			return API.DoesBlipExist(Handle);
		}
		public static bool Exists(Blip blip)
		{
			return !ReferenceEquals(blip, null) && blip.Exists();
		}

		public bool Equals(Blip blip)
		{
			return !ReferenceEquals(blip, null) && Handle == blip.Handle;
		}
		public override bool Equals(object obj)
		{
			return !ReferenceEquals(obj, null) && obj.GetType() == GetType() && Equals((Blip)obj);
		}

		public sealed override int GetHashCode()
		{
			return Handle.GetHashCode();
		}

		public static bool operator ==(Blip left, Blip right)
		{
			return ReferenceEquals(left, null) ? ReferenceEquals(right, null) : left.Equals(right);
		}
		public static bool operator !=(Blip left, Blip right)
		{
			return !(left == right);
		}
	}
}
