using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using CitizenFX.Core.Native;
using System.Security;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
	public enum CargobobHook
	{
		Hook,
		Magnet
	}
	public enum LicensePlateStyle
	{
		BlueOnWhite1 = 3,
		BlueOnWhite2 = 0,
		BlueOnWhite3 = 4,
		YellowOnBlack = 1,
		YellowOnBlue = 2,		
		NorthYankton = 5
	}
	public enum LicensePlateType
	{
		FrontAndRearPlates,
		FrontPlate,
		RearPlate,
		None
	}
	public enum VehicleClass
	{
		Compacts,
		Sedans,
		SUVs,
		Coupes,
		Muscle,
		SportsClassics,
		Sports,
		Super,
		Motorcycles,
		OffRoad,
		Industrial,
		Utility,
		Vans,
		Cycles,
		Boats,
		Helicopters,
		Planes,
		Service,
		Emergency,
		Military,
		Commercial,
		Trains
	}
	public enum VehicleColor
	{
		MetallicBlack,
		MetallicGraphiteBlack,
		MetallicBlackSteel,
		MetallicDarkSilver,
		MetallicSilver,
		MetallicBlueSilver,
		MetallicSteelGray,
		MetallicShadowSilver,
		MetallicStoneSilver,
		MetallicMidnightSilver,
		MetallicGunMetal,
		MetallicAnthraciteGray,
		MatteBlack,
		MatteGray,
		MatteLightGray,
		UtilBlack,
		UtilBlackPoly,
		UtilDarksilver,
		UtilSilver,
		UtilGunMetal,
		UtilShadowSilver,
		WornBlack,
		WornGraphite,
		WornSilverGray,
		WornSilver,
		WornBlueSilver,
		WornShadowSilver,
		MetallicRed,
		MetallicTorinoRed,
		MetallicFormulaRed,
		MetallicBlazeRed,
		MetallicGracefulRed,
		MetallicGarnetRed,
		MetallicDesertRed,
		MetallicCabernetRed,
		MetallicCandyRed,
		MetallicSunriseOrange,
		MetallicClassicGold,
		MetallicOrange,
		MatteRed,
		MatteDarkRed,
		MatteOrange,
		MatteYellow,
		UtilRed,
		UtilBrightRed,
		UtilGarnetRed,
		WornRed,
		WornGoldenRed,
		WornDarkRed,
		MetallicDarkGreen,
		MetallicRacingGreen,
		MetallicSeaGreen,
		MetallicOliveGreen,
		MetallicGreen,
		MetallicGasolineBlueGreen,
		MatteLimeGreen,
		UtilDarkGreen,
		UtilGreen,
		WornDarkGreen,
		WornGreen,
		WornSeaWash,
		MetallicMidnightBlue,
		MetallicDarkBlue,
		MetallicSaxonyBlue,
		MetallicBlue,
		MetallicMarinerBlue,
		MetallicHarborBlue,
		MetallicDiamondBlue,
		MetallicSurfBlue,
		MetallicNauticalBlue,
		MetallicBrightBlue,
		MetallicPurpleBlue,
		MetallicSpinnakerBlue,
		MetallicUltraBlue,
		UtilDarkBlue = 75,
		UtilMidnightBlue,
		UtilBlue,
		UtilSeaFoamBlue,
		UtilLightningBlue,
		UtilMauiBluePoly,
		UtilBrightBlue,
		MatteDarkBlue,
		MatteBlue,
		MatteMidnightBlue,
		WornDarkBlue,
		WornBlue,
		WornLightBlue,
		MetallicTaxiYellow,
		MetallicRaceYellow,
		MetallicBronze,
		MetallicYellowBird,
		MetallicLime,
		MetallicChampagne,
		MetallicPuebloBeige,
		MetallicDarkIvory,
		MetallicChocoBrown,
		MetallicGoldenBrown,
		MetallicLightBrown,
		MetallicStrawBeige,
		MetallicMossBrown,
		MetallicBistonBrown,
		MetallicBeechwood,
		MetallicDarkBeechwood,
		MetallicChocoOrange,
		MetallicBeachSand,
		MetallicSunBleechedSand,
		MetallicCream,
		UtilBrown,
		UtilMediumBrown,
		UtilLightBrown,
		MetallicWhite,
		MetallicFrostWhite,
		WornHoneyBeige,
		WornBrown,
		WornDarkBrown,
		WornStrawBeige,
		BrushedSteel,
		BrushedBlackSteel,
		BrushedAluminium,
		Chrome,
		WornOffWhite,
		UtilOffWhite,
		WornOrange,
		WornLightOrange,
		MetallicSecuricorGreen,
		WornTaxiYellow,
		PoliceCarBlue,
		MatteGreen,
		MatteBrown,
		MatteWhite = 131,
		WornWhite,
		WornOliveArmyGreen,
		PureWhite,
		HotPink,
		Salmonpink,
		MetallicVermillionPink,
		Orange,
		Green,
		Blue,
		MettalicBlackBlue,
		MetallicBlackPurple,
		MetallicBlackRed,
		HunterGreen,
		MetallicPurple,
		MetaillicVDarkBlue,
		ModshopBlack1,
		MattePurple,
		MatteDarkPurple,
		MetallicLavaRed,
		MatteForestGreen,
		MatteOliveDrab,
		MatteDesertBrown,
		MatteDesertTan,
		MatteFoliageGreen,
		DefaultAlloyColor,
		EpsilonBlue,
		PureGold,
		BrushedGold
	}
	public enum VehicleLandingGearState
	{
		Deployed,
		Closing,
		Opening,
		Retracted
	}
	public enum VehicleLockStatus
	{
		None,
		Unlocked,
		Locked,
		LockedForPlayer,
		StickPlayerInside,
		CanBeBrokenInto = 7,
		CanBeBrokenIntoPersist,
		CannotBeTriedToEnter = 10
	}
	public enum VehicleNeonLight
	{
		Left,
		Right,
		Front,
		Back
	}
	public enum VehicleRoofState
	{
		Closed,
		Opening,
		Opened,
		Closing
	}
	public enum VehicleSeat
	{
		None = -3,
		Any,
		Driver,
		Passenger,
		LeftFront = -1,
		RightFront,
		LeftRear,
		RightRear,
		ExtraSeat1,
		ExtraSeat2,
		ExtraSeat3,
		ExtraSeat4,
		ExtraSeat5,
		ExtraSeat6,
		ExtraSeat7,
		ExtraSeat8,
		ExtraSeat9,
		ExtraSeat10,
		ExtraSeat11,
		ExtraSeat12
	}
	public enum VehicleWindowTint
	{
		None,
		PureBlack,
		DarkSmoke,
		LightSmoke,
		Stock,
		Limo,
		Green
	}

	public sealed class Vehicle : Entity
	{
		#region Fields
		VehicleDoorCollection _doors;
		VehicleModCollection _mods;
		VehicleWheelCollection _wheels;
		VehicleWindowCollection _windows;
		#endregion

		public Vehicle(int handle) : base(handle)
		{
		}

		/// <summary>
		/// Gets the display name of this <see cref="Vehicle"/>.
		/// <remarks>Use <see cref="Game.GetGXTEntry(string)"/> to get the localized name.</remarks>
		/// </summary>
		public string DisplayName
		{
			get
			{
				return GetModelDisplayName(base.Model);
			}
		}
		/// <summary>
		/// Gets the localized name of this <see cref="Vehicle"/>
		/// </summary>
		public string LocalizedName
		{
			get
			{
				return Game.GetGXTEntry(Function.Call<ulong>(Hash.GET_DISPLAY_NAME_FROM_VEHICLE_MODEL, base.Model));
			}
		}

		/// <summary>
		/// Gets the display name of this <see cref="Vehicle"/>s <see cref="VehicleClass"/>.
		/// <remarks>Use <see cref="Game.GetGXTEntry(string)"/> to get the localized class name.</remarks>
		/// </summary>
		public string ClassDisplayName
		{
			get
			{
				return GetClassDisplayName(ClassType);
			}
		}

		/// <summary>
		/// Gets the localized name of this <see cref="Vehicle"/>s <see cref="VehicleClass"/>.
		/// </summary>
		public string ClassLocalizedName
		{
			get
			{
				return Game.GetGXTEntry(ClassDisplayName);
			}
		}

		/// <summary>
		/// Gets the class of this <see cref="Vehicle"/>.
		/// </summary>
		public VehicleClass ClassType
		{
			get
			{
				return Function.Call<VehicleClass>(Hash.GET_VEHICLE_CLASS, Handle);
			}
		}

		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/>s body health.
		/// </summary>
		public float BodyHealth
		{
			get
			{
				return Function.Call<float>(Hash.GET_VEHICLE_BODY_HEALTH, Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_BODY_HEALTH, Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/> engine health.
		/// </summary>
		public float EngineHealth
		{
			get
			{
				return Function.Call<float>(Hash.GET_VEHICLE_ENGINE_HEALTH, Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_ENGINE_HEALTH, Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/> petrol tank health.
		/// </summary>
		public float PetrolTankHealth
		{
			get
			{
				return Function.Call<float>(Hash.GET_VEHICLE_PETROL_TANK_HEALTH, Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_PETROL_TANK_HEALTH, Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/> fuel level.
		/// </summary>
		public float FuelLevel
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x768 : 0x758;
				offset = Game.Version >= GameVersion.v1_0_877_1_Steam ? 0x788 : offset;

				return MemoryAccess.ReadFloat(MemoryAddress + offset);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x768 : 0x758;
				offset = Game.Version >= GameVersion.v1_0_877_1_Steam ? 0x788 : offset;

				MemoryAccess.WriteFloat(MemoryAddress + offset, value);
			}
		}

		public float OilLevel
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x76C : 0x75C;
				offset = Game.Version >= GameVersion.v1_0_877_1_Steam ? 0x78C : offset;

				return MemoryAccess.ReadFloat(MemoryAddress + offset);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x76C : 0x75C;
				offset = Game.Version >= GameVersion.v1_0_877_1_Steam ? 0x78C : offset;

				MemoryAccess.WriteFloat(MemoryAddress + offset, value);
			}
		}

		public float Gravity
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}
				
				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x0B2C : 0x0B1C;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x0B4C : offset;

				return MemoryAccess.ReadFloat(MemoryAddress + offset);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x0B2C : 0x0B1C;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x0B4C : offset;

				MemoryAccess.WriteFloat(MemoryAddress + offset, value);
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/>s engine is running.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Vehicle"/>s engine is running; otherwise, <c>false</c>.
		/// </value>
		public bool IsEngineRunning
		{
			get
			{
				return Function.Call<bool>(Hash.GET_IS_VEHICLE_ENGINE_RUNNING, Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_ENGINE_ON, Handle, value, true);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/>s engine is currently starting.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Vehicle"/>s engine is starting; otherwise, <c>false</c>.
		/// </value>
		public bool IsEngineStarting
		{
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return false;
				}
				//Unsure of the exact version this switched, but all others in the rangs are the same
				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x842 : 0x832;

				return MemoryAccess.IsBitSet(MemoryAddress + offset, 5);
			}
		}
		/// <summary>
		/// Turns this <see cref="Vehicle"/>s radio on or off
		/// </summary>
		public bool IsRadioEnabled
		{
			set
			{
				Function.Call(Hash.SET_VEHICLE_RADIO_ENABLED, Handle, value);
			}
		}
		/// <summary>
		/// Sets this <see cref="Vehicle"/>s radio station.
		/// </summary>
		public RadioStation RadioStation
		{
			set
			{
				if (value == RadioStation.RadioOff)
				{
					Function.Call(Hash.SET_VEH_RADIO_STATION, "OFF");
				}
				else if (Enum.IsDefined(typeof(RadioStation), value))
				{
					Function.Call(Hash.SET_VEH_RADIO_STATION, Game._radioNames[(int)value]);
				}
			}
		}

		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/>s speed.
		/// </summary>
		/// <value>
		/// The speed in m/s.
		/// </value>
		public float Speed
		{
			get
			{
				return Function.Call<float>(Hash.GET_ENTITY_SPEED, Handle);
			}
			set
			{
				if (Model.IsTrain)
				{
					Function.Call(Hash.SET_TRAIN_SPEED, Handle, value);
					Function.Call(Hash.SET_TRAIN_CRUISE_SPEED, Handle, value);
				}
				else
				{
					Function.Call(Hash.SET_VEHICLE_FORWARD_SPEED, Handle, value);
				}
			}
		}
		/// <summary>
		/// Gets the speed the drive wheels are turning at, This is the value used for the dashboard speedometers(after being converted to mph).
		/// </summary>
		public float WheelSpeed
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}

				//old game version hasnt been tested, just following the patterns above for old game ver
				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x9A4 : 0x994;
				offset = Game.Version >= GameVersion.v1_0_877_1_Steam ? 0x9C4 : offset;

				return MemoryAccess.ReadFloat(MemoryAddress + offset);
			}
		}
		/// <summary>
		/// Gets the acceleration of this <see cref="Vehicle"/>.
		/// </summary>
		public float Acceleration
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x7E4 : 0x7D4;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x804 : offset;

				return MemoryAccess.ReadFloat(MemoryAddress + offset);
			}
		}
		/// <summary>
		/// Gets or sets the current RPM of this <see cref="Vehicle"/>.
		/// </summary>
		/// <value>
		/// The current RPM between <c>0.0f</c> and <c>1.0f</c>.
		/// </value>
		public float CurrentRPM
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x7D4 : 0x7C4;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x7F4 : offset;

				return MemoryAccess.ReadFloat(MemoryAddress + offset);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x7D4 : 0x7C4;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x7F4 : offset;

				MemoryAccess.WriteFloat(MemoryAddress + offset, value);
			}
		}

		public byte HighGear
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x7A6 : 0x796;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x7C6 : offset;

				return MemoryAccess.ReadByte(MemoryAddress + offset);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x7A6 : 0x796;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x7C6 : offset;

				MemoryAccess.WriteByte(MemoryAddress + offset, value);
			}
		}
		/// <summary>
		/// Gets the current gear this <see cref="Vehicle"/> is using.
		/// </summary>
		public int CurrentGear
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x7A0 : 0x790;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x7C0 : offset;

				return (int)MemoryAccess.ReadByte(MemoryAddress + offset);
			}
		}

		/// <summary>
		/// Gets the steering angle of this <see cref="Vehicle"/>.
		/// </summary>
		/// <value>
		/// The steering angle in degrees.
		/// </value>
		public float SteeringAngle
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x8AC : 0x89C;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x8CC : offset;

				return (float)(MemoryAccess.ReadFloat(MemoryAddress + offset) * (180.0 / System.Math.PI));
			}
		}
		/// <summary>
		/// Gets or sets the steering scale of this <see cref="Vehicle"/>.
		/// </summary>
		public float SteeringScale
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x8A4 : 0x894;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x8C4 : offset;

				return MemoryAccess.ReadFloat(MemoryAddress + offset);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x8A4 : 0x894;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x8C4 : offset;

				MemoryAccess.WriteFloat(MemoryAddress + offset, value);
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Vehicle"/> has forks.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has forks; otherwise, <c>false</c>.
		/// </value>
		public bool HasForks
		{
			get
			{
				return Bones.HasBone("forks");
			}
		}

		/// <summary>
		/// Sets a value indicating whether this <see cref="Vehicle"/> has an alarm set.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has an alarm set; otherwise, <c>false</c>.
		/// </value>
		public bool IsAlarmSet
		{
			set
			{
				Function.Call(Hash.SET_VEHICLE_ALARM, Handle, value);
			}

			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 2456 : 2440;

				return (ushort)MemoryAccess.ReadShort(MemoryAddress + offset) == ushort.MaxValue; //if the value is 0xFFFF, the alarm is set
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Vehicle"/> is sounding its alarm.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> is sounding its alarm; otherwise, <c>false</c>.
		/// </value>
		public bool IsAlarmSounding
		{
			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_ALARM_ACTIVATED, Handle);
			}
		}
		/// <summary>
		/// Gets or sets time left before this <see cref="Vehicle"/> alarm stops.
		/// If greater than zero, the vehicle alarm will be sounding.
		/// the value is up to 65534.
		/// </summary>
		/// <value>
		/// The time left before this <see cref="Vehicle"/> alarm stops.
		/// </value>
		public int AlarmTimeLeft
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 2456 : 2440;

				ushort alarmTime = (ushort)MemoryAccess.ReadShort(MemoryAddress + offset);
				if (alarmTime == ushort.MaxValue)
				{
					return 0;
				}

				return (int) alarmTime;
			}
			set
			{
				ushort alarmTime = (ushort)value;
				if (alarmTime == ushort.MaxValue)
				{
					return;
				}

				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 2456 : 2440;

				MemoryAccess.WriteShort(MemoryAddress + offset, (short)alarmTime);
			}
		}
		/// <summary>
		/// Starts sounding the alarm on this <see cref="Vehicle"/>.
		/// </summary>
		public void StartAlarm()
		{
			Function.Call(Hash.START_VEHICLE_ALARM, Handle);
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Vehicle"/> has a siren.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has a siren; otherwise, <c>false</c>.
		/// </value>
		public bool HasSiren
		{
			get
			{
				return Bones.HasBone("siren1");
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> has its siren turned on.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has its siren turned on; otherwise, <c>false</c>.
		/// </value>
		public bool IsSirenActive
		{
			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_SIREN_ON, Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_SIREN, Handle, value);
			}
		}
		/// <summary>
		/// Sets a value indicating whether the siren on this <see cref="Vehicle"/> plays sounds.
		/// </summary>
		/// <value>
		/// <c>true</c> if the siren on this <see cref="Vehicle"/> plays sounds; otherwise, <c>false</c>.
		/// </value>
		public bool IsSirenSilent
		{
			set
			{
				// Sets if the siren is silent actually 
				Function.Call(Hash.DISABLE_VEHICLE_IMPACT_EXPLOSION_ACTIVATION, Handle, value);
			}
		}
		/// <summary>
		/// Sounds the horn on this <see cref="Vehicle"/>.
		/// </summary>
		/// <param name="duration">The duration to sound the horn for.</param>
		public void SoundHorn(int duration)
		{
			Function.Call(Hash.START_VEHICLE_HORN, Handle, duration, Game.GenerateHash("HELDDOWN"), 0);
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> is wanted by the police.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> is wanted by the police; otherwise, <c>false</c>.
		/// </value>
		public bool IsWanted
		{
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return false;
				}
				//Unsure of the exact version this switched, but all others in the rangs are the same
				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x84C : 0x83C;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x86C : offset;

				return MemoryAccess.IsBitSet(MemoryAddress + offset, 3);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_IS_WANTED, Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether peds can use this <see cref="Vehicle"/> for cover.
		/// </summary>
		/// <value>
		///   <c>true</c> if peds can use this <see cref="Vehicle"/> for cover; otherwise, <c>false</c>.
		/// </value>
		public bool ProvidesCover
		{
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return false;
				}
				//Unsure of the exact version this switched, but all others in the rangs are the same
				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x83C : 0x82C;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x85C : offset;

				return MemoryAccess.IsBitSet(MemoryAddress + offset, 2);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_PROVIDES_COVER, Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> drops money when destroyed.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Vehicle"/> drops money when destroyed; otherwise, <c>false</c>.
		/// </value>
		public bool DropsMoneyOnExplosion
		{		   
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return false;
				}
				//Unsure of the exact version this switched or if it switched over a few title updates
				//as its shifted by 0x20 bytes where as rest are only 0x10 bytes
				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0xA98 : 0xA78;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0xAC8 : offset; // just a guess

				if (MemoryAccess.ReadInt(memoryAddress + offset) <= 8)
				{
					return MemoryAccess.IsBitSet(memoryAddress + 0x12F9, 1);
				}
				return false;
			}
			set
			{
				Function.Call(Hash._SET_VEHICLE_CREATES_MONEY_PICKUPS_WHEN_EXPLODED, Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> was previously owned by a <see cref="Player"/>.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Vehicle"/> was previously owned by a <see cref="Player"/>; otherwise, <c>false</c>.
		/// </value>
		public bool PreviouslyOwnedByPlayer
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x844 : 0x834;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x864 : offset;

				return MemoryAccess.IsBitSet(MemoryAddress + offset, 1);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_HAS_BEEN_OWNED_BY_PLAYER, Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> needs to be hotwired to start.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> needs to be hotwired to start; otherwise, <c>false</c>.
		/// </value>
		public bool NeedsToBeHotwired
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x844 : 0x834;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x864 : offset;

				return MemoryAccess.IsBitSet(MemoryAddress + offset, 2);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_NEEDS_TO_BE_HOTWIRED, Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> has its lights on.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has its lights on; otherwise, <c>false</c>.
		/// </value>
		public bool AreLightsOn
		{
            [SecuritySafeCritical]
            get
			{
				bool lightState1, lightState2;
				unsafe
				{
					Function.Call(Hash.GET_VEHICLE_LIGHTS_STATE, Handle, &lightState1, &lightState2);
				}

				return lightState1;
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_LIGHTS, Handle, value ? 3 : 4);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> has its high beams on.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has its high beams on; otherwise, <c>false</c>.
		/// </value>
		public bool AreHighBeamsOn
		{
            [SecuritySafeCritical]
            get
			{
				bool lightState1, lightState2;
				unsafe
				{
					Function.Call(Hash.GET_VEHICLE_LIGHTS_STATE, Handle, &lightState1, &lightState2);
				}

				return lightState2;
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_FULLBEAM, Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> has its interior lights on.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has its interior lights on; otherwise, <c>false</c>.
		/// </value>
		public bool IsInteriorLightOn
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x841 : 0x831;
				offset = Game.Version > GameVersion.v1_0_877_1_Steam ? 0x861 : offset;

				return MemoryAccess.IsBitSet(MemoryAddress + offset, 6);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_INTERIORLIGHT, Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> has its search light on.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has its search light on; otherwise, <c>false</c>.
		/// </value>
		public bool IsSearchLightOn
		{
			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_SEARCHLIGHT_ON, Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_SEARCHLIGHT, Handle, value, 0);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> has its taxi light on.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has its taxi light on; otherwise, <c>false</c>.
		/// </value>
		public bool IsTaxiLightOn
		{
			get
			{
				return Function.Call<bool>(Hash.IS_TAXI_LIGHT_ON, Handle);
			}
			set
			{
				Function.Call(Hash.SET_TAXI_LIGHTS, Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> has its left indicator light on.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has its left indicator light on; otherwise, <c>false</c>.
		/// </value>
		public bool IsLeftIndicatorLightOn
		{
			set
			{
				Function.Call(Hash.SET_VEHICLE_INDICATOR_LIGHTS, Handle, true, value);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> has its right indicator light on.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has its right indicator light on; otherwise, <c>false</c>.
		/// </value>
		public bool IsRightIndicatorLightOn
		{
			set
			{
				Function.Call(Hash.SET_VEHICLE_INDICATOR_LIGHTS, Handle, false, value);
			}
		}
		/// <summary>
		/// Sets a value indicating whether the Handbrake on this <see cref="Vehicle"/> is forced on.
		/// </summary>
		/// <value>
		///   <c>true</c> if the Handbrake on this <see cref="Vehicle"/> is forced on; otherwise, <c>false</c>.
		/// </value>
		public bool IsHandbrakeForcedOn
		{
			set
			{
				Function.Call(Hash.SET_VEHICLE_HANDBRAKE, Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> has its brake light on.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> has its brake light on; otherwise, <c>false</c>.
		/// </value>
		public bool AreBrakeLightsOn
		{
			set
			{
				Function.Call(Hash.SET_VEHICLE_BRAKE_LIGHTS, Handle, value);
			}
		}
		public float LightsMultiplier
		{
			set
			{
				Function.Call(Hash.SET_VEHICLE_LIGHT_MULTIPLIER, Handle, value);
			}
		}

		public bool CanBeVisiblyDamaged
		{
			set
			{
				Function.Call(Hash.SET_VEHICLE_CAN_BE_VISIBLY_DAMAGED, Handle, value);
			}
		}

		public bool IsDamaged
		{
			get
			{
				return Function.Call<bool>(Hash._IS_VEHICLE_DAMAGED, Handle);
			}
		}
		public bool IsDriveable
		{
			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_DRIVEABLE, Handle, 0);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_UNDRIVEABLE, Handle, !value);
			}
		}
		public bool HasRoof
		{
			get
			{
				return Function.Call<bool>(Hash.DOES_VEHICLE_HAVE_ROOF, Handle);
			}
		}
		public bool IsLeftHeadLightBroken
		{
			get
			{
				return Function.Call<bool>(Hash.GET_IS_LEFT_VEHICLE_HEADLIGHT_DAMAGED, Handle);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				IntPtr address = MemoryAddress + 1916;

				if (value)
				{
					MemoryAccess.SetBit(address, 0);
				}
				else
				{
					MemoryAccess.ClearBit(address, 0);
				}
			}
		}
		public bool IsRightHeadLightBroken
		{
			get
			{
				return Function.Call<bool>(Hash.GET_IS_RIGHT_VEHICLE_HEADLIGHT_DAMAGED, Handle);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				IntPtr address = MemoryAddress + 1916;

				if (value)
				{
					MemoryAccess.SetBit(address, 1);
				}
				else
				{
					MemoryAccess.ClearBit(address, 1);
				}
			}
		}
		public bool IsRearBumperBrokenOff
		{
			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_BUMPER_BROKEN_OFF, Handle, false);
			}
		}
		public bool IsFrontBumperBrokenOff
		{
			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_BUMPER_BROKEN_OFF, Handle, true);
			}
		}

		public bool IsAxlesStrong
		{
			set
			{
				Function.Call<bool>(Hash.SET_VEHICLE_HAS_STRONG_AXLES, Handle, value);
			}
		}

		public bool CanEngineDegrade
		{
			set
			{
				Function.Call(Hash.SET_VEHICLE_ENGINE_CAN_DEGRADE, Handle, value);
			}
		}
		public float EnginePowerMultiplier
		{
			set
			{
				Function.Call(Hash._SET_VEHICLE_ENGINE_POWER_MULTIPLIER, Handle, value);
			}
		}
		public float EngineTorqueMultiplier
		{
			set
			{
				Function.Call(Hash._SET_VEHICLE_ENGINE_TORQUE_MULTIPLIER, Handle, value);
			}
		}

		public VehicleLandingGearState LandingGearState
		{
			get
			{
				return Function.Call<VehicleLandingGearState>(Hash.GET_LANDING_GEAR_STATE, Handle);
			}
			set
			{
				Function.Call(Hash._SET_VEHICLE_LANDING_GEAR, Handle, value);
			}
		}
		public VehicleRoofState RoofState
		{
			get
			{
				return Function.Call<VehicleRoofState>(Hash.GET_CONVERTIBLE_ROOF_STATE, Handle);
			}
			set
			{
				switch (value)
				{
					case VehicleRoofState.Closed:
						Function.Call(Hash.RAISE_CONVERTIBLE_ROOF, Handle, true);
						Function.Call(Hash.RAISE_CONVERTIBLE_ROOF, Handle, false);
						break;
					case VehicleRoofState.Closing:
						Function.Call(Hash.RAISE_CONVERTIBLE_ROOF, Handle, false);
						break;
					case VehicleRoofState.Opened:
						Function.Call(Hash.LOWER_CONVERTIBLE_ROOF, Handle, true);
						Function.Call(Hash.LOWER_CONVERTIBLE_ROOF, Handle, false);
						break;
					case VehicleRoofState.Opening:
						Function.Call(Hash.LOWER_CONVERTIBLE_ROOF, Handle, false);
						break;
				}
			}
		}
		public VehicleLockStatus LockStatus
		{
			get
			{
				return Function.Call<VehicleLockStatus>(Hash.GET_VEHICLE_DOOR_LOCK_STATUS, Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_DOORS_LOCKED, Handle, value);
			}
		}

		public float MaxBraking
		{
			get
			{
				return Function.Call<float>(Hash.GET_VEHICLE_MAX_BRAKING, Handle);
			}
		}
		public float MaxTraction
		{
			get
			{
				return Function.Call<float>(Hash.GET_VEHICLE_MAX_TRACTION, Handle);
			}
		}

		public bool IsOnAllWheels
		{

			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_ON_ALL_WHEELS, Handle);
			}
		}

		public bool IsStopped
		{

			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_STOPPED, Handle);
			}
		}
		public bool IsStoppedAtTrafficLights
		{

			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_STOPPED_AT_TRAFFIC_LIGHTS, Handle);
			}
		}

		public bool IsStolen
		{
			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_STOLEN, Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_IS_STOLEN, Handle, value);
			}
		}

		public bool IsConvertible
		{

			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_A_CONVERTIBLE, Handle, 0);
			}
		}

		public bool IsBurnoutForced
		{
			set
			{
				Function.Call<bool>(Hash.SET_VEHICLE_BURNOUT, Handle, value);
			}
		}
		public bool IsInBurnout
		{
			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_IN_BURNOUT, Handle);
			}
		}

		public Ped Driver
		{
			get
			{
				return GetPedOnSeat(VehicleSeat.Driver);
			}
		}
		public Ped[] Occupants
		{
			get
			{
				Ped driver = Driver;

				if (!Ped.Exists(driver))
				{
					return Passengers;
				}

				var result = new Ped[PassengerCount + 1];
				result[0] = driver;

				for (int i = 0, j = 0, seats = PassengerCapacity; i < seats && j < result.Length; i++)
				{												  
					if (!IsSeatFree((VehicleSeat)i))
					{
						result[j++ + 1] = GetPedOnSeat((VehicleSeat)i);
					}
				}

				return result;
			}
		}
		public Ped[] Passengers
		{
			get
			{
				var result = new Ped[PassengerCount];

				if (result.Length == 0)
				{
					return result;
				}

				for (int i = 0, j = 0, seats = PassengerCapacity; i < seats && j < result.Length; i++)
				{
					if (!IsSeatFree((VehicleSeat)i))
					{
						result[j++] = GetPedOnSeat((VehicleSeat)i);
					}
				}

				return result;
			}
		}
		public int PassengerCapacity
		{
			get
			{
				return Function.Call<int>(Hash.GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS, Handle);
			}
		}
		public int PassengerCount
		{
			get
			{
				return Function.Call<int>(Hash.GET_VEHICLE_NUMBER_OF_PASSENGERS, Handle);
			}
		}

		public VehicleDoorCollection Doors
		{
			get
			{
				if (_doors == null)
				{
					_doors = new VehicleDoorCollection(this);
				}

				return _doors;
			}
		}
		public VehicleModCollection Mods
		{
			get
			{
				if (_mods == null)
				{
					_mods = new VehicleModCollection(this);
				}

				return _mods;
			}
		}
		public VehicleWheelCollection Wheels
		{
			get
			{
				if (_wheels == null)
				{
					_wheels = new VehicleWheelCollection(this);
				}

				return _wheels;
			}
		}
		public VehicleWindowCollection Windows
		{
			get
			{
				if (_windows == null)
				{
					_windows = new VehicleWindowCollection(this);
				}

				return _windows;
			}
		}

		public bool ExtraExists(int extra)
		{
			return Function.Call<bool>(Hash.DOES_EXTRA_EXIST, Handle, extra);
		}
		public bool IsExtraOn(int extra)
		{
			return Function.Call<bool>(Hash.IS_VEHICLE_EXTRA_TURNED_ON, Handle, extra);
		}
		public void ToggleExtra(int extra, bool toggle)
		{
			Function.Call(Hash.SET_VEHICLE_EXTRA, Handle, extra, !toggle);
		}

		public Ped GetPedOnSeat(VehicleSeat seat)
		{
			return new Ped(Function.Call<int>(Hash.GET_PED_IN_VEHICLE_SEAT, Handle, seat));
		}
		public bool IsSeatFree(VehicleSeat seat)
		{
			return Function.Call<bool>(Hash.IS_VEHICLE_SEAT_FREE, Handle, seat);
		}

		public void Wash()
		{
			DirtLevel = 0f;
		}
		public float DirtLevel
		{
			get
			{
				return Function.Call<float>(Hash.GET_VEHICLE_DIRT_LEVEL, Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_DIRT_LEVEL, Handle, value);
			}
		}

		public bool PlaceOnGround()
		{
			return Function.Call<bool>(Hash.SET_VEHICLE_ON_GROUND_PROPERLY, Handle);
		}
        [SecuritySafeCritical]
        public void PlaceOnNextStreet()
		{
			Vector3 currentPosition = Position;
			NativeVector3 newPosition;
			float heading;
			long unkn;

			for (int i = 1; i < 40; i++)
			{
				unsafe
				{
					Function.Call(Hash.GET_NTH_CLOSEST_VEHICLE_NODE_WITH_HEADING, currentPosition.X, currentPosition.Y, currentPosition.Z, i, &newPosition, &heading, &unkn, 1, 0x40400000, 0);
				}


				if (!Function.Call<bool>(Hash.IS_POINT_OBSCURED_BY_A_MISSION_ENTITY, newPosition.X, newPosition.Y, newPosition.Z, 5.0f, 5.0f, 5.0f, 0))
				{
					Position = newPosition;
					PlaceOnGround();
					Heading = heading;
					break;
				}
			}
		}

		public void Repair()
		{
			Function.Call(Hash.SET_VEHICLE_FIXED, Handle);
		}
		public void Explode()
		{
			Function.Call(Hash.EXPLODE_VEHICLE, Handle, true, false);
		}

		public bool CanTiresBurst
		{
			get
			{
				return Function.Call<bool>(Hash.GET_VEHICLE_TYRES_CAN_BURST, Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_TYRES_CAN_BURST, Handle, value);
			}
		}
		public bool CanWheelsBreak
		{
			set
			{
				Function.Call(Hash.SET_VEHICLE_WHEELS_CAN_BREAK, Handle, value);
			}
		}

		public bool HasBombBay
		{
			get
			{
				return Bones.HasBone("door_hatch_l") && Bones.HasBone("door_hatch_r");
			}
		}
		public void OpenBombBay()
		{
			if (HasBombBay)
			{
				Function.Call(Hash.OPEN_BOMB_BAY_DOORS, Handle);
			}
		}
		public void CloseBombBay()
		{
			if (HasBombBay)
			{
				Function.Call(Hash.CLOSE_BOMB_BAY_DOORS, Handle);
			}
		}

		public void SetHeliYawPitchRollMult(float mult)
		{
			if (Model.IsHelicopter && mult >= 0 && mult <= 1)
			{
				Function.Call(Hash._SET_HELICOPTER_ROLL_PITCH_YAW_MULT, Handle, mult);
			}
		}

		public void DropCargobobHook(CargobobHook hook)
		{
			if (Model.IsCargobob)
			{
				Function.Call(Hash._ENABLE_CARGOBOB_HOOK, Handle, hook);
			}
		}
		public void RetractCargobobHook()
		{
			if (Model.IsCargobob)
			{
				Function.Call(Hash._RETRACT_CARGOBOB_HOOK, Handle);
			}
		}
		public bool IsCargobobHookActive()
		{
			if (Model.IsCargobob)
			{
				return Function.Call<bool>(Hash._IS_CARGOBOB_HOOK_ACTIVE, Handle) || Function.Call<bool>(Hash._IS_CARGOBOB_MAGNET_ACTIVE, Handle);
			}

			return false;
		}
		public bool IsCargobobHookActive(CargobobHook hook)
		{
			if (Model.IsCargobob)
			{
				switch (hook)
				{
					case CargobobHook.Hook:
						return Function.Call<bool>(Hash._IS_CARGOBOB_HOOK_ACTIVE, Handle);
					case CargobobHook.Magnet:
						return Function.Call<bool>(Hash._IS_CARGOBOB_MAGNET_ACTIVE, Handle);
				}
			}

			return false;
		}
		public void CargoBobMagnetGrabVehicle()
		{
			if (IsCargobobHookActive(CargobobHook.Magnet))
			{
				Function.Call(Hash._CARGOBOB_MAGNET_GRAB_VEHICLE, Handle, true);
			}
		}
		public void CargoBobMagnetReleaseVehicle()
		{
			if (IsCargobobHookActive(CargobobHook.Magnet))
			{
				Function.Call(Hash._CARGOBOB_MAGNET_GRAB_VEHICLE, Handle, false);
			}
		}

		public bool HasTowArm
		{
			get
			{
				return Bones.HasBone("tow_arm");
			}
		}
		public float TowingCraneRaisedAmount
		{
			set
			{
				Function.Call(Hash._SET_TOW_TRUCK_CRANE_HEIGHT, Handle, value);
			}
		}
		public Vehicle TowedVehicle
		{
			get
			{
				return new Vehicle(Function.Call<int>(Hash.GET_ENTITY_ATTACHED_TO_TOW_TRUCK, Handle));
			}
		}
		public void TowVehicle(Vehicle vehicle, bool rear)
		{
			Function.Call(Hash.ATTACH_VEHICLE_TO_TOW_TRUCK, Handle, vehicle.Handle, rear, 0f, 0f, 0f);
		}
		public void DetachFromTowTruck()
		{
			Function.Call(Hash.DETACH_VEHICLE_FROM_ANY_TOW_TRUCK, Handle);
		}
		public void DetachTowedVehicle()
		{
			Vehicle vehicle = TowedVehicle;

			if (Exists(vehicle))
			{
				Function.Call(Hash.DETACH_VEHICLE_FROM_TOW_TRUCK, Handle, vehicle.Handle);
			}
		}

		public void Deform(Vector3 position, float damageAmount, float radius)
		{
			Function.Call(Hash.SET_VEHICLE_DAMAGE, position.X, position.Y, position.Z, damageAmount, radius);
		}

		public async Task<Ped> CreatePedOnSeat(VehicleSeat seat, Model model)
		{
			if (!IsSeatFree(seat))
			{
				throw new ArgumentException("The VehicleSeat selected was not free", "seat");
			}
			if (!model.IsPed || !await model.Request(1000))
			{
				return null;
			}

			return new Ped(Function.Call<int>(Hash.CREATE_PED_INSIDE_VEHICLE, Handle, 26, model.Hash, seat, 1, 1));
		}
		public Ped CreateRandomPedOnSeat(VehicleSeat seat)
		{
			if (!IsSeatFree(seat))
			{
				throw new ArgumentException("The VehicleSeat selected was not free", "seat");
			}
			if (seat == VehicleSeat.Driver)
			{
				return new Ped(Function.Call<int>(Hash.CREATE_RANDOM_PED_AS_DRIVER, Handle, true));
			}
			else
			{
				int pedHandle = Function.Call<int>(Hash.CREATE_RANDOM_PED, 0f, 0f, 0f);
				Function.Call(Hash.SET_PED_INTO_VEHICLE, pedHandle, Handle, seat);

				return new Ped(pedHandle);
			}
		}

		public static string GetModelDisplayName(Model vehicleModel)
		{
			return Function.Call<string>(Hash.GET_DISPLAY_NAME_FROM_VEHICLE_MODEL, vehicleModel.Hash);
		}

		public static VehicleClass GetModelClass(Model vehicleModel)
		{
			return Function.Call<VehicleClass>(Hash.GET_VEHICLE_CLASS_FROM_NAME, vehicleModel.Hash);
		}

		public static string GetClassDisplayName(VehicleClass vehicleClass)
		{
			return "VEH_CLASS_" + ((int)vehicleClass).ToString();
		}

		public static VehicleHash[] GetAllModelsOfClass(VehicleClass vehicleClass)
		{
			return Array.ConvertAll<int, VehicleHash>(MemoryAccess.VehicleModels[(int) vehicleClass].ToArray(), item => (VehicleHash)item);
		}

		public static VehicleHash[] GetAllModels()
		{
			List<VehicleHash> allModels = new List<VehicleHash>();
			for (int i = 0; i < 0x20; i++)
			{
				allModels.AddRange(Array.ConvertAll<int, VehicleHash>(MemoryAccess.VehicleModels[i].ToArray(), item => (VehicleHash)item));
			}
			return allModels.ToArray();
		}

		/// <summary>
		/// Determines whether this <see cref="Vehicle"/> exists.
		/// </summary>
		/// <returns><c>true</c> if this <see cref="Vehicle"/> exists; otherwise, <c>false</c></returns>
		public new bool Exists()
		{
			return Function.Call<int>(Hash.GET_ENTITY_TYPE, Handle) == 2;
		}
		/// <summary>
		/// Determines whether the <see cref="Vehicle"/> exists.
		/// </summary>
		/// <param name="vehicle">The <see cref="Vehicle"/> to check.</param>
		/// <returns><c>true</c> if the <see cref="Vehicle"/> exists; otherwise, <c>false</c></returns>
		public static bool Exists(Vehicle vehicle)
		{
			return !ReferenceEquals(vehicle, null) && vehicle.Exists();
		}

	}
}
