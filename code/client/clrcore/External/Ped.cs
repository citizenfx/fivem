using System;
using CitizenFX.Core.Native;
using CitizenFX.Core.NaturalMotion;
using System.Security;

namespace CitizenFX.Core
{
	public enum Gender
	{
		Male,
		Female
	}
	public enum DrivingStyle
	{
		None = 0,
		Normal = 786603,
		IgnoreLights = 2883621,
		SometimesOvertakeTraffic = 5,
		Rushed = 1074528293,
		AvoidTraffic = 786468,
		AvoidTrafficExtremely = 6,
		AvoidHighwaysWhenPossible = 536870912,
		IgnorePathing = 16777216,
		IgnoreRoads = 4194304,
		ShortestPath = 262144,
		Backwards = 1024
	}
	[Flags]
	public enum VehicleDrivingFlags : uint
	{
		None = 0,
		FollowTraffic = 1,
		YieldToPeds = 2,
		AvoidVehicles = 4,
		AvoidEmptyVehicles = 8,
		AvoidPeds = 16,
		AvoidObjects = 32,
		StopAtTrafficLights = 128,
		UseBlinkers = 256,
		AllowGoingWrongWay = 512,
		Reverse = 1024,
		AllowMedianCrossing = 262144,
		DriveBySight = 4194304,
		IgnorePathFinding = 16777216,
		TryToAvoidHighways = 536870912,
		StopAtDestination = 2147483648
	}
	public enum HelmetType : uint
	{
		RegularMotorcycleHelmet = 4096u,
		FiremanHelmet = 16384u,
		PilotHeadset = 32768u
	}
	public enum ParachuteLandingType
	{
		None = -1,
		Stumbling = 1,
		Rolling,
		Ragdoll
	}
	public enum ParachuteState
	{
		None = -1,
		FreeFalling,
		Deploying,
		Gliding,
		LandingOrFallingToDoom
	}

	public enum RagdollType
	{
		Normal = 0,
		StiffLegs = 1,
		NarrowLegs = 2,
		WideLegs = 3,
	}

	public enum SpeechModifier
	{
		Standard = 0,
		AllowRepeat = 1,
		Beat = 2,
		Force = 3,
		ForceFrontend = 4,
		ForceNoRepeatFrontend = 5,
		ForceNormal = 6,
		ForceNormalClear = 7,
		ForceNormalCritical = 8,
		ForceShouted = 9,
		ForceShoutedClear = 10,
		ForceShoutedCritical = 11,
		ForcePreloadOnly = 12,
		Megaphone = 13,
		Helicopter = 14,
		ForceMegaphone = 15,
		ForceHelicopter = 16,
		Interrupt = 17,
		InterruptShouted = 18,
		InterruptShoutedClear = 19,
		InterruptShoutedCritical = 20,
		InterruptNoForce = 21,
		InterruptFrontend = 22,
		InterruptNoForceFrontend = 23,
		AddBlip = 24,
		AddBlipAllowRepeat = 25,
		AddBlipForce = 26,
		AddBlipShouted = 27,
		AddBlipShoutedForce = 28,
		AddBlipInterrupt = 29,
		AddBlipInterruptForce = 30,
		ForcePreloadOnlyShouted = 31,
		ForcePreloadOnlyShoutedClear = 32,
		ForcePreloadOnlyShoutedCritical = 33,
		Shouted = 34,
		ShoutedClear = 35,
		ShoutedCritical = 36
	}

	public sealed class Ped : Entity
	{
		#region Fields
		Tasks _tasks;
		Euphoria _euphoria;
		WeaponCollection _weapons;
		Style _style;
		PedBoneCollection _pedBones;

		internal static readonly string[] _speechModifierNames = {
			"SPEECH_PARAMS_STANDARD",
			"SPEECH_PARAMS_ALLOW_REPEAT",
			"SPEECH_PARAMS_BEAT",
			"SPEECH_PARAMS_FORCE",
			"SPEECH_PARAMS_FORCE_FRONTEND",
			"SPEECH_PARAMS_FORCE_NO_REPEAT_FRONTEND",
			"SPEECH_PARAMS_FORCE_NORMAL",
			"SPEECH_PARAMS_FORCE_NORMAL_CLEAR",
			"SPEECH_PARAMS_FORCE_NORMAL_CRITICAL",
			"SPEECH_PARAMS_FORCE_SHOUTED",
			"SPEECH_PARAMS_FORCE_SHOUTED_CLEAR",
			"SPEECH_PARAMS_FORCE_SHOUTED_CRITICAL",
			"SPEECH_PARAMS_FORCE_PRELOAD_ONLY",
			"SPEECH_PARAMS_MEGAPHONE",
			"SPEECH_PARAMS_HELI",
			"SPEECH_PARAMS_FORCE_MEGAPHONE",
			"SPEECH_PARAMS_FORCE_HELI",
			"SPEECH_PARAMS_INTERRUPT",
			"SPEECH_PARAMS_INTERRUPT_SHOUTED",
			"SPEECH_PARAMS_INTERRUPT_SHOUTED_CLEAR",
			"SPEECH_PARAMS_INTERRUPT_SHOUTED_CRITICAL",
			"SPEECH_PARAMS_INTERRUPT_NO_FORCE",
			"SPEECH_PARAMS_INTERRUPT_FRONTEND",
			"SPEECH_PARAMS_INTERRUPT_NO_FORCE_FRONTEND",
			"SPEECH_PARAMS_ADD_BLIP",
			"SPEECH_PARAMS_ADD_BLIP_ALLOW_REPEAT",
			"SPEECH_PARAMS_ADD_BLIP_FORCE",
			"SPEECH_PARAMS_ADD_BLIP_SHOUTED",
			"SPEECH_PARAMS_ADD_BLIP_SHOUTED_FORCE",
			"SPEECH_PARAMS_ADD_BLIP_INTERRUPT",
			"SPEECH_PARAMS_ADD_BLIP_INTERRUPT_FORCE",
			"SPEECH_PARAMS_FORCE_PRELOAD_ONLY_SHOUTED",
			"SPEECH_PARAMS_FORCE_PRELOAD_ONLY_SHOUTED_CLEAR",
			"SPEECH_PARAMS_FORCE_PRELOAD_ONLY_SHOUTED_CRITICAL",
			"SPEECH_PARAMS_SHOUTED",
			"SPEECH_PARAMS_SHOUTED_CLEAR",
			"SPEECH_PARAMS_SHOUTED_CRITICAL",
		};
		#endregion


		public Ped(int handle) : base(handle)
		{
		}

		/// <summary>
		/// Get the headblend data from this <see cref="Ped"/>.
		/// </summary>
		/// <returns>A <see cref="PedHeadBlendData"/> struct containing all headblend data from a mp ped.</returns>
		public PedHeadBlendData GetHeadBlendData()
		{
			return _GetHeadBlendData();
		}

		/// <summary>
		/// Gets the unsafe headblend struct and converts it into a safe struct and returns that struct.
		/// </summary>
		/// <returns>A <see cref="PedHeadBlendData"/> struct.</returns>
		[SecuritySafeCritical]
		private PedHeadBlendData _GetHeadBlendData()
		{
			UnsafePedHeadBlendData data;
			unsafe
			{
				Function.Call(Hash._GET_PED_HEAD_BLEND_DATA, API.PlayerPedId(), &data);
			}
			return data.GetData();
		}

		/// <summary>
		/// Gets or sets how much money this <see cref="Ped"/> is carrying.
		/// </summary>
		public int Money
		{
			get
			{
				return API.GetPedMoney(Handle);
			}
			set
			{
				API.SetPedMoney(Handle, value);
			}
		}
		/// <summary>
		/// Gets the gender of this <see cref="Ped"/>. Note this does not seem to work correctly for all peds.
		/// </summary>
		public Gender Gender
		{
			get
			{
				return API.IsPedMale(Handle) ? Gender.Male : Gender.Female;
			}
		}
		/// <summary>
		/// Gets or sets how much Armor this <see cref="Ped"/> is wearing.
		/// </summary>
		/// <remarks>if you need to get or set the value strictly, use <see cref="ArmorFloat"/> instead.</remarks>
		public int Armor
		{
			get
			{
				return API.GetPedArmour(Handle);
			}
			set
			{
				API.SetPedArmour(Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets how much Armor this <see cref="Ped"/> is wearing in float.
		/// </summary>
		public float ArmorFloat
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x1474 : 0x1464;

				return MemoryAccess.ReadFloat(MemoryAddress + offset);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x1474 : 0x1464;

				MemoryAccess.WriteFloat(MemoryAddress + offset, value);
			}
		}
		/// <summary>
		/// Gets or sets how accurate this <see cref="Ped"/>s shooting ability is.
		/// </summary>
		/// <value>
		/// The accuracy from 0 to 100, 0 being very innacurate, 100 being perfectly accurate.
		/// </value>
		public int Accuracy
		{
			get
			{
				return API.GetPedAccuracy(Handle);
			}
			set
			{
				API.SetPedAccuracy(Handle, value);
			}
		}

		/// <summary>
		/// Opens a list of <see cref="Tasks"/> that this <see cref="Ped"/> can carry out.
		/// </summary>
		public Tasks Task
		{
			get
			{
				if (ReferenceEquals(_tasks, null))
				{
					_tasks = new Tasks(this);
				}
				return _tasks;
			}
		}
		/// <summary>
		/// Gets the stage of the <see cref="TaskSequence"/> this <see cref="Ped"/> is currently executing.
		/// </summary>
		public int TaskSequenceProgress
		{
			get
			{
				return API.GetSequenceProgress(Handle);
			}
		}

		/// <summary>
		/// Opens a list of <see cref="NaturalMotion.Euphoria"/> Helpers which can be applied to this <see cref="Ped"/>.
		/// </summary>
		public Euphoria Euphoria
		{
			get
			{
				if (ReferenceEquals(_euphoria, null))
				{
					_euphoria = new Euphoria(this);
				}
				return _euphoria;
			}
		}

		/// <summary>
		/// Gets a collection of all this <see cref="Ped"/>s <see cref="Weapon"/>s.
		/// </summary>
		public WeaponCollection Weapons
		{
			get
			{
				if (ReferenceEquals(_weapons, null))
				{
					_weapons = new WeaponCollection(this);
				}
				return _weapons;
			}
		}

		/// <summary>
		/// Opens a list of clothing and prop configurations that this <see cref="Ped"/> can wear.
		/// </summary>
		public Style Style
		{
			get
			{
				if (ReferenceEquals(_style, null))
				{
					_style = new Style(this);
				}
				return _style;
			}
		}

		/// <summary>
		/// Gets the vehicle weapon this <see cref="Ped"/> is using.
		/// <remarks>The vehicle weapon, returns <see cref="VehicleWeaponHash.Invalid"/> if this <see cref="Ped"/> isnt using a vehicle weapon.</remarks>
		/// </summary>
		public VehicleWeaponHash VehicleWeapon
		{
			get
			{
				uint hash = 0u;
				if (API.GetCurrentPedVehicleWeapon(Handle, ref hash))
				{
					return (VehicleWeaponHash)hash;
				}
				return VehicleWeaponHash.Invalid;
			}
		}

		/// <summary>
		/// Gets the last <see cref="Vehicle"/> this <see cref="Ped"/> used.
		/// </summary>
		/// <remarks>returns <c>null</c> if the last vehicle doesn't exist.</remarks>
		public Vehicle LastVehicle
		{
			get
			{
				Vehicle veh = new Vehicle(API.GetVehiclePedIsIn(Handle, true));
				return veh.Exists() ? veh : null;
			}
		}
		/// <summary>
		/// Gets the current <see cref="Vehicle"/> this <see cref="Ped"/> is using.
		/// </summary>
		/// <remarks>returns <c>null</c> if this <see cref="Ped"/> isn't in a <see cref="Vehicle"/>.</remarks>
		public Vehicle CurrentVehicle
		{
			get
			{
				Vehicle veh = new Vehicle(API.GetVehiclePedIsIn(Handle, false));
				return veh.Exists() ? veh : null;
			}
		}
		/// <summary>
		/// Gets the <see cref="Vehicle"/> this <see cref="Ped"/> is trying to enter.
		/// </summary>
		/// <remarks>returns <c>null</c> if this <see cref="Ped"/> isn't trying to enter a <see cref="Vehicle"/>.</remarks>
		public Vehicle VehicleTryingToEnter
		{
			get
			{
				Vehicle veh = new Vehicle(API.GetVehiclePedIsTryingToEnter(Handle));
				return veh.Exists() ? veh : null;
			}
		}
		/// <summary>
		/// Gets the PedGroup this <see cref="Ped"/> is in.
		/// </summary>
		public PedGroup PedGroup
		{
			get
			{
				if (!IsInGroup)
				{
					return null;
				}

				return new PedGroup(API.GetPedGroupIndex(Handle));
			}
		}

		/// <summary>
		/// Gets or sets the how much sweat should be rendered on this <see cref="Ped"/>.
		/// </summary>
		/// <value>
		/// The sweat from 0 to 100, 0 being no sweat, 100 being saturated.
		/// </value>
		public float Sweat
		{
			[SecuritySafeCritical]
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0;
				}
				return MemoryAccess.ReadInt(MemoryAddress + 4464);
			}
			set
			{
				if (value < 0)
				{
					value = 0;
				}
				if (value > 100)
				{
					value = 100;
				}

				API.SetPedSweat(Handle, value);
			}
		}
		/// <summary>
		/// Sets how high up on this <see cref="Ped"/>s body water should be visible.
		/// </summary>
		/// <value>
		/// The height ranges from 0.0f to 1.99f, 0.0f being no water visible, 1.99f being covered in water.
		/// </value>
		public float WetnessHeight
		{
			set
			{
				if (value == 0.0f)
				{
					API.ClearPedWetness(Handle);
				}
				else
				{
					API.SetPedWetnessHeight(Handle, value);
				}
			}
		}

		/// <summary>
		/// Sets the voice to use when this <see cref="Ped"/> speaks.
		/// </summary>
		public string Voice
		{
			set
			{
				API.SetAmbientVoiceName(Handle, value);
			}
		}

		/// <summary>
		/// Sets the rate this <see cref="Ped"/> will shoot at.
		/// </summary>
		/// <value>
		/// The shoot rate from 0.0f to 1000.0f, 100.0f is the default value.
		/// </value>
		public int ShootRate
		{
			set
			{
				API.SetPedShootRate(Handle, value);
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Ped"/> was killed by a stealth attack.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Ped"/> was killed by stealth; otherwise, <c>false</c>.
		/// </value>
		public bool WasKilledByStealth
		{
			get
			{
				return API.WasPedKilledByStealth(Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Ped"/> was killed by a takedown.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Ped"/> was killed by a takedown; otherwise, <c>false</c>.
		/// </value>
		public bool WasKilledByTakedown
		{
			get
			{
				return API.WasPedKilledByTakedown(Handle);
			}
		}

		/// <summary>
		/// Gets the <see cref="VehicleSeat"/> this <see cref="Ped"/> is in.
		/// </summary>
		/// <value>
		/// The <see cref="VehicleSeat"/> this <see cref="Ped"/> is in if this <see cref="Ped"/> is in a <see cref="Vehicle"/>; otherwise, <see cref="VehicleSeat.None"/>.
		/// </value>
		public VehicleSeat SeatIndex
		{
			get
			{
				if (!IsInVehicle())
				{
					return VehicleSeat.None;
				}
				for (int seatIndex = -1; seatIndex < API.GetVehicleModelNumberOfSeats((uint)CurrentVehicle.Model.Hash); seatIndex++)
				{
					if (CurrentVehicle.GetPedOnSeat((VehicleSeat)seatIndex).Handle == Handle)
					{
						return (VehicleSeat)seatIndex;
					}
				}
				return VehicleSeat.None;
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Ped"/> is jumping out of their vehicle.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Ped"/> is jumping out of their vehicle; otherwise, <c>false</c>.
		/// </value>
		public bool IsJumpingOutOfVehicle
		{
			get
			{
				return API.IsPedJumpingOutOfVehicle(Handle);
			}
		}
		/// <summary>
		/// Sets a value indicating whether this <see cref="Ped"/> will stay in the vehicle when the driver gets jacked.
		/// </summary>
		/// <value>
		/// <c>true</c> if <see cref="Ped"/> stays in vehicle when jacked; otherwise, <c>false</c>.
		/// </value>
		public bool StaysInVehicleWhenJacked
		{
			set
			{
				API.SetPedStayInVehicleWhenJacked(Handle, value);
			}
		}

		/// <summary>
		/// Sets the maximum driving speed this <see cref="Ped"/> can drive at.
		/// </summary>
		public float MaxDrivingSpeed
		{
			set
			{
				API.SetDriveTaskMaxCruiseSpeed(Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets the injury health threshold for this <see cref="Ped"/>. 
		/// The ped is considered injured when its health drops below this value.
		/// </summary>
		/// <value>
		/// The injury health threshold. Should be below <see cref="Entity.MaxHealth"/>.
		/// </value>
		public float InjuryHealthThreshold
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x1480 : 0x1470;

				return MemoryAccess.ReadFloat(MemoryAddress + offset);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x1480 : 0x1470;

				MemoryAccess.WriteFloat(MemoryAddress + offset, value);
			}
		}

		/// <summary>
		/// Gets or sets the fatal injury health threshold for this <see cref="Ped"/>.
		/// The ped is considered dead when its health drops below this value.
		/// </summary>
		/// <value>
		/// The fatal injury health threshold. Should be below <see cref="Entity.MaxHealth"/>.
		/// </value>
		/// <remarks>
		/// Note on player controlled peds: One of the game scripts will kill the player when their health drops below 100, regardless of this setting.
		/// </remarks>
		public float FatalInjuryHealthThreshold
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x1484 : 0x1474;

				return MemoryAccess.ReadFloat(MemoryAddress + 5248);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x1484 : 0x1474;

				MemoryAccess.WriteFloat(MemoryAddress + offset, value);
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Ped"/> is human.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Ped"/> is human; otherwise, <c>false</c>.
		/// </value>
		public bool IsHuman
		{
			get
			{
				return API.IsPedHuman(Handle);
			}
		}
		public bool IsEnemy
		{
			set
			{
				API.SetPedAsEnemy(Handle, value);
			}
		}
		public bool IsPriorityTargetForEnemies
		{
			set
			{
				API.SetEntityIsTargetPriority(Handle, value, 0);
			}
		}
		public bool IsPlayer
		{
			get
			{
				return API.IsPedAPlayer(Handle);
			}
		}

		public bool IsCuffed
		{
			get
			{
				return API.IsPedCuffed(Handle);
			}
		}
		public bool IsWearingHelmet
		{
			get
			{
				return API.IsPedWearingHelmet(Handle);
			}
		}

		public bool IsRagdoll
		{
			get
			{
				return API.IsPedRagdoll(Handle);
			}
		}

		public bool IsIdle
		{
			get
			{
				return !IsInjured && !IsRagdoll && !IsInAir && !IsOnFire && !IsDucking && !IsGettingIntoAVehicle && !IsInCombat && !IsInMeleeCombat && (!IsInVehicle() || IsSittingInVehicle());
			}
		}
		public bool IsProne
		{
			get
			{
				return API.IsPedProne(Handle);
			}
		}
		public bool IsDucking
		{
			get
			{
				return API.IsPedDucking(Handle);
			}
			set
			{
				API.SetPedDucking(Handle, value);
			}
		}
		public bool IsGettingUp
		{
			get
			{
				return API.IsPedGettingUp(Handle);
			}
		}
		public bool IsClimbing
		{
			get
			{
				return API.IsPedClimbing(Handle);
			}
		}
		public bool IsJumping
		{
			get
			{
				return API.IsPedJumping(Handle);
			}
		}
		public bool IsFalling
		{
			get
			{
				return API.IsPedFalling(Handle);
			}
		}
		public bool IsStopped
		{
			get
			{
				return API.IsPedStopped(Handle);
			}
		}
		public bool IsWalking
		{
			get
			{
				return API.IsPedWalking(Handle);
			}
		}
		public bool IsRunning
		{
			get
			{
				return API.IsPedRunning(Handle);
			}
		}
		public bool IsSprinting
		{
			get
			{
				return API.IsPedSprinting(Handle);
			}
		}
		public bool IsDiving
		{
			get
			{
				return API.IsPedDiving(Handle);
			}
		}
		public bool IsInParachuteFreeFall
		{
			get
			{
				return API.IsPedInParachuteFreeFall(Handle);
			}
		}
		public bool IsSwimming
		{
			get
			{
				return API.IsPedSwimming(Handle);
			}
		}
		public bool IsSwimmingUnderWater
		{
			get
			{
				return API.IsPedSwimmingUnderWater(Handle);
			}
		}
		public bool IsVaulting
		{
			get
			{
				return API.IsPedVaulting(Handle);
			}
		}

		public bool IsOnBike
		{
			get
			{
				return API.IsPedOnAnyBike(Handle);
			}
		}
		public bool IsOnFoot
		{
			get
			{
				return API.IsPedOnFoot(Handle);
			}
		}
		public bool IsInSub
		{
			get
			{
				return API.IsPedInAnySub(Handle);
			}
		}
		public bool IsInTaxi
		{
			get
			{
				return API.IsPedInAnyTaxi(Handle);
			}
		}
		public bool IsInTrain
		{
			get
			{
				return API.IsPedInAnyTrain(Handle);
			}
		}
		public bool IsInHeli
		{
			get
			{
				return API.IsPedInAnyHeli(Handle);
			}
		}
		public bool IsInPlane
		{
			get
			{
				return API.IsPedInAnyPlane(Handle);
			}
		}
		public bool IsInFlyingVehicle
		{
			get
			{
				return API.IsPedInFlyingVehicle(Handle);
			}
		}
		public bool IsInBoat
		{
			get
			{
				return API.IsPedInAnyBoat(Handle);
			}
		}
		public bool IsInPoliceVehicle
		{
			get
			{
				return API.IsPedInAnyPoliceVehicle(Handle);
			}
		}

		public bool IsJacking
		{
			get
			{
				return API.IsPedJacking(Handle);
			}
		}
		public bool IsBeingJacked
		{
			get
			{
				return API.IsPedBeingJacked(Handle);
			}
		}
		public bool IsGettingIntoAVehicle
		{
			get
			{
				return API.IsPedGettingIntoAVehicle(Handle);
			}
		}
		public bool IsTryingToEnterALockedVehicle
		{
			get
			{
				return API.IsPedTryingToEnterALockedVehicle(Handle);
			}
		}

		public bool IsInjured
		{
			get
			{
				return API.IsPedInjured(Handle);
			}
		}
		public bool IsFleeing
		{
			get
			{
				return API.IsPedFleeing(Handle);
			}
		}

		public bool IsInCombat
		{
			get
			{
				return API.IsPedInCombat(Handle, API.PlayerPedId()); // native descriptiong is pretty vague, might need testing.
			}
		}
		public bool IsInMeleeCombat
		{
			get
			{
				return API.IsPedInMeleeCombat(Handle);
			}
		}
		public bool IsInStealthMode
		{
			get
			{
				return API.GetPedStealthMovement(Handle);
			}
		}
		public bool IsAmbientSpeechplaying
		{
			get
			{
				return API.IsAmbientSpeechPlaying(Handle);
			}
		}
		public bool IsScriptedSpeechplaying
		{
			get
			{
				return API.IsScriptedSpeechPlaying(Handle);
			}
		}
		public bool IsAnySpeechplaying
		{
			get
			{
				return API.IsAnySpeechPlaying(Handle);
			}
		}
		public bool IsAmbientSpeechEnabled
		{
			get
			{
				return !API.IsAmbientSpeechDisabled(Handle);
			}
		}
		public bool IsPainAudioEnabled
		{
			set
			{
				API.DisablePedPainAudio(Handle, !value);
			}
		}
		public bool IsPlantingBomb
		{

			get
			{
				return API.IsPedPlantingBomb(Handle);
			}
		}
		public bool IsShooting
		{

			get
			{
				return API.IsPedShooting(Handle);
			}
		}
		public bool IsAiming
		{
			get
			{
				return GetConfigFlag(78);
			}
		}
		public bool IsReloading
		{

			get
			{
				return API.IsPedReloading(Handle);
			}
		}
		public bool IsDoingDriveBy
		{
			get
			{
				return API.IsPedDoingDriveby(Handle);
			}
		}
		public bool IsGoingIntoCover
		{
			get
			{
				return API.IsPedGoingIntoCover(Handle);
			}
		}
		public bool IsBeingStunned
		{
			get
			{
				return API.IsPedBeingStunned(Handle, 0);
			}
		}
		public bool IsBeingStealthKilled
		{
			get
			{
				return API.IsPedBeingStealthKilled(Handle);
			}
		}
		public bool IsPerformingStealthKill
		{
			get
			{
				return API.IsPedPerformingStealthKill(Handle);
			}
		}

		public bool IsAimingFromCover
		{
			get
			{
				return API.IsPedAimingFromCover(Handle);
			}
		}
		public bool IsInCover()
		{
			return IsInCover(false);
		}
		public bool IsInCover(bool expectUseWeapon)
		{
			return API.IsPedInCover(Handle, expectUseWeapon);
		}
		public bool IsInCoverFacingLeft
		{
			get
			{
				return API.IsPedInCoverFacingLeft(Handle);
			}
		}

		public string MovementAnimationSet
		{
			set
			{
				if (value == null)
				{
					API.ResetPedMovementClipset(Handle, 0.25f);
					Task.ClearAll();
				}
				else
				{
					//Movement sets can be applied from anim_dicts and anim_sets(also clip_sets but they use the same native as anim_sets)
					//so check if the string is a valid anim_dict, if so load it as anim dict
					//otherwise load it as an anim_set
					if (API.DoesAnimDictExist(value))
					{
						if (API.HasAnimDictLoaded(value))
						{
							API.RequestAnimDict(value);
						}

						var endtime = DateTime.UtcNow + new TimeSpan(0, 0, 0, 0, 1000);

						// CFX-TODO

						//while (!Function.Call<bool>(Hash.HAS_ANIM_DICT_LOADED, value))
						{
							//Script.Yield();

							if (DateTime.UtcNow >= endtime)
							{
								return;
							}
						}
					}
					else
					{
						API.RequestAnimSet(value);
						var endtime = DateTime.UtcNow + new TimeSpan(0, 0, 0, 0, 1000);

						// CFX-TODO

						//while (!Function.Call<bool>(Hash.HAS_ANIM_SET_LOADED, value))
						{
							//Script.Yield();

							if (DateTime.UtcNow >= endtime)
							{
								return;
							}
						}
					}
					API.SetPedMovementClipset(Handle, value, 0.25f);
				}
			}
		}

		public FiringPattern FiringPattern
		{
			set
			{
				API.SetPedFiringPattern(Handle, (uint)value);
			}
		}
		public ParachuteLandingType ParachuteLandingType
		{
			get
			{
				return (ParachuteLandingType)API.GetPedParachuteLandingType(Handle);
			}
		}
		public ParachuteState ParachuteState
		{
			get
			{
				return (ParachuteState)API.GetPedParachuteState(Handle);
			}
		}

		public bool DropsWeaponsOnDeath
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				int offset = (Game.Version >= GameVersion.v1_0_877_1_Steam ? 0x13E5 : 0x13BD);

				return (MemoryAccess.ReadByte(MemoryAddress + offset) & (1 << 6)) == 0;
			}
			set
			{
				API.SetPedDropsWeaponsWhenDead(Handle, value);
			}
		}

		public float DrivingSpeed
		{
			set
			{
				API.SetDriveTaskCruiseSpeed(Handle, value);
			}
		}
		public DrivingStyle DrivingStyle
		{
			set
			{
				API.SetDriveTaskDrivingStyle(Handle, (int)value);
			}
		}
		public VehicleDrivingFlags VehicleDrivingFlags
		{
			set
			{
				API.SetDriveTaskDrivingStyle(Handle, (int)value);
			}
		}

		public bool CanRagdoll
		{
			get
			{
				return API.CanPedRagdoll(Handle);
			}
			set
			{
				API.SetPedCanRagdoll(Handle, value);
			}
		}
		public bool CanPlayGestures
		{
			set
			{
				API.SetPedCanPlayGestureAnims(Handle, value);
			}
		}
		public bool CanSwitchWeapons
		{
			set
			{
				API.SetPedCanSwitchWeapon(Handle, value);
			}
		}
		public bool CanWearHelmet
		{
			set
			{
				API.SetPedHelmet(Handle, value);
			}
		}
		public bool CanBeTargetted
		{
			set
			{
				API.SetPedCanBeTargetted(Handle, value);
			}
		}
		public bool CanBeShotInVehicle
		{
			set
			{
				API.SetPedCanBeShotInVehicle(Handle, value);
			}
		}
		public bool CanBeDraggedOutOfVehicle
		{
			set
			{
				API.SetPedCanBeDraggedOut(Handle, value);
			}
		}
		public bool CanBeKnockedOffBike
		{
			set
			{
				API.SetPedCanBeKnockedOffVehicle(Handle, value ? 0 : 1);
			}
		}
		public bool CanFlyThroughWindscreen
		{
			get
			{
				return API.GetPedConfigFlag(Handle, 32, true);
			}
			set
			{
				API.SetPedConfigFlag(Handle, 32, value);
			}
		}
		public bool CanSufferCriticalHits
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0x13BC : 0x13AC;
				offset = (Game.Version >= GameVersion.v1_0_877_1_Steam ? 0x13E4 : offset);

				return (MemoryAccess.ReadByte(MemoryAddress + offset) & (1 << 2)) == 0;
			}
			set
			{
				API.SetPedSuffersCriticalHits(Handle, value);
			}
		}
		public bool CanWrithe
		{
			get
			{
				return !GetConfigFlag(281);
			}
			set
			{
				SetConfigFlag(281, !value);
			}
		}
		/// <summary>
		/// Sets whether permanent events are blocked for this <see cref="Ped"/>.
		///  If permanent events are blocked, this <see cref="Ped"/> will only do as it's told, and won't flee when shot at, etc.
		/// </summary>
		/// <value>
		///   <c>true</c> if permanent events are blocked; otherwise, <c>false</c>.
		/// </value>
		public bool BlockPermanentEvents
		{
			set
			{
				API.SetBlockingOfNonTemporaryEvents(Handle, value);
			}
		}

		public bool AlwaysKeepTask
		{
			set
			{
				API.SetPedKeepTask(Handle, value);
			}
		}
		public bool AlwaysDiesOnLowHealth
		{
			set
			{
				API.SetPedDiesWhenInjured(Handle, value);
			}
		}
		public bool DrownsInWater
		{
			set
			{
				API.SetPedDiesInWater(Handle, value);
			}
		}
		public bool DrownsInSinkingVehicle
		{
			set
			{
				API.SetPedDiesInSinkingVehicle(Handle, value);
			}
		}
		public bool DiesInstantlyInWater
		{
			set
			{
				API.SetPedDiesInstantlyInWater(Handle, value);
			}
		}

		public bool IsInVehicle()
		{
			return API.IsPedInAnyVehicle(Handle, false);
		}
		public bool IsInVehicle(Vehicle vehicle)
		{
			return API.IsPedInVehicle(Handle, vehicle.Handle, false);
		}
		public bool IsSittingInVehicle()
		{
			return API.IsPedSittingInAnyVehicle(Handle);
		}
		public bool IsSittingInVehicle(Vehicle vehicle)
		{
			return API.IsPedSittingInVehicle(Handle, vehicle.Handle);
		}
		public void SetIntoVehicle(Vehicle vehicle, VehicleSeat seat)
		{
			API.SetPedIntoVehicle(Handle, vehicle.Handle, (int)seat);
		}

		public Relationship GetRelationshipWithPed(Ped ped)
		{
			return (Relationship)API.GetRelationshipBetweenPeds(Handle, ped.Handle);
		}

		public bool IsHeadtracking(Entity entity)
		{
			return API.IsPedHeadtrackingEntity(Handle, entity.Handle);
		}
		public bool IsInCombatAgainst(Ped target)
		{
			return API.IsPedInCombat(Handle, target.Handle);
		}

		public Ped GetJacker()
		{
			return new Ped(API.GetPedsJacker(Handle));
		}
		public Ped GetJackTarget()
		{
			return new Ped(API.GetJackTarget(Handle));
		}
		public Ped GetMeleeTarget()
		{
			return new Ped(API.GetMeleeTargetForPed(Handle));
		}
		public Entity GetKiller()
		{
			return Entity.FromHandle(API.GetPedSourceOfDeath(Handle));
		}

		public void Kill()
		{
			Health = -1;
		}
		public void Resurrect()
		{
			int maxHealth = MaxHealth;
			bool isCollisionEnabled = IsCollisionEnabled;

			API.ResurrectPed(Handle);
			MaxHealth = maxHealth;
			Health = maxHealth;
			IsCollisionEnabled = isCollisionEnabled;
			API.ClearPedTasksImmediately(Handle);
		}

		public void ResetVisibleDamage()
		{
			API.ResetPedVisibleDamage(Handle);
		}
		public void ClearBloodDamage()
		{
			API.ClearPedBloodDamage(Handle);
		}



		public RelationshipGroup RelationshipGroup
		{
			get
			{
				return new RelationshipGroup(API.GetPedRelationshipGroupHash(Handle));
			}
			set
			{
				API.SetPedRelationshipGroupHash(Handle, (uint)value.Hash);
			}
		}
		public bool IsInGroup
		{
			get
			{
				return API.IsPedInGroup(Handle);
			}
		}
		public bool NeverLeavesGroup
		{
			set
			{
				API.SetPedNeverLeavesGroup(Handle, value);
			}
		}
		public void LeaveGroup()
		{
			API.RemovePedFromGroup(Handle);
		}

		public void PlayAmbientSpeech(string speechName, SpeechModifier modifier = SpeechModifier.Standard)
		{
			if ((int)modifier >= 0 && (int)modifier < _speechModifierNames.Length)
			{
				API.PlayAmbientSpeech1(Handle, speechName, _speechModifierNames[(int)modifier]);
			}
			else
			{
				new ArgumentOutOfRangeException("modifier");
			}
		}

		public void PlayAmbientSpeech(string voiceName, string speechName, SpeechModifier modifier = SpeechModifier.Standard)
		{
			if ((int)modifier >= 0 && (int)modifier < _speechModifierNames.Length)
			{
				API.PlayAmbientSpeechWithVoice(Handle, speechName, voiceName, _speechModifierNames[(int)modifier], false);
			}
			else
			{
				new ArgumentOutOfRangeException("modifier");
			}
		}

		public void ApplyDamage(int damageAmount)
		{
			API.ApplyDamageToPed(Handle, damageAmount, true);
		}
		public override bool HasBeenDamagedBy(WeaponHash weapon)
		{
			return API.HasPedBeenDamagedByWeapon(Handle, (uint)weapon, 0);
		}
		public override bool HasBeenDamagedByAnyWeapon()
		{
			return API.HasPedBeenDamagedByWeapon(Handle, 0, 2);
		}
		public override bool HasBeenDamagedByAnyMeleeWeapon()
		{
			return API.HasPedBeenDamagedByWeapon(Handle, 0, 1);
		}
		public override void ClearLastWeaponDamage()
		{
			API.ClearPedLastWeaponDamage(Handle);
		}

		public new PedBoneCollection Bones
		{
			get
			{
				if (ReferenceEquals(_pedBones, null))
				{
					_pedBones = new PedBoneCollection(this);
				}
				return _pedBones;
			}
		}

		public Vector3 GetLastWeaponImpactPosition()
		{
			Vector3 position = new Vector3();

			if (API.GetPedLastWeaponImpactCoord(Handle, ref position))
			{
				return position;
			}

			return Vector3.Zero;
		}

		public void Ragdoll(int duration = -1, RagdollType ragdollType = RagdollType.Normal)
		{
			CanRagdoll = true;
			API.SetPedToRagdoll(Handle, duration, duration, (int)ragdollType, false, false, false);
		}

		public void CancelRagdoll()
		{
			API.SetPedToRagdoll(Handle, 1, 1, 1, false, false, false);
		}

		public void GiveHelmet(bool canBeRemovedByPed, HelmetType helmetType, int textureIndex)
		{
			API.GivePedHelmet(Handle, !canBeRemovedByPed, (int)helmetType, textureIndex);
		}
		public void RemoveHelmet(bool instantly)
		{
			API.RemovePedHelmet(Handle, instantly);
		}

		public void OpenParachute()
		{
			API.ForcePedToOpenParachute(Handle);
		}

		public bool GetConfigFlag(int flagID)
		{
			return API.GetPedConfigFlag(Handle, flagID, true);
		}
		public void SetConfigFlag(int flagID, bool value)
		{
			API.SetPedConfigFlag(Handle, flagID, value);
		}
		public void ResetConfigFlag(int flagID)
		{
			API.SetPedResetFlag(Handle, flagID, true);
		}

		public Ped Clone(float heading = 0.0f)
		{
			return new Ped(API.ClonePed(Handle, heading, false, false));
		}
		/// <summary>
		/// Determines whether this <see cref="Ped"/> exists.
		/// </summary>
		/// <returns><c>true</c> if this <see cref="Ped"/> exists; otherwise, <c>false</c></returns>
		public new bool Exists()
		{
			return base.Exists() && API.GetEntityType(Handle) == 1;
		}
		/// <summary>
		/// Determines whether the <see cref="Ped"/> exists.
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to check.</param>
		/// <returns><c>true</c> if the <see cref="Ped"/> exists; otherwise, <c>false</c></returns>
		public static bool Exists(Ped ped)
		{
			return !ReferenceEquals(ped, null) && ped.Exists();
		}

		/// <summary>
		/// Get the first entity in front of the relative Ped 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> the Raycast will start from</param>
		/// <param name="distance">Max distance of the Raycast</param>
		/// <returns>Returns the first Entity encountered in a distance specified</returns>
		public static Entity GetEntityInFrontOfPed(this Ped ped, float distance = 5f)
		{
			RaycastResult raycast = World.Raycast(ped.Position, ped.GetOffsetPosition(new Vector3(0f, distance, 0f)), IntersectOptions.Everything);
			if (raycast.DitHitEntity)
				return (Entity)raycast.HitEntity;
			return null;
		}

	}
}
