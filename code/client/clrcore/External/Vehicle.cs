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
				return GetModelDisplayName(Model);
			}
		}
		/// <summary>
		/// Gets the localized name of this <see cref="Vehicle"/>
		/// </summary>
		public string LocalizedName
		{
			get
			{
				return Game.GetGXTEntry(API.GetDisplayNameFromVehicleModel((uint)Model.Hash));
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
				return (VehicleClass)API.GetVehicleClass(Handle);
			}
		}

		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/>s body health.
		/// </summary>
		public float BodyHealth
		{
			get
			{
				return API.GetVehicleBodyHealth(Handle);
			}
			set
			{
				API.SetVehicleBodyHealth(Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/> engine health.
		/// </summary>
		public float EngineHealth
		{
			get
			{
				return API.GetVehicleEngineHealth(Handle);
			}
			set
			{
				API.SetVehicleEngineHealth(Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/> petrol tank health.
		/// </summary>
		public float PetrolTankHealth
		{
			get
			{
				return API.GetVehiclePetrolTankHealth(Handle);
			}
			set
			{
				API.SetVehiclePetrolTankHealth(Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets this <see cref="Vehicle"/> fuel level.
		/// </summary>
		public float FuelLevel
		{
			get
			{
				return API.GetVehicleFuelLevel(Handle);
			}
			set
			{
				API.SetVehicleFuelLevel(Handle, value);
			}
		}

		public float OilLevel
		{
			get
			{
				return API.GetVehicleOilLevel(Handle);
			}
			set
			{
				API.SetVehicleOilLevel(Handle, value);
			}
		}

		public float Gravity
		{
			get
			{
				return API.GetVehicleGravityAmount(Handle);
			}
			set
			{
				API.SetVehicleGravityAmount(Handle, value);
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
			get => API.GetIsVehicleEngineRunning(Handle);
			set => API.SetVehicleEngineOn(Handle, value, true, true);
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
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}
				return API.IsVehicleEngineStarting(Handle);
			}
			set
			{
				if ((IsEngineStarting || IsEngineRunning) && value)
				{
					return;
				}
				API.SetVehicleEngineOn(Handle, value, value ? false : true, true);
			}
		}
		/// <summary>
		/// Turns this <see cref="Vehicle"/>s radio on or off
		/// </summary>
		public bool IsRadioEnabled
		{
			set
			{
				API.SetVehicleRadioEnabled(Handle, value);
			}
			get
			{
				// not the greatest way, but at least it's something...
				if (Game.PlayerPed.IsInVehicle(this))
				{
					return API.IsPlayerVehicleRadioEnabled();
				}
				return false;
			}

		}
		/// <summary>
		/// Sets this <see cref="Vehicle"/>s radio station.
		/// </summary>
		public RadioStation RadioStation
		{
			set
			{
				// you need to enable it first before chaning the station to prevent your script from crashing.
				API.SetVehicleRadioEnabled(Handle, true);

				if (value == RadioStation.RadioOff)
				{
					API.SetVehRadioStation(Handle, "OFF");
				}
				else if (Enum.IsDefined(typeof(RadioStation), value))
				{
					API.SetVehRadioStation(Handle, Game._radioNames[(int)value]);
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
				return API.GetEntitySpeed(Handle);
			}
			set
			{
				if (Model.IsTrain)
				{
					API.SetTrainSpeed(Handle, value);
					API.SetTrainCruiseSpeed(Handle, value);
				}
				else
				{
					API.SetVehicleForwardSpeed(Handle, value);
				}
			}
		}
		/// <summary>
		/// Gets the speed the drive wheels are turning at, This is the value used for the dashboard speedometers(after being converted to mph).
		/// </summary>
		public float WheelSpeed
		{
			get => API.GetVehicleDashboardSpeed(Handle);
		}
		/// <summary>
		/// Gets the acceleration of this <see cref="Vehicle"/>.
		/// </summary>
		public float Acceleration
		{
			get => API.GetVehicleCurrentAcceleration(Handle);
		}
		/// <summary>
		/// Gets or sets the current RPM of this <see cref="Vehicle"/>.
		/// </summary>
		/// <value>
		/// The current RPM between <c>0.0f</c> and <c>1.0f</c>.
		/// </value>
		public float CurrentRPM
		{
			get => API.GetVehicleCurrentRpm(Handle);
			set
			{
				API.SetVehicleCurrentRpm(Handle, value);
			}
		}

		public byte HighGear
		{
			get => (byte)API.GetVehicleHighGear(Handle);
			set => API.SetVehicleHighGear(Handle, value);
		}
		/// <summary>
		/// Gets the current gear this <see cref="Vehicle"/> is using.
		/// </summary>
		public int CurrentGear => API.GetVehicleCurrentGear(Handle);

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
				return API.GetVehicleSteeringAngle(Handle);
			}
			set
			{
				API.SetVehicleSteeringAngle(Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets the steering scale of this <see cref="Vehicle"/>.
		/// </summary>
		public float SteeringScale
		{
			get => API.GetVehicleSteeringScale(Handle);
			set => API.SetVehicleSteeringScale(Handle, value);
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
				API.SetVehicleAlarm(Handle, value);
			}

			get => API.IsVehicleAlarmSet(Handle);
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
				return API.IsVehicleAlarmActivated(Handle);
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
			get => API.GetVehicleAlarmTimeLeft(Handle);
			set => API.SetVehicleAlarmTimeLeft(Handle, value);
		}
		/// <summary>
		/// Starts sounding the alarm on this <see cref="Vehicle"/>.
		/// </summary>
		public void StartAlarm()
		{
			API.StartVehicleAlarm(Handle);
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
				return API.IsVehicleSirenOn(Handle);
			}
			set
			{
				API.SetVehicleSiren(Handle, value);
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
				API.DisableVehicleImpactExplosionActivation(Handle, value);
			}
		}
		/// <summary>
		/// Sounds the horn on this <see cref="Vehicle"/>.
		/// </summary>
		/// <param name="duration">The duration to sound the horn for.</param>
		public void SoundHorn(int duration)
		{
			API.StartVehicleHorn(Handle, duration, (uint)Game.GenerateHash("HELDDOWN"), false);
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Vehicle"/> is wanted by the police.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Vehicle"/> is wanted by the police; otherwise, <c>false</c>.
		/// </value>
		public bool IsWanted
		{
			get => API.IsVehicleWanted(Handle);
			set
			{
				API.SetVehicleIsWanted(Handle, value);
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
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				// 1290
				int offset = 0x8D4;

				return MemoryAccess.IsBitSet(MemoryAddress + offset, 2);
			}
			set
			{
				API.SetVehicleProvidesCover(Handle, value);
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
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}
				int offset = 0xB58;

				if (MemoryAccess.ReadInt(MemoryAddress + offset) <= 8)
				{
					return MemoryAccess.IsBitSet(MemoryAddress + 0x1409, 1);
				}
				return false;
			}
			set
			{
				API.SetVehicleCreatesMoneyPickupsWhenExploded(Handle, value);
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
			get => API.IsVehiclePreviouslyOwnedByPlayer(Handle);
			set
			{
				API.SetVehicleHasBeenOwnedByPlayer(Handle, value);
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
			get => API.IsVehicleNeedsToBeHotwired(Handle);
			set
			{
				API.SetVehicleNeedsToBeHotwired(Handle, value);
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
			get
			{
				bool lightState1 = false, lightState2 = false;

				API.GetVehicleLightsState(Handle, ref lightState1, ref lightState2);

				return lightState1;
			}
			set
			{
				API.SetVehicleLights(Handle, value ? 3 : 4);
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
			get
			{
				bool lightState1 = false, lightState2 = false;

				API.GetVehicleLightsState(Handle, ref lightState1, ref lightState2);

				return lightState2;
			}
			set
			{
				API.SetVehicleFullbeam(Handle, value);
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
			get => API.IsVehicleInteriorLightOn(Handle);
			set
			{
				API.SetVehicleInteriorlight(Handle, value);
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
				return API.IsVehicleSearchlightOn(Handle);
			}
			set
			{
				API.SetVehicleSearchlight(Handle, value, false);
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
				return API.IsTaxiLightOn(Handle);
			}
			set
			{
				API.SetTaxiLights(Handle, value);
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
				API.SetVehicleIndicatorLights(Handle, 1, value);
			}
			get
			{
				int val = API.GetVehicleIndicatorLights(Handle);
				if (val == 1 || val == 3)
				{
					return true;
				}
				return false;
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
				API.SetVehicleIndicatorLights(Handle, 0, value);
			}
			get
			{
				int val = API.GetVehicleIndicatorLights(Handle);
				if (val >= 2)
				{
					return true;
				}
				return false;
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
				API.SetVehicleHandbrake(Handle, value);
			}
			get
			{
				return API.GetVehicleHandbrake(Handle);
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
				API.SetVehicleBrakeLights(Handle, value);
			}
		}
		public float LightsMultiplier
		{
			set
			{
				API.SetVehicleLightMultiplier(Handle, value);
			}
		}

		public bool CanBeVisiblyDamaged
		{
			set
			{
				API.SetVehicleCanBeVisiblyDamaged(Handle, value);
			}
		}

		public bool IsDamaged
		{
			get
			{
				return API.IsVehicleDamaged(Handle);
			}
		}
		public bool IsDriveable
		{
			get
			{
				return API.IsVehicleDriveable(Handle, false);
			}
			set
			{
				API.SetVehicleUndriveable(Handle, !value);
			}
		}
		/// <summary>
		/// Gets whether or not the engine is on fire and losing health rapdily.
		/// </summary>
		public bool IsEngineOnFire
		{
			get
			{
				return API.IsVehicleEngineOnFire(Handle);
			}
		}
		public bool HasRoof
		{
			get
			{
				return API.DoesVehicleHaveRoof(Handle);
			}
		}
		public bool IsLeftHeadLightBroken
		{
			get
			{
				return API.GetIsLeftVehicleHeadlightDamaged(Handle);
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
				return API.GetIsRightVehicleHeadlightDamaged(Handle);
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
				return API.IsVehicleBumperBrokenOff(Handle, false);
			}
		}
		public bool IsFrontBumperBrokenOff
		{
			get
			{
				return API.IsVehicleBumperBrokenOff(Handle, true);
			}
		}

		public bool IsAxlesStrong
		{
			set
			{
				API.SetVehicleHasStrongAxles(Handle, value);
			}
		}

		public bool CanEngineDegrade
		{
			set
			{
				API.SetVehicleEngineCanDegrade(Handle, value);
			}
		}
		public float EnginePowerMultiplier
		{
			set
			{
				API.SetVehicleEnginePowerMultiplier(Handle, value);
			}
		}
		public float EngineTorqueMultiplier
		{
			set
			{
				API.SetVehicleEngineTorqueMultiplier(Handle, value);
			}
		}

		public VehicleLandingGearState LandingGearState
		{
			get
			{
				return (VehicleLandingGearState)API.GetLandingGearState(Handle);
			}
			set
			{
				API.SetVehicleLandingGear(Handle, (int)value);
			}
		}
		public VehicleRoofState RoofState
		{
			get
			{
				return (VehicleRoofState)API.GetConvertibleRoofState(Handle);
			}
			set
			{
				switch (value)
				{
					case VehicleRoofState.Closed:
						API.RaiseConvertibleRoof(Handle, true);
						API.RaiseConvertibleRoof(Handle, false);
						break;
					case VehicleRoofState.Closing:
						API.RaiseConvertibleRoof(Handle, false);
						break;
					case VehicleRoofState.Opened:
						API.LowerConvertibleRoof(Handle, true);
						API.LowerConvertibleRoof(Handle, false);
						break;
					case VehicleRoofState.Opening:
						API.LowerConvertibleRoof(Handle, false);
						break;
				}
			}
		}
		public VehicleLockStatus LockStatus
		{
			get
			{
				return (VehicleLockStatus)API.GetVehicleDoorLockStatus(Handle);
			}
			set
			{
				API.SetVehicleDoorsLocked(Handle, (int)value);
			}
		}

		public float MaxBraking
		{
			get
			{
				return API.GetVehicleMaxBraking(Handle);
			}
		}
		public float MaxTraction
		{
			get
			{
				return API.GetVehicleMaxTraction(Handle);
			}
		}

		public bool IsOnAllWheels
		{
			get
			{
				return API.IsVehicleOnAllWheels(Handle);
			}
		}

		public bool IsStopped
		{
			get
			{
				return API.IsVehicleStopped(Handle);
			}
		}
		public bool IsStoppedAtTrafficLights
		{
			get
			{
				return API.IsVehicleStoppedAtTrafficLights(Handle);
			}
		}

		public bool IsStolen
		{
			get
			{
				return API.IsVehicleStolen(Handle);
			}
			set
			{
				API.SetVehicleIsStolen(Handle, value);
			}
		}

		public bool IsConvertible
		{
			get
			{
				return API.IsVehicleAConvertible(Handle, false);
			}
		}

		public bool IsBurnoutForced
		{
			set
			{
				API.SetVehicleBurnout(Handle, value);
			}
		}
		public bool IsInBurnout
		{
			get
			{
				return API.IsVehicleInBurnout(Handle);
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
				return API.GetVehicleMaxNumberOfPassengers(Handle);
			}
		}
		public int PassengerCount
		{
			get
			{
				return API.GetVehicleNumberOfPassengers(Handle);
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
			return API.DoesExtraExist(Handle, extra);
		}
		public bool IsExtraOn(int extra)
		{
			return ExtraExists(extra) ? API.IsVehicleExtraTurnedOn(Handle, extra) : false;
		}
		public void ToggleExtra(int extra, bool toggle)
		{
			if (ExtraExists(extra)) API.SetVehicleExtra(Handle, extra, !toggle);
		}

		public Ped GetPedOnSeat(VehicleSeat seat)
		{
			return new Ped(API.GetPedInVehicleSeat(Handle, (int)seat));
		}
		public bool IsSeatFree(VehicleSeat seat)
		{
			return API.IsVehicleSeatFree(Handle, (int)seat);
		}

		public void Wash()
		{
			DirtLevel = 0f;
		}
		public float DirtLevel
		{
			get
			{
				return API.GetVehicleDirtLevel(Handle);
			}
			set
			{
				API.SetVehicleDirtLevel(Handle, value);
			}
		}

		public bool PlaceOnGround()
		{
			return API.SetVehicleOnGroundProperly(Handle);
		}
		public void PlaceOnNextStreet()
		{
			Vector3 currentPosition = Position;
			Vector3 newPosition = new Vector3();
			float heading = 0f;
			int unkn = 0;

			for (int i = 1; i < 40; i++)
			{
				API.GetNthClosestVehicleNodeWithHeading(currentPosition.X, currentPosition.Y, currentPosition.Z, i, ref newPosition, ref heading, ref unkn, 1, 3f, 0f);
				if (!API.IsPointObscuredByAMissionEntity(newPosition.X, newPosition.Y, newPosition.Z, 5f, 5f, 5f, 0))
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
			API.SetVehicleFixed(Handle);
		}
		public void Explode()
		{
			API.ExplodeVehicle(Handle, true, false);
		}
		public void ExplodeNetworked()
		{
			API.NetworkExplodeVehicle(Handle, true, false, false);
		}

		public bool CanTiresBurst
		{
			get
			{
				return API.GetVehicleTyresCanBurst(Handle);
			}
			set
			{
				API.SetVehicleTyresCanBurst(Handle, value);
			}
		}
		public bool CanWheelsBreak
		{
			set
			{
				API.SetVehicleWheelsCanBreak(Handle, value);
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
				API.OpenBombBayDoors(Handle);
			}
		}
		public void CloseBombBay()
		{
			if (HasBombBay)
			{
				API.CloseBombBayDoors(Handle);
			}
		}

		public void SetHeliYawPitchRollMult(float mult)
		{
			if (Model.IsHelicopter && mult >= 0 && mult <= 1)
			{
				API.SetHelicopterRollPitchYawMult(Handle, mult);
			}
		}

		public void DropCargobobHook(CargobobHook hook)
		{
			if (Model.IsCargobob)
			{
				API.EnableCargobobHook(Handle, (int)hook);
			}
		}
		public void RetractCargobobHook()
		{
			if (Model.IsCargobob)
			{
				API.RetractCargobobHook(Handle);
			}
		}
		public bool IsCargobobHookActive()
		{
			if (Model.IsCargobob)
			{
				return API.IsCargobobHookActive(Handle) || API.IsCargobobMagnetActive(Handle);
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
						return API.IsCargobobHookActive(Handle);
					case CargobobHook.Magnet:
						return API.IsCargobobMagnetActive(Handle);
				}
			}

			return false;
		}
		public void CargoBobMagnetGrabVehicle()
		{
			if (IsCargobobHookActive(CargobobHook.Magnet))
			{
				API.CargobobMagnetGrabVehicle(Handle, true);
			}
		}
		public void CargoBobMagnetReleaseVehicle()
		{
			if (IsCargobobHookActive(CargobobHook.Magnet))
			{
				API.CargobobMagnetGrabVehicle(Handle, false);
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
				API.SetTowTruckCraneHeight(Handle, value);
			}
		}
		public Vehicle TowedVehicle
		{
			get
			{
				return new Vehicle(API.GetEntityAttachedToTowTruck(Handle));
			}
		}
		public void TowVehicle(Vehicle vehicle, bool rear)
		{
			API.AttachVehicleToTowTruck(Handle, vehicle.Handle, rear, 0f, 0f, 0f);
		}
		public void DetachFromTowTruck()
		{
			API.DetachVehicleFromAnyTowTruck(Handle);
		}
		public void DetachTowedVehicle()
		{
			Vehicle vehicle = TowedVehicle;

			if (Exists(vehicle))
			{
				API.DetachVehicleFromTowTruck(Handle, vehicle.Handle);
			}
		}

		public void Deform(Vector3 position, float damageAmount, float radius)
		{
			API.SetVehicleDamage(Handle, position.X, position.Y, position.Z, damageAmount, radius, false);
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

			return new Ped(API.CreatePedInsideVehicle(Handle, 26, (uint)model.Hash, (int)seat, true, true));
		}
		public Ped CreateRandomPedOnSeat(VehicleSeat seat)
		{
			if (!IsSeatFree(seat))
			{
				throw new ArgumentException("The VehicleSeat selected was not free", "seat");
			}
			if (seat == VehicleSeat.Driver)
			{
				return new Ped(API.CreateRandomPedAsDriver(Handle, true));
			}
			else
			{
				int pedHandle = API.CreateRandomPed(0f, 0f, 0f);
				API.SetPedIntoVehicle(pedHandle, Handle, (int)seat);

				return new Ped(pedHandle);
			}
		}

		public static string GetModelDisplayName(Model vehicleModel)
		{
			return API.GetDisplayNameFromVehicleModel((uint)vehicleModel.Hash);
		}

		public static VehicleClass GetModelClass(Model vehicleModel)
		{
			return (VehicleClass)API.GetVehicleClassFromName((uint)vehicleModel.Hash);
		}

		public static string GetClassDisplayName(VehicleClass vehicleClass)
		{
			return "VEH_CLASS_" + ((int)vehicleClass).ToString();
		}

		public static VehicleHash[] GetAllModelsOfClass(VehicleClass vehicleClass)
		{
			return Array.ConvertAll<int, VehicleHash>(MemoryAccess.VehicleModels[(int)vehicleClass].ToArray(), item => (VehicleHash)item);
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
			return base.Exists() && API.GetEntityType(Handle) == 2;
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

		/// <summary>
		/// Get closest vehicle at the specified coords
		/// </summary>
		/// <param name="Coords"></param>
		/// <returns></returns>
		public static Tuple<float, Vehicle> GetClosestVehicle(Vector3 Coords)
		{
			Vehicle[] vehs = World.GetAllVehicles();
			float closestDistance = -1f;
			Vehicle closestVehicle = null;
			foreach (Vehicle v in vehs)
			{
				Vector3 vehcoords = v.Position;
				float distance = World.GetDistance(vehcoords, Coords);
				if (closestDistance == -1 || closestDistance > distance)
				{
					closestVehicle = v;
					closestDistance = distance;
				}
			}
			return new Tuple<float, Vehicle>(closestDistance, closestVehicle);
		}

	}
}
