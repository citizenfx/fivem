using CitizenFX.Core.Native;
using System;
using System.Security;

namespace CitizenFX.Core
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
				return Function.Call<Vector3>(Hash.GET_BLIP_INFO_ID_COORD, Handle);
			}
			set
			{
				Function.Call(Hash.SET_BLIP_COORDS, Handle, value.X, value.Y, value.Z);
			}
		}
		/// <summary>
		/// Sets the rotation of this <see cref="Blip"/> on the map.
		/// </summary>
		public int Rotation
		{
			set
			{
				Function.Call(Hash.SET_BLIP_ROTATION, Handle, value);
			}
		}
		/// <summary>
		/// Sets the scale of this <see cref="Blip"/> on the map.
		/// </summary>
		public float Scale
		{
			set
			{
				Function.Call(Hash.SET_BLIP_SCALE, Handle, value);
			}
		}

		/// <summary>
		/// Gets the type of this <see cref="Blip"/>.
		/// </summary>
		public int Type
		{
			get
			{
				return Function.Call<int>(Hash.GET_BLIP_INFO_ID_TYPE, Handle);
			}
		}
		/// <summary>
		/// Gets or sets the alpha of this <see cref="Blip"/> on the map.
		/// </summary>
		public int Alpha
		{
			get
			{
				return Function.Call<int>(Hash.GET_BLIP_ALPHA, Handle);
			}
			set
			{
				Function.Call(Hash.SET_BLIP_ALPHA, Handle, value);
			}
		}
		/// <summary>
		/// Sets the priority of this <see cref="Blip"/>.
		/// </summary>
		public int Priority
		{
			set
			{
				Function.Call(Hash.SET_BLIP_PRIORITY, Handle, value);
			}
		}
		/// <summary>
		/// Sets this <see cref="Blip"/>s label to the given number.
		/// </summary>
		public int NumberLabel
		{
			set
			{
				Function.Call(Hash.SHOW_NUMBER_ON_BLIP, Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets the color of this <see cref="Blip"/>.
		/// </summary>
		public BlipColor Color
		{
			get
			{
				return (BlipColor)Function.Call<int>(Hash.GET_BLIP_COLOUR, Handle);
			}
			set
			{
				Function.Call(Hash.SET_BLIP_COLOUR, Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets the sprite of this <see cref="Blip"/>.
		/// </summary>
		public BlipSprite Sprite
		{
			get
			{
				return (BlipSprite)Function.Call<int>(Hash.GET_BLIP_SPRITE, Handle);
			}
			set
			{
				Function.Call(Hash.SET_BLIP_SPRITE, Handle, value);
			}
		}
		/// <summary>
		/// Sets this <see cref="Blip"/>s label to the given string.
		/// </summary>
		public string Name
		{
			[SecuritySafeCritical]
			set
			{
				Function.Call(Hash.BEGIN_TEXT_COMMAND_SET_BLIP_NAME, MemoryAccess.StringPtr);
				Function.Call(Hash.ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, value);
				Function.Call(Hash.END_TEXT_COMMAND_SET_BLIP_NAME, Handle);
			}
		}
		/// <summary>
		/// Gets the <see cref="Entity"/> this <see cref="Blip"/> is attached to.
		/// </summary>
		public Entity Entity
		{
			get
			{
				return Function.Call<Entity>(Hash.GET_BLIP_INFO_ID_ENTITY_INDEX, Handle);
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
				Function.Call(Hash.SET_BLIP_ROUTE, Handle, value);
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
				Function.Call(Hash.SET_BLIP_AS_FRIENDLY, Handle, value);
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
				return Function.Call<bool>(Hash.IS_BLIP_FLASHING, Handle);
			}
			set
			{
				Function.Call(Hash.SET_BLIP_FLASHES, Handle, value);
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
				return Function.Call<bool>(Hash.IS_BLIP_ON_MINIMAP, Handle);
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
				return Function.Call<bool>(Hash.IS_BLIP_SHORT_RANGE, Handle);
			}
			set
			{
				Function.Call(Hash.SET_BLIP_AS_SHORT_RANGE, Handle, value);
			}
		}

		/// <summary>
		/// Removes the number label for this <see cref="Blip"/>.
		/// </summary>
		public void RemoveNumberLabel()
		{
			Function.Call(Hash.HIDE_NUMBER_ON_BLIP, Handle);
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
			unsafe
			{
				Function.Call(Hash.REMOVE_BLIP, &handle);
			}
			Handle = handle;
		}

		public override bool Exists()
		{
			return Function.Call<bool>(Hash.DOES_BLIP_EXIST, Handle);
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
