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
		Normal = 786603,
		IgnoreLights = 2883621,
		SometimesOvertakeTraffic = 5,
		Rushed = 1074528293,
		AvoidTraffic = 786468,
		AvoidTrafficExtremely = 6
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
		/// Gets or sets how much money this <see cref="Ped"/> is carrying.
		/// </summary>
		public int Money
		{
			get
			{
				return Function.Call<int>(Hash.GET_PED_MONEY, Handle);
			}
			set
			{
				Function.Call(Hash.SET_PED_MONEY, Handle, value);
			}
		}
		/// <summary>
		/// Gets the gender of this <see cref="Ped"/>.
		/// </summary>
		public Gender Gender
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_MALE, Handle) ? Gender.Male : Gender.Female;
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
				return Function.Call<int>(Hash.GET_PED_ARMOUR, Handle);
			}
			set
			{
				Function.Call(Hash.SET_PED_ARMOUR, Handle, value);
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
				return Function.Call<int>(Hash.GET_PED_ACCURACY, Handle);
			}
			set
			{
				Function.Call(Hash.SET_PED_ACCURACY, Handle, value);
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
				return Function.Call<int>(Hash.GET_SEQUENCE_PROGRESS, Handle);
			}
		}

		/// <summary>
		/// Opens a list of <see cref="GTA.NaturalMotion.Euphoria"/> Helpers which can be applied to this <see cref="Ped"/>.
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
            [SecuritySafeCritical]
            get
			{
				int hash;
				unsafe
				{
					if (Function.Call<bool>(Hash.GET_CURRENT_PED_VEHICLE_WEAPON, Handle, &hash))
					{
						return (VehicleWeaponHash)hash;
					}
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
				Vehicle veh = new Vehicle(Function.Call<int>(Hash.GET_VEHICLE_PED_IS_IN, Handle, true));
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
				Vehicle veh = new Vehicle(Function.Call<int>(Hash.GET_VEHICLE_PED_IS_IN, Handle, false));
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
				Vehicle veh = new Vehicle(Function.Call<int>(Hash.GET_VEHICLE_PED_IS_TRYING_TO_ENTER, Handle));
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

				return new PedGroup(Function.Call<int>(Hash.GET_PED_GROUP_INDEX, Handle, false));
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

				Function.Call(Hash.SET_PED_SWEAT, Handle, value);
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
					Function.Call(Hash.CLEAR_PED_WETNESS, Handle);
				}
				else
				{
					Function.Call<float>(Hash.SET_PED_WETNESS_HEIGHT, Handle, value);
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
				Function.Call(Hash.SET_AMBIENT_VOICE_NAME, Handle, value);
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
				Function.Call(Hash.SET_PED_SHOOT_RATE, Handle, value);
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
				return Function.Call<bool>(Hash.WAS_PED_KILLED_BY_STEALTH, Handle);
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
				return Function.Call<bool>(Hash.WAS_PED_KILLED_BY_TAKEDOWN, Handle);
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
				if (MemoryAddress == IntPtr.Zero)
				{
					return VehicleSeat.None;
				}

				int offset = (Game.Version >= GameVersion.v1_0_877_1_Steam ? 0x158A : 0x1542);

				int seatIndex = MemoryAccess.ReadInt(MemoryAddress + offset);

				if (seatIndex == -1 || !IsInVehicle())
				{
					return VehicleSeat.None;
				}

				return (VehicleSeat)(seatIndex - 1);
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
				return Function.Call<bool>(Hash.IS_PED_JUMPING_OUT_OF_VEHICLE, Handle);
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
				Function.Call(Hash.SET_PED_STAY_IN_VEHICLE_WHEN_JACKED, Handle, value);
			}
		}

		/// <summary>
		/// Sets the maximum driving speed this <see cref="Ped"/> can drive at.
		/// </summary>
		public float MaxDrivingSpeed
		{
			set
			{
				Function.Call(Hash.SET_DRIVE_TASK_MAX_CRUISE_SPEED, Handle, value);
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
				return Function.Call<bool>(Hash.IS_PED_HUMAN, Handle);
			}
		}
		public bool IsEnemy
		{
			set
			{
				Function.Call(Hash.SET_PED_AS_ENEMY, Handle, value);
			}
		}
		public bool IsPriorityTargetForEnemies
		{
			set
			{
				Function.Call(Hash.SET_ENTITY_IS_TARGET_PRIORITY, Handle, value, 0);
			}
		}
		public bool IsPlayer
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_A_PLAYER, Handle);
			}
		}

		public bool IsCuffed
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_CUFFED, Handle);
			}
		}
		public bool IsWearingHelmet
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_WEARING_HELMET, Handle);
			}
		}

		public bool IsRagdoll
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_RAGDOLL, Handle);
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
				return Function.Call<bool>(Hash.IS_PED_PRONE, Handle);
			}
		}
		public bool IsDucking
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_DUCKING, Handle);
			}
			set
			{
				Function.Call(Hash.SET_PED_DUCKING, Handle, value);
			}
		}
		public bool IsGettingUp
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_GETTING_UP, Handle);
			}
		}
		public bool IsClimbing
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_CLIMBING, Handle);
			}
		}
		public bool IsJumping
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_JUMPING, Handle);
			}
		}
		public bool IsFalling
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_FALLING, Handle);
			}
		}
		public bool IsStopped
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_STOPPED, Handle);
			}
		}
		public bool IsWalking
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_WALKING, Handle);
			}
		}
		public bool IsRunning
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_RUNNING, Handle);
			}
		}
		public bool IsSprinting
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_SPRINTING, Handle);
			}
		}
		public bool IsDiving
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_DIVING, Handle);
			}
		}
		public bool IsInParachuteFreeFall
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_PARACHUTE_FREE_FALL, Handle);
			}
		}
		public bool IsSwimming
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_SWIMMING, Handle);
			}
		}
		public bool IsSwimmingUnderWater
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_SWIMMING_UNDER_WATER, Handle);
			}
		}
		public bool IsVaulting
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_VAULTING, Handle);
			}
		}

		public bool IsOnBike
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_ON_ANY_BIKE, Handle);
			}
		}
		public bool IsOnFoot
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_ON_FOOT, Handle);
			}
		}
		public bool IsInSub
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_ANY_SUB, Handle);
			}
		}
		public bool IsInTaxi
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_ANY_TAXI, Handle);
			}
		}
		public bool IsInTrain
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_ANY_TRAIN, Handle);
			}
		}
		public bool IsInHeli
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_ANY_HELI, Handle);
			}
		}
		public bool IsInPlane
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_ANY_PLANE, Handle);
			}
		}
		public bool IsInFlyingVehicle
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_FLYING_VEHICLE, Handle);
			}
		}
		public bool IsInBoat
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_ANY_BOAT, Handle);
			}
		}
		public bool IsInPoliceVehicle
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_ANY_POLICE_VEHICLE, Handle);
			}
		}

		public bool IsJacking
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_JACKING, Handle);
			}
		}
		public bool IsBeingJacked
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_BEING_JACKED, Handle);
			}
		}
		public bool IsGettingIntoAVehicle
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_GETTING_INTO_A_VEHICLE, Handle);
			}
		}
		public bool IsTryingToEnterALockedVehicle
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_TRYING_TO_ENTER_A_LOCKED_VEHICLE, Handle);
			}
		}

		public bool IsInjured
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_INJURED, Handle);
			}
		}
		public bool IsFleeing
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_FLEEING, Handle);
			}
		}

		public bool IsInCombat
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_COMBAT, Handle);
			}
		}
		public bool IsInMeleeCombat
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_MELEE_COMBAT, Handle);
			}
		}
		public bool IsInStealthMode
		{
			get
			{
				return Function.Call<bool>(Hash.GET_PED_STEALTH_MOVEMENT, Handle);
			}
		}
		public bool IsAmbientSpeechplaying
		{
			get
			{
				return Function.Call<bool>(Hash.IS_AMBIENT_SPEECH_PLAYING, Handle);
			}
		}
		public bool IsScriptedSpeechplaying
		{
			get
			{
				return Function.Call<bool>(Hash.IS_SCRIPTED_SPEECH_PLAYING, Handle);
			}
		}
		public bool IsAnySpeechplaying
		{
			get
			{
				return Function.Call<bool>(Hash.IS_ANY_SPEECH_PLAYING, Handle);
			}
		}
		public bool IsAmbientSpeechEnabled
		{
			get
			{
				return !Function.Call<bool>(Hash.IS_AMBIENT_SPEECH_DISABLED, Handle);
			}
		}
		public bool IsPainAudioEnabled
		{
			set
			{
				Function.Call(Hash.DISABLE_PED_PAIN_AUDIO, Handle, !value);
			}
		}
		public bool IsPlantingBomb
		{

			get
			{
				return Function.Call<bool>(Hash.IS_PED_PLANTING_BOMB, Handle);
			}
		}
		public bool IsShooting
		{

			get
			{
				return Function.Call<bool>(Hash.IS_PED_SHOOTING, Handle);
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
				return Function.Call<bool>(Hash.IS_PED_RELOADING, Handle);
			}
		}
		public bool IsDoingDriveBy
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_DOING_DRIVEBY, Handle);
			}
		}
		public bool IsGoingIntoCover
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_GOING_INTO_COVER, Handle);
			}
		}
		public bool IsBeingStunned
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_BEING_STUNNED, Handle);
			}
		}
		public bool IsBeingStealthKilled
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_BEING_STEALTH_KILLED, Handle);
			}
		}
		public bool IsPerformingStealthKill
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_PERFORMING_STEALTH_KILL, Handle);
			}
		}

		public bool IsAimingFromCover
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_AIMING_FROM_COVER, Handle);
			}
		}
		public bool IsInCover()
		{
			return IsInCover(false);
		}
		public bool IsInCover(bool expectUseWeapon)
		{
			return Function.Call<bool>(Hash.IS_PED_IN_COVER, Handle, expectUseWeapon);
		}
		public bool IsInCoverFacingLeft
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_COVER_FACING_LEFT, Handle);
			}
		}

		public string MovementAnimationSet
		{
			set
			{
				if (value == null)
				{
					Function.Call(Hash.RESET_PED_MOVEMENT_CLIPSET, 0.25f);
					Task.ClearAll();
				}
				else
				{
					//Movement sets can be applied from anim_dicts and anim_sets(also clip_sets but they use the same native as anim_sets)
					//so check if the string is a valid anim_dict, if so load it as anim dict
					//otherwise load it as an anim_set
					if (Function.Call<bool>(Hash.DOES_ANIM_DICT_EXIST, value))
					{
						Function.Call(Hash.REQUEST_ANIM_DICT, value);
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
						Function.Call(Hash.REQUEST_ANIM_SET, value);
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
					Function.Call(Hash.SET_PED_MOVEMENT_CLIPSET, value, 0.25f);
				}
			}
		}

		public FiringPattern FiringPattern
		{
			set
			{
				Function.Call(Hash.SET_PED_FIRING_PATTERN, Handle, value);
			}
		}
		public ParachuteLandingType ParachuteLandingType
		{
			get
			{
				return Function.Call<ParachuteLandingType>(Hash.GET_PED_PARACHUTE_LANDING_TYPE, Handle);
			}
		}
		public ParachuteState ParachuteState
		{
			get
			{
				return Function.Call<ParachuteState>(Hash.GET_PED_PARACHUTE_STATE, Handle);
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
				Function.Call(Hash.SET_PED_DROPS_WEAPONS_WHEN_DEAD, Handle, value);
			}
		}

		public float DrivingSpeed
		{
			set
			{
				Function.Call(Hash.SET_DRIVE_TASK_CRUISE_SPEED, Handle, value);
			}
		}
		public DrivingStyle DrivingStyle
		{
			set
			{
				Function.Call(Hash.SET_DRIVE_TASK_DRIVING_STYLE, Handle, value);
			}
		}
		public VehicleDrivingFlags VehicleDrivingFlags
		{
			set
			{
				Function.Call(Hash.SET_DRIVE_TASK_DRIVING_STYLE, Handle, value);
			}
		}

		public bool CanRagdoll
		{
			get
			{
				return Function.Call<bool>(Hash.CAN_PED_RAGDOLL, Handle);
			}
			set
			{
				Function.Call(Hash.SET_PED_CAN_RAGDOLL, Handle, value);
			}
		}
		public bool CanPlayGestures
		{
			set
			{
				Function.Call(Hash.SET_PED_CAN_PLAY_GESTURE_ANIMS, Handle, value);
			}
		}
		public bool CanSwitchWeapons
		{
			set
			{
				Function.Call(Hash.SET_PED_CAN_SWITCH_WEAPON, Handle, value);
			}
		}
		public bool CanWearHelmet
		{
			set
			{
				Function.Call(Hash.SET_PED_HELMET, Handle, value);
			}
		}
		public bool CanBeTargetted
		{
			set
			{
				Function.Call(Hash.SET_PED_CAN_BE_TARGETTED, Handle, value);
			}
		}
		public bool CanBeShotInVehicle
		{
			set
			{
				Function.Call(Hash.SET_PED_CAN_BE_SHOT_IN_VEHICLE, Handle, value);
			}
		}
		public bool CanBeDraggedOutOfVehicle
		{
			set
			{
				Function.Call(Hash.SET_PED_CAN_BE_DRAGGED_OUT, Handle, value);
			}
		}
		public bool CanBeKnockedOffBike
		{
			set
			{
				Function.Call(Hash.SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE, Handle, value);
			}
		}
		public bool CanFlyThroughWindscreen
		{
			get
			{
				return Function.Call<bool>(Hash.GET_PED_CONFIG_FLAG, Handle, 32, true);
			}
			set
			{
				Function.Call(Hash.SET_PED_CONFIG_FLAG, Handle, 32, value);
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
				Function.Call(Hash.SET_PED_SUFFERS_CRITICAL_HITS, Handle, value);
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
				Function.Call(Hash.SET_BLOCKING_OF_NON_TEMPORARY_EVENTS, Handle, value);
			}
		}

		public bool AlwaysKeepTask
		{
			set
			{
				Function.Call(Hash.SET_PED_KEEP_TASK, Handle, value);
			}
		}
		public bool AlwaysDiesOnLowHealth
		{
			set
			{
				Function.Call(Hash.SET_PED_DIES_WHEN_INJURED, Handle, value);
			}
		}
		public bool DrownsInWater
		{
			set
			{
				Function.Call(Hash.SET_PED_DIES_IN_WATER, Handle, value);
			}
		}
		public bool DrownsInSinkingVehicle
		{
			set
			{
				Function.Call(Hash.SET_PED_DIES_IN_SINKING_VEHICLE, Handle, value);
			}
		}
		public bool DiesInstantlyInWater
		{
			set
			{
				Function.Call(Hash.SET_PED_DIES_INSTANTLY_IN_WATER, Handle, value);
			}
		}

		public bool IsInVehicle()
		{
			return Function.Call<bool>(Hash.IS_PED_IN_ANY_VEHICLE, Handle, 0);
		}
		public bool IsInVehicle(Vehicle vehicle)
		{
			return Function.Call<bool>(Hash.IS_PED_IN_VEHICLE, Handle, vehicle.Handle, 0);
		}
		public bool IsSittingInVehicle()
		{
			return Function.Call<bool>(Hash.IS_PED_SITTING_IN_ANY_VEHICLE, Handle);
		}
		public bool IsSittingInVehicle(Vehicle vehicle)
		{
			return Function.Call<bool>(Hash.IS_PED_SITTING_IN_VEHICLE, Handle, vehicle.Handle);
		}
		public void SetIntoVehicle(Vehicle vehicle, VehicleSeat seat)
		{
			Function.Call(Hash.SET_PED_INTO_VEHICLE, Handle, vehicle.Handle, seat);
		}

		public Relationship GetRelationshipWithPed(Ped ped)
		{
			return (Relationship)Function.Call<int>(Hash.GET_RELATIONSHIP_BETWEEN_PEDS, Handle, ped.Handle);
		}

		public bool IsHeadtracking(Entity entity)
		{
			return Function.Call<bool>(Hash.IS_PED_HEADTRACKING_ENTITY, Handle, entity.Handle);
		}
		public bool IsInCombatAgainst(Ped target)
		{
			return Function.Call<bool>(Hash.IS_PED_IN_COMBAT, Handle, target.Handle);
		}

		public Ped GetJacker()
		{
			return new Ped(Function.Call<int>(Hash.GET_PEDS_JACKER, Handle));
		}
		public Ped GetJackTarget()
		{
			return new Ped(Function.Call<int>(Hash.GET_JACK_TARGET, Handle));
		}
		public Ped GetMeleeTarget()
		{
			return new Ped(Function.Call<int>(Hash.GET_MELEE_TARGET_FOR_PED, Handle));
		}
		public Entity GetKiller()
		{
			return Entity.FromHandle(Function.Call<int>(Hash.GET_PED_SOURCE_OF_DEATH, Handle));
		}

		public void Kill()
		{
			Health = -1;
		}
		public void Resurrect()
		{
			int maxHealth = MaxHealth;
			bool isCollisionEnabled = IsCollisionEnabled;

			Function.Call(Hash.RESURRECT_PED, Handle);
			MaxHealth = maxHealth;
			Health = maxHealth;
			IsCollisionEnabled = isCollisionEnabled;
		    Function.Call(Hash.CLEAR_PED_TASKS_IMMEDIATELY, Handle);
		}

		public void ResetVisibleDamage()
		{
			Function.Call(Hash.RESET_PED_VISIBLE_DAMAGE, Handle);
		}
		public void ClearBloodDamage()
		{
			Function.Call(Hash.CLEAR_PED_BLOOD_DAMAGE, Handle);
		}

		

		public RelationshipGroup RelationshipGroup
		{
			get
			{
				return new CitizenFX.Core.RelationshipGroup(Function.Call<int>(Hash.GET_PED_RELATIONSHIP_GROUP_HASH, Handle));
			}
			set
			{
				Function.Call(Hash.SET_PED_RELATIONSHIP_GROUP_HASH, Handle, value.Hash);
			}
		}
		public bool IsInGroup
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PED_IN_GROUP, Handle);
			}
		}
		public bool NeverLeavesGroup
		{
			set
			{
				Function.Call(Hash.SET_PED_NEVER_LEAVES_GROUP, Handle, value);
			}
		}
		public void LeaveGroup()
		{
			Function.Call(Hash.REMOVE_PED_FROM_GROUP, Handle);
		}

		public void PlayAmbientSpeech(string speechName, SpeechModifier modifier = SpeechModifier.Standard)
		{
			if ((int) modifier >= 0 && (int) modifier < _speechModifierNames.Length)
			{
				Function.Call(Hash._PLAY_AMBIENT_SPEECH1, Handle, speechName, _speechModifierNames[(int)modifier]);
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
				Function.Call(Hash._PLAY_AMBIENT_SPEECH_WITH_VOICE, Handle, speechName, voiceName, _speechModifierNames[(int)modifier], 0);
			}
			else
			{
				new ArgumentOutOfRangeException("modifier");
			}
		}

		public void ApplyDamage(int damageAmount)
		{
			Function.Call(Hash.APPLY_DAMAGE_TO_PED, Handle, damageAmount, true);
		}
		public override bool HasBeenDamagedBy(WeaponHash weapon)
		{
			return Function.Call<bool>(Hash.HAS_PED_BEEN_DAMAGED_BY_WEAPON, Handle, weapon, 0);
		}
		public override bool HasBeenDamagedByAnyWeapon()
		{
			return Function.Call<bool>(Hash.HAS_PED_BEEN_DAMAGED_BY_WEAPON, Handle, 0, 2);
		}
		public override bool HasBeenDamagedByAnyMeleeWeapon()
		{
			return Function.Call<bool>(Hash.HAS_PED_BEEN_DAMAGED_BY_WEAPON, Handle, 0, 1);
		}
		public override void ClearLastWeaponDamage()
		{
			Function.Call(Hash.CLEAR_PED_LAST_WEAPON_DAMAGE, Handle);
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

        [SecuritySafeCritical]
        public Vector3 GetLastWeaponImpactPosition()
		{
			NativeVector3 position;

			unsafe
			{
				if (Function.Call<bool>(Hash.GET_PED_LAST_WEAPON_IMPACT_COORD, Handle, &position))
				{
					return position;
				}
			}

			return Vector3.Zero;
		}

		public void Ragdoll(int duration = -1, RagdollType ragdollType = RagdollType.Normal)
		{
			CanRagdoll = true;
			Function.Call(Hash.SET_PED_TO_RAGDOLL, Handle, duration, duration, ragdollType, false, false, false);
		}

		public void CancelRagdoll()
		{
			Function.Call(Hash.SET_PED_TO_RAGDOLL, Handle, 1, 1, 1, false, false, false);
		}

		public void GiveHelmet(bool canBeRemovedByPed, HelmetType helmetType, int textureIndex)
		{
			Function.Call(Hash.GIVE_PED_HELMET, Handle, !canBeRemovedByPed, helmetType, textureIndex);
		}
		public void RemoveHelmet(bool instantly)
		{
			Function.Call(Hash.REMOVE_PED_HELMET, Handle, instantly);
		}

		public void OpenParachute()
		{
			Function.Call(Hash.FORCE_PED_TO_OPEN_PARACHUTE, Handle);
		}

		public bool GetConfigFlag(int flagID)
		{
			return Function.Call<bool>(Hash.GET_PED_CONFIG_FLAG, Handle, flagID, true);
		}
		public void SetConfigFlag(int flagID, bool value)
		{
			Function.Call(Hash.SET_PED_CONFIG_FLAG, Handle, flagID, value);
		}
		public void ResetConfigFlag(int flagID)
		{
			Function.Call(Hash.SET_PED_RESET_FLAG, Handle, flagID, true);
		}

		public Ped Clone(float heading = 0.0f)
		{
			return new Ped(Function.Call<int>(Hash.CLONE_PED, Handle, heading, false, false));
		}
		/// <summary>
		/// Determines whether this <see cref="Ped"/> exists.
		/// </summary>
		/// <returns><c>true</c> if this <see cref="Ped"/> exists; otherwise, <c>false</c></returns>
		public new bool Exists()
		{
			return Function.Call<int>(Hash.GET_ENTITY_TYPE, Handle) == 1;
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
	}
}
