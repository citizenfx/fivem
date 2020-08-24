using CitizenFX.Core.Native;
using System;
using System.Drawing;
using System.Security;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
	public enum ParachuteTint
	{
		None = -1,
		Rainbow,
		Red,
		SeasideStripes,
		WidowMaker,
		Patriot,
		Blue,
		Black,
		Hornet,
		AirFocce,
		Desert,
		Shadow,
		HighAltitude,
		Airbone,
		Sunrise
	}

	public sealed class Player : INativeValue, IEquatable<Player>
	{
		#region Fields
		private int _handle;
		Ped _ped;
		#endregion
		public Player(int handle)
		{
			_handle = handle;
		}

		public int Handle { get { return _handle; } }
		public override ulong NativeValue
		{
			get { return (ulong)_handle; }
			set { _handle = unchecked((int)value); }
		}

		/// <summary>
		/// Gets the <see cref="Ped"/> this <see cref="Player"/> is controling.
		/// </summary>
		public Ped Character
		{
			get
			{
				int handle = API.GetPlayerPed(Handle);

				if (ReferenceEquals(_ped, null) || handle != _ped.Handle)
				{
					_ped = new Ped(handle);
				}

				return _ped;
			}
		}

		/// <summary>
		/// Gets the <see cref="StateBag"/> of this <see cref="Player"/>
		/// </summary>
		public StateBag State
		{
			get
			{
				return new StateBag("player:" + ServerId);
			}
		}

		/// <summary>
		/// Gets the name of this <see cref="Player"/>.
		/// </summary>
		public string Name
		{
			get
			{
				return API.GetPlayerName(Handle);
			}
		}
		/// <summary>
		/// Gets or sets how much money this <see cref="Player"/> has.
		/// <remarks>Only works if current player is <see cref="PedHash.Michael"/>, <see cref="PedHash.Franklin"/> or <see cref="PedHash.Trevor"/></remarks>
		/// </summary>
		public int Money
		{
			get
			{
				int stat;

				switch ((PedHash)Character.Model.Hash)
				{
					case PedHash.Michael:
						stat = Game.GenerateHash("SP0_TOTAL_CASH");
						break;
					case PedHash.Franklin:
						stat = Game.GenerateHash("SP1_TOTAL_CASH");
						break;
					case PedHash.Trevor:
						stat = Game.GenerateHash("SP2_TOTAL_CASH");
						break;
					default:
						return 0;
				}

				int result = 0;
				API.StatGetInt((uint)stat, ref result, -1);

				return result;
			}
			set
			{
				uint stat;

				switch ((PedHash)Character.Model.Hash)
				{
					case PedHash.Michael:
						stat = (uint)Game.GenerateHash("SP0_TOTAL_CASH");
						break;
					case PedHash.Franklin:
						stat = (uint)Game.GenerateHash("SP1_TOTAL_CASH");
						break;
					case PedHash.Trevor:
						stat = (uint)Game.GenerateHash("SP2_TOTAL_CASH");
						break;
					default:
						return;
				}

				API.StatSetInt(stat, value, true);
			}
		}

		/// <summary>
		/// Gets or sets the wanted level for this <see cref="Player"/>.
		/// </summary>
		public int WantedLevel
		{
			get
			{
				return API.GetPlayerWantedLevel(Handle);
			}
			set
			{
				API.SetPlayerWantedLevel(Handle, value, false);
				API.SetPlayerWantedLevelNow(Handle, false);
			}
		}
		/// <summary>
		/// Gets or sets the wanted center position for this <see cref="Player"/>.
		/// </summary>
		/// <value>
		/// The place in world coords where the police think this <see cref="Player"/> is.
		/// </value>
		public Vector3 WantedCenterPosition
		{
			get
			{
				return API.GetPlayerWantedCentrePosition(Handle);
			}
			set
			{
				API.SetPlayerWantedCentrePosition(Handle, value.X, value.Y, value.Z);
			}
		}

		/// <summary>
		/// Gets or sets the maximum amount of armor this <see cref="Player"/> can carry.
		/// </summary>
		public int MaxArmor
		{
			get
			{
				return API.GetPlayerMaxArmour(Handle);
			}
			set
			{
				API.SetPlayerMaxArmour(Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets the primary parachute tint for this <see cref="Player"/>.
		/// </summary>
		public ParachuteTint PrimaryParachuteTint
		{
			get
			{
				int result = 0;

				API.GetPlayerParachuteTintIndex(Handle, ref result);

				return (ParachuteTint)result;
			}
			set
			{
				API.SetPlayerParachuteTintIndex(Handle, (int)value);
			}
		}
		/// <summary>
		/// Gets or sets the reserve parachute tint for this <see cref="Player"/>.
		/// </summary>
		public ParachuteTint ReserveParachuteTint
		{
			get
			{
				int result = 0;

				API.GetPlayerReserveParachuteTintIndex(Handle, ref result);

				return (ParachuteTint)result;
			}
			set
			{
				API.SetPlayerReserveParachuteTintIndex(Handle, (int)value);
			}
		}

		/// <summary>
		/// Sets a value indicating whether this <see cref="Player"/> can leave a parachute smoke trail.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Player"/> can leave a parachute smoke trail; otherwise, <c>false</c>.
		/// </value>
		public bool CanLeaveParachuteSmokeTrail
		{
			set
			{
				API.SetPlayerCanLeaveParachuteSmokeTrail(Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets the color of the parachute smoke trail for this <see cref="Player"/>.
		/// </summary>
		/// <value>
		/// The color of the parachute smoke trail for this <see cref="Player"/>.
		/// </value>
		public Color ParachuteSmokeTrailColor
		{
			get
			{
				int r = 0, g = 0, b = 0;

				API.GetPlayerParachuteSmokeTrailColor(Handle, ref r, ref g, ref b);

				return Color.FromArgb(r, g, b);
			}
			set
			{
				API.SetPlayerParachuteSmokeTrailColor(Handle, value.R, value.G, value.B);
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Player"/> is alive.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Player"/> is alive; otherwise, <c>false</c>.
		/// </value>
		public bool IsAlive
		{
			get
			{
				return !IsDead;
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Player"/> is dead.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Player"/> is dead; otherwise, <c>false</c>.
		/// </value>
		public bool IsDead
		{
			get
			{
				return API.IsPlayerDead(Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Player"/> is aiming.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Player"/> is aiming; otherwise, <c>false</c>.
		/// </value>
		public bool IsAiming
		{
			get
			{
				return API.IsPlayerFreeAiming(Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Player"/> is climbing.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Player"/> is climbing; otherwise, <c>false</c>.
		/// </value>
		public bool IsClimbing
		{
			get
			{
				return API.IsPlayerClimbing(Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Player"/> is riding a train.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Player"/> is riding a train; otherwise, <c>false</c>.
		/// </value>
		public bool IsRidingTrain
		{
			get
			{
				return API.IsPlayerRidingTrain(Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Player"/> is pressing a horn.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Player"/> is pressing a horn; otherwise, <c>false</c>.
		/// </value>
		public bool IsPressingHorn
		{
			get
			{
				return API.IsPlayerPressingHorn(Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Player"/> is playing.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Player"/> is playing; otherwise, <c>false</c>.
		/// </value>
		public bool IsPlaying
		{
			get
			{
				return API.IsPlayerPlaying(Handle);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Player"/> is invincible.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Player"/> is invincible; otherwise, <c>false</c>.
		/// </value>
		public bool IsInvincible
		{
			get
			{
				return API.GetPlayerInvincible(Handle);
			}
			set
			{
				API.SetPlayerInvincible(Handle, value);
			}
		}

		/// <summary>
		/// Sets a value indicating whether this <see cref="Player"/> is ignored by the police.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Player"/> is ignored by the police; otherwise, <c>false</c>.
		/// </value>
		public bool IgnoredByPolice
		{
			set
			{
				API.SetPoliceIgnorePlayer(Handle, value);
			}
		}
		/// <summary>
		/// Sets a value indicating whether this <see cref="Player"/> is ignored by everyone.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Player"/> is ignored by everyone; otherwise, <c>false</c>.
		/// </value>
		public bool IgnoredByEveryone
		{
			set
			{
				API.SetEveryoneIgnorePlayer(Handle, value);
			}
		}

		/// <summary>
		/// Sets a value indicating whether cops will be dispatched for this <see cref="Player"/>
		/// </summary>
		/// <value>
		///   <c>true</c> if cops will be dispatched; otherwise, <c>false</c>.
		/// </value>
		public bool DispatchsCops
		{
			set
			{
				API.SetDispatchCopsForPlayer(Handle, value);
			}
		}

		/// <summary>
		/// Sets a value indicating whether this <see cref="Player"/> can use cover.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Player"/> can use cover; otherwise, <c>false</c>.
		/// </value>
		public bool CanUseCover
		{
			set
			{
				API.SetPlayerCanUseCover(Handle, value);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Player"/> can start a mission.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Player"/> can start a mission; otherwise, <c>false</c>.
		/// </value>
		public bool CanStartMission
		{
			get
			{
				return API.CanPlayerStartMission(Handle);
			}
		}

		/// <summary>
		/// Sets a value indicating whether this <see cref="Player"/> can control ragdoll.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Player"/> can control ragdoll; otherwise, <c>false</c>.
		/// </value>
		public bool CanControlRagdoll
		{
			set
			{
				API.GivePlayerRagdollControl(Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Player"/> can control its <see cref="Ped"/>.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Player"/> can control its <see cref="Ped"/>; otherwise, <c>false</c>.
		/// </value>
		public bool CanControlCharacter
		{
			get
			{
				return API.IsPlayerControlOn(Handle);
			}
			set
			{
				API.SetPlayerControl(Handle, value, 0);
			}
		}

		/// <summary>
		/// Attempts to change the <see cref="Model"/> of this <see cref="Player"/>.
		/// </summary>
		/// <param name="model">The <see cref="Model"/> to change this <see cref="Player"/> to.</param>
		/// <returns><c>true</c> if the change was sucessful; otherwise, <c>false</c>.</returns>
		public async Task<bool> ChangeModel(Model model)
		{
			if (!model.IsInCdImage || !model.IsPed || !await model.Request(1000))
			{
				return false;
			}

			API.SetPlayerModel(Handle, (uint)model.Hash);

			model.MarkAsNoLongerNeeded();

			return true;
		}

		/// <summary>
		/// Gets how long this <see cref="Player"/> can remain sprinting for.
		/// </summary>
		public float RemainingSprintTime
		{
			get
			{
				return API.GetPlayerSprintTimeRemaining(Handle);
			}
		}

		/// <summary>
		/// Gets how much sprint stamina this <see cref="Player"/> currently has. 
		/// </summary>
		public float RemainingSprintStamina
		{
			get
			{
				return API.GetPlayerSprintStaminaRemaining(Handle);
			}
		}
		/// <summary>
		/// Gets how long this <see cref="Player"/> can stay underwater before they start losing health.
		/// </summary>
		public float RemainingUnderwaterTime
		{
			get
			{
				return API.GetPlayerUnderwaterTimeRemaining(Handle);
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Player"/> is using their special ability.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Player"/> is using their special ability; otherwise, <c>false</c>.
		/// </value>
		public bool IsSpecialAbilityActive
		{
			get
			{
				return API.IsSpecialAbilityActive(Handle);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Player"/> can use their special ability.
		/// </summary>
		/// <value>
		/// <c>true</c> if this  <see cref="Player"/> can use their special ability; otherwise, <c>false</c>.
		/// </value>
		public bool IsSpecialAbilityEnabled
		{
			get
			{
				return API.IsSpecialAbilityEnabled(Handle);
			}
			set
			{
				API.EnableSpecialAbility(Handle, value);
			}
		}
		/// <summary>
		/// Charges the special ability for this <see cref="Player"/>.
		/// </summary>
		/// <param name="absoluteAmount">The absolute amount.</param>
		public void ChargeSpecialAbility(int absoluteAmount)
		{
			API.SpecialAbilityChargeAbsolute(Handle, absoluteAmount, true);
		}
		/// <summary>
		/// Charges the special ability for this <see cref="Player"/>.
		/// </summary>
		/// <param name="normalizedRatio">The amount between <c>0.0f</c> and <c>1.0f</c></param>
		public void ChargeSpecialAbility(float normalizedRatio)
		{
			API.SpecialAbilityChargeNormalized(Handle, normalizedRatio, true);
		}
		/// <summary>
		/// Refills the special ability for this <see cref="Player"/>.
		/// </summary>
		public void RefillSpecialAbility()
		{
			API.SpecialAbilityFillMeter(Handle, true);
		}
		/// <summary>
		/// Depletes the special ability for this <see cref="Player"/>.
		/// </summary>
		public void DepleteSpecialAbility()
		{
			API.SpecialAbilityDepleteMeter(Handle, true);
		}

		/// <summary>
		/// Gets the last <see cref="Vehicle"/> this <see cref="Player"/> used.
		/// </summary>
		/// <remarks>returns <c>null</c> if the last vehicle doesn't exist.</remarks>
		public Vehicle LastVehicle
		{
			get
			{
				Vehicle veh = new Vehicle(API.GetPlayersLastVehicle());
				return veh.Exists() ? veh : null;
			}
		}

		/// <summary>
		/// Determines whether this <see cref="Player"/> is targetting the specified <see cref="Entity"/>.
		/// </summary>
		/// <param name="entity">The <see cref="Entity"/> to check.</param>
		/// <returns>
		///   <c>true</c> if this <see cref="Player"/> is targetting the specified <see cref="Entity"/>; otherwise, <c>false</c>.
		/// </returns>
		public bool IsTargetting(Entity entity)
		{
			return API.IsPlayerFreeAimingAtEntity(Handle, entity.Handle);
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Player"/> is targetting anything.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Player"/> is targetting anything; otherwise, <c>false</c>.
		/// </value>
		public bool IsTargettingAnything
		{
			get
			{
				return API.IsPlayerTargettingAnything(Handle);
			}
		}
		/// <summary>
		/// Gets the <see cref="Entity"/> this <see cref="Player"/> is targetting.
		/// </summary>
		/// <returns>The <see cref="Entity"/> if this <see cref="Player"/> is targetting any <see cref="Entity"/>; otherwise, <c>null</c></returns>
		public Entity GetTargetedEntity()
		{
			int entityHandle = 0;

			if (API.GetEntityPlayerIsFreeAimingAt(Handle, ref entityHandle))
			{
				if (API.DoesEntityExist(entityHandle))
					return Entity.FromHandle(entityHandle);
			}
			return null;
		}
		/// <summary>
		/// Sets a value indicating whether ths player is forced to aim.
		/// </summary>
		/// <value>
		///   <c>true</c> to make the player always be aiming; otherwise, <c>false</c>.
		/// </value>
		public bool ForcedAim
		{
			set { API.SetPlayerForcedAim(Handle, value); }
		}

		/// <summary>
		/// Prevents this <see cref="Player"/> firing this frame.
		/// </summary>
		public void DisableFiringThisFrame()
		{
			API.DisablePlayerFiring(Handle, false);
		}
		/// <summary>
		/// Sets the run speed mult for this this <see cref="Player"/> this frame. (THIS NAME IS WRONG, SHOULD NOT BE CALLED EVERY FRAME).
		/// </summary>
		/// <param name="mult">The factor - min: <c>0.0f</c>, default: <c>1.0f</c>, max: <c>1.499f</c>.</param>
		public void SetRunSpeedMultThisFrame(float mult)
		{
			if (mult > 1.499f)
			{
				mult = 1.499f;
			}

			API.SetRunSprintMultiplierForPlayer(Handle, mult);
		}
		/// <summary>
		/// Sets the swim speed mult for this this <see cref="Player"/> this frame. (THIS NAME IS WRONG, SHOULD NOT BE CALLED EVERY FRAME).
		/// </summary>
		/// <param name="mult">The factor - min: <c>0.0f</c>, default: <c>1.0f</c>, max: <c>1.499f</c>.</param>
		public void SetSwimSpeedMultThisFrame(float mult)
		{
			if (mult > 1.499f)
			{
				mult = 1.499f;
			}

			API.SetSwimMultiplierForPlayer(Handle, mult);
		}
		/// <summary>
		/// Makes this <see cref="Player"/> shoot fire bullets this frame.
		/// </summary>
		public void SetFireAmmoThisFrame()
		{
			API.SetFireAmmoThisFrame(Handle);
		}
		/// <summary>
		/// Makes this <see cref="Player"/> shoot explosive bullets this frame.
		/// </summary>
		public void SetExplosiveAmmoThisFrame()
		{
			API.SetExplosiveAmmoThisFrame(Handle);
		}
		/// <summary>
		/// Makes this <see cref="Player"/> have an explosive melee attack this frame.
		/// </summary>
		public void SetExplosiveMeleeThisFrame()
		{
			API.SetExplosiveMeleeThisFrame(Handle);
		}
		/// <summary>
		/// Lets this <see cref="Player"/> jump really high this frame.
		/// </summary>
		public void SetSuperJumpThisFrame()
		{
			API.SetSuperJumpThisFrame(Handle);
		}
		/// <summary>
		/// Blocks this <see cref="Player"/> from entering any <see cref="Vehicle"/> this frame.
		/// </summary>
		public void SetMayNotEnterAnyVehicleThisFrame()
		{
			API.SetPlayerMayNotEnterAnyVehicle(Handle);
		}
		/// <summary>
		/// Only lets this <see cref="Player"/> enter a specific <see cref="Vehicle"/> this frame.
		/// </summary>
		/// <param name="vehicle">The <see cref="Vehicle"/> this <see cref="Player"/> is allowed to enter.</param>
		public void SetMayOnlyEnterThisVehicleThisFrame(Vehicle vehicle)
		{
			API.SetPlayerMayOnlyEnterThisVehicle(Handle, vehicle.Handle);
		}

		public bool Equals(Player player)
		{
			return !ReferenceEquals(player, null) && Handle == player.Handle;
		}
		public override bool Equals(object obj)
		{
			return !ReferenceEquals(obj, null) && obj.GetType() == GetType() && Equals((Entity)obj);
		}

		public override int GetHashCode()
		{
			return Handle.GetHashCode();
		}

		public static bool operator ==(Player left, Player right)
		{
			return ReferenceEquals(left, null) ? ReferenceEquals(right, null) : left.Equals(right);
		}
		public static bool operator !=(Player left, Player right)
		{
			return !(left == right);
		}

		// CFX-EXTENSION
		public int ServerId
		{
			get
			{
				return API.GetPlayerServerId(Handle);
			}
		}
	}
}
