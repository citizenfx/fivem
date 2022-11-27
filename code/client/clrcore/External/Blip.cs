using System;
using System.Security;

#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;

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
		PoliceOfficer = 3,
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
		PolicePlayer,
		CriminalWanted,
		PoliceStation = 60,
		Hospital,
		Elevator = 63,
		Helicopter,
		StrangersAndFreaks = 66,
		ArmoredTruck,
		TowTruck,
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
		SmallCircle,
		Nigel,
		AssaultRifle,
		Bat,
		Grenade,
		PlusSymbol,
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
		GunCar = 229,
		Custody = 237,
		ArmsTraffickingAir = 251,
		Fairground = 266,
		PropertyManagement,
		Altruist = 269,
		Enemy,
		Chop = 273,
		Dead,
		DollarSignSmall = 276,
		Hooker = 279,
		Friend,
		CarSteal = 289,
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
		MartinMadrazo = 352,
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
		ParachuteBackpack,
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
		SmallSquare,
		Creator = 398,
		CreatorDirection,
		Abigail,
		Blimp,
		Repair,
		Testosterone,
		Dinghy,
		Fanatic,
		Invisible,
		Information,
		CaptureBriefcase,
		LastTeamStanding,
		Boat,
		CaptureHouse,
		BadlyDrawnCaptureBriefcase = 413,
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
		Insurgent,
		Speedboat,
		Heist,
		Skull,
		Stopwatch,
		DollarSignCircled,
		Crosshair2,
		SmallCrosshair2,
		DollarSignSquared,
		RaceFinishBottomRight,
		Fire,
		DoubleSkull,
		SmallCrown = 439,
		DeadDrop,
		CrossArrows,
		Beast,
		CrossTheLinePointer,
		CrossTheLine,
		Lamar,
		Bennys,
		LamarMission1,
		LamarMission2,
		LamarMission3,
		LamarMission4,
		LamarMission5,
		LamarMission6,
		LamarMission7,
		LamarMission8,
		Yacht,
		SmallQuestionMark,
		Briefcase,
		ExecutiveSearch,
		Signal,
		TurretedLimo,
		MarkedShield,
		YachtLocation,
		SmallBeast,
		Hourglass,
		VerySmallQuestionMark,
		SlowTime,
		Flipped,
		ThermalVision,
		SmallMarijuana2,
		Railgun,
		Seashark,
		Dark,
		Warehouse,
		WarehouseForSale,
		Office,
		OfficeForSale,
		Truck,
		SpecialCargo,
		Trailer,
		Person,
		Cargobob,
		SquareOutline,
		Jammed,
		Ghost,
		Detonator,
		Bomb,
		Shield,
		Stunt,
		SmallHeart,
		StuntPremium,
		Adversary,
		Clubhouse,
		CagedIn,
		CircledBike,
		Joust,
		Marijuana2,
		Cocaine,
		FakeID,
		Erlenmeyer,
		Money,
		Package,
		CaptureNumber1,
		CaptureNumber2,
		CaptureNumber3,
		CaptureNumber4,
		CaptureNumber5,
		CaptureNumber6,
		CaptureNumber7,
		CaptureNumber8,
		CaptureNumber9,
		CaptureNumber10,
		QuadBike,
		Bus,
		DrugsPackage,
		Hop,
		Adversary4,
		Adversary8,
		Adversary10,
		Adversary12,
		Adversary16,
		Laptop,
		Deadline,
		SportsCar,
		VehicleWarehouse,
		RegistrationPapers,
		PoliceStation2,
		Junkyard,
		PhantomWedge,
		ArmoredBoxville,
		Ruiner2000,
		RampBuggy,
		Wastelander,
		RocketVoltic,
		TechnicalAqua,
		TargetA,
		TargetB,
		TargetC,
		TargetD,
		TargetE,
		TargetF,
		TargetG,
		TargetH,
		Juggernaut,
		Wrench,
		SteeringWheel,
		Trophy,
		RocketBoost,
		HomingRocket,
		MachineGun,
		Parachute,
		Seconds5,
		Seconds10,
		Seconds15,
		Seconds20,
		Seconds30,
		Ammo,
		Bunker,
		APC,
		Opressor,
		HalfTrack,
		DuneFAV,
		WeaponizedTampa,
		AntiAircraftTrailer,
		MobileOperationsCenter,
		AdversaryBunker,
		BunkerVehicleWorkshop,
		WeaponWorkshop,
		Cargo,
		MilitaryHangar,
		TransformCheckpoint,
		RaceTransform,
		AlphaZ1,
		Bombushka,
		Havok,
		HowardNX25,
		Hunter,
		Ultralight,
		Mogul,
		V65Molotok,
		P45Nokota,
		Pyro,
		Rogue,
		Starling,
		Seabreeze,
		Tula,
		Equipment,
		Treasure,
		OrbitalCannon,
		Avenger,
		Facility,
		HeistUpperCorner,
		SAMTurret,
		Firewall,
		Node,
		Stromberg,
		Deluxo,
		Thruster,
		Khanjali,
		RCV,
		Volatol,
		Barrage,
		Akula,
		Chernobog,
		SmallCCTV,
		StarterPackIdentifier,
		TurretStation,
		RotatingMirror,
		StaticMirror,
		Proxy,
		TargetAssault,
		SuperSportCircuit,
		SeaSparrow,
		Caracara,
		Nightclub,
		CargoCrate,
		Van,
		Jewel,
		Gold,
		Keypad,
		HackTarget,
		Heart,
		BlastIncrease,
		BlastDecrease,
		BombIncrease,
		BombDecrease,
		Rival,
		Drone,
		CashRegister,
		CCTV,
		FestivalBus = 631,
		Terrorbyte,
		Menacer,
		Scramjet,
		PounderCustom,
		MuleCustom,
		SpeedoCustom,
		Blimp2,
		OpressorMkII,
		B11Strikeforce,
		ArenaSeries,
		ArenaPremium,
		ArenaWorkshop,
		RaceArenaWar,
		ArenaTurret,
		RCVehicle,
		RCWorkshop,
		FirePit,
		Flipper,
		SeaMine,
		TurnTable,
		Pit,
		Mines,
		BarrelBomb,
		RisingWall,
		Bollards,
		SideBollard,
		Bruiser,
		Brutus,
		Cerberus,
		Deathbike,
		Dominator,
		Impaler,
		Imperator,
		Issi,
		Sasquatch,
		Scarab,
		Slamvan,
		ZR380,
		ArenaPoints,
		HardcoreComicStore,
		CopCar,
		RCBanditoTimeTrials,
		Crown,
		ThreeCrowns,
		Backpack,
		ShippingContainer,
		Agatha,
		Casino,
		TableGames,
		LuckyWheel,
		Information2,
		Chips,
		HorseRacing,
		AdversaryFeatured,
		RouletteNumber1,
		RouletteNumber2,
		RouletteNumber3,
		RouletteNumber4,
		RouletteNumber5,
		RouletteNumber6,
		RouletteNumber7,
		RouletteNumber8,
		RouletteNumber9,
		RouletteNumber10,
		RouletteNumber11,
		RouletteNumber12,
		RouletteNumber13,
		RouletteNumber14,
		RouletteNumber15,
		RouletteNumber16,
		RouletteNumber17,
		RouletteNumber18,
		RouletteNumber19,
		RouletteNumber20,
		RouletteNumber21,
		RouletteNumber22,
		RouletteNumber23,
		RouletteNumber24,
		RouletteNumber25,
		RouletteNumber26,
		RouletteNumber27,
		RouletteNumber28,
		RouletteNumber29,
		RouletteNumber30,
		RouletteNumber31,
		RouletteNumber32,
		RouletteNumber33,
		RouletteNumber34,
		RouletteNumber35,
		RouletteNumber36,
		RouletteNumber0,
		RouletteNumber00,
		Limo,
		AlienWeapon,
		OpenWheel,
		RappelPoint,
		Swap,
		ScubaGear,
		ControlPanel1,
		ControlPanel2,
		ControlPanel3,
		ControlPanel4,
		SnowTruck,
		Buggy1,
		Buggy2,
		Zhaba,
		Gerald,
		Ron,
		Arcade,
		DroneControls,
		RCTank,
		Stairs,
		Camera2,
		Winky,
		Minisub,
		RetroKart,
		ModernKart,
		MilitaryQuad,
		MilitaryTruck,
		ShipWheel,
		UFO,
		Sparrow,
		WeaponizedDinghy,
		PatrolBoat,
		RetroSportsCar,
		Squadee,
		FoldingWingJet,
		Valkyrie2,
		Kosatka,
		BoltCutters,
		GrapplingEquipment,
		Keycard,
		Codes,
		CayoPericoEquipment,
		BeachParty,
		ControlTower,
		DrainageTunnel,
		PowerStation,
		MainGate,
		RappelPoint2,
		Keypad2,
		SubControls,
		SubPeriscope,
		SubMissile,
		Painting,
		LSCarMeet,
		SteeringWheel2,
		AutoShop,
		Anchor,
		Gift,
		TestCar,
		JobBoard,
		RobberyEquipment,
		StreetRaceSeries,
		PursuitSeries,
		Illuminati = 788,
		BountyCollectibles,
		MovieCollectibles,
		TrailerRamp,
		RaceOrganizer,
		VehicleList,
		ExportVehicle,
		Train,
		Slamvan2 = 799,
		Crusader,
		ConstructionOutfit,
		JammedLightning,
		CayoPericoHeist,
		DiamondCasinoHeist,
		DoomsdayHeist,
		HSWRaceSeries,
		HSWTimeTrial,
		HSWTestVehicle,
		RockstarGames,
		VehicleForSale,
		CarKeys,
		SUV,
		SecurityContract,
		Safe,
		Raymond,
		Eugene,
		Payphone,
		PatriolMilSpec,
		RecordAStudios,
		Jubilee,
		Granger3600LX,
		SatchelCharge,
		Deity,
		DewbaucheeChampion,
		BuffaloSTX,
		Agency,
		BikerBar,
		SimeonOverlay,
		JunkEnergySkydive,
		LuxuryAutos,
		CarShowroom,
		SimeonCarShowroom,
		FlamingSkull,
		WeaponAmmo,
		Rockstar,
		CayoPericoSeries,
		ClubhouseContract,
		AgentULP
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
