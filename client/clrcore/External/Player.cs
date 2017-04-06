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
				int handle = Function.Call<int>(Hash.GET_PLAYER_PED, Handle);

				if (ReferenceEquals(_ped, null) || handle != _ped.Handle)
				{
					_ped = new Ped(handle);
				}

				return _ped;
			}
		}

		/// <summary>
		/// Gets the name of this <see cref="Player"/>.
		/// </summary>
		public string Name
		{
			get
			{
				return Function.Call<string>(Hash.GET_PLAYER_NAME, Handle);
			}
		}
		/// <summary>
		/// Gets or sets how much money this <see cref="Player"/> has.
		/// <remarks>Only works if current player is <see cref="PedHash.Michael"/>, <see cref="PedHash.Franklin"/> or <see cref="PedHash.Trevor"/></remarks>
		/// </summary>
		public int Money
		{
            [SecuritySafeCritical]
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

				int result;
				unsafe
				{
					Function.Call(Hash.STAT_GET_INT, stat, &result, -1);
				}

				return result;
			}
			set
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
						return;
				}

				Function.Call(Hash.STAT_SET_INT, stat, value, 1);
			}
		}

		/// <summary>
		/// Gets or sets the wanted level for this <see cref="Player"/>.
		/// </summary>
		public int WantedLevel
		{
			get
			{
				return Function.Call<int>(Hash.GET_PLAYER_WANTED_LEVEL, Handle);
			}
			set
			{
				Function.Call(Hash.SET_PLAYER_WANTED_LEVEL, Handle, value, false);
				Function.Call(Hash.SET_PLAYER_WANTED_LEVEL_NOW, Handle, false);
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
				return Function.Call<Vector3>(Hash.GET_PLAYER_WANTED_CENTRE_POSITION, Handle);
			}
			set
			{
				Function.Call(Hash.SET_PLAYER_WANTED_CENTRE_POSITION, Handle, value.X, value.Y, value.Z);
			}
		}

		/// <summary>
		/// Gets or sets the maximum amount of armor this <see cref="Player"/> can carry.
		/// </summary>
		public int MaxArmor
		{
			get
			{
				return Function.Call<int>(Hash.GET_PLAYER_MAX_ARMOUR, Handle);
			}
			set
			{
				Function.Call(Hash.SET_PLAYER_MAX_ARMOUR, Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets the primary parachute tint for this <see cref="Player"/>.
		/// </summary>
		public ParachuteTint PrimaryParachuteTint
		{
            [SecuritySafeCritical]
            get
			{
				int result;

				unsafe
				{
					Function.Call(Hash.GET_PLAYER_PARACHUTE_TINT_INDEX, Handle, &result);
				}

				return (ParachuteTint)result;
			}
			set
			{
				Function.Call(Hash.SET_PLAYER_PARACHUTE_TINT_INDEX, Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets the reserve parachute tint for this <see cref="Player"/>.
		/// </summary>
		public ParachuteTint ReserveParachuteTint
		{
            [SecuritySafeCritical]
            get
			{
				int result;

				unsafe
				{
					Function.Call(Hash.GET_PLAYER_RESERVE_PARACHUTE_TINT_INDEX, Handle, &result);
				}

				return (ParachuteTint)result;
			}
			set
			{
				Function.Call(Hash.SET_PLAYER_RESERVE_PARACHUTE_TINT_INDEX, Handle, value);
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
				Function.Call(Hash.SET_PLAYER_CAN_LEAVE_PARACHUTE_SMOKE_TRAIL, Handle, value);
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
            [SecuritySafeCritical]
            get
			{
				int r, g, b;
				unsafe
				{
					Function.Call(Hash.GET_PLAYER_PARACHUTE_SMOKE_TRAIL_COLOR, Handle, &r, &g, &b);
				}

				return Color.FromArgb(r, g, b);
			}
			set
			{
				Function.Call(Hash.SET_PLAYER_PARACHUTE_SMOKE_TRAIL_COLOR, Handle, value.R, value.G, value.B);
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
				return Function.Call<bool>(Hash.IS_PLAYER_DEAD, Handle);
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
				return Function.Call<bool>(Hash.IS_PLAYER_FREE_AIMING, Handle);
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
				return Function.Call<bool>(Hash.IS_PLAYER_CLIMBING, Handle);
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
				return Function.Call<bool>(Hash.IS_PLAYER_RIDING_TRAIN, Handle);
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
				return Function.Call<bool>(Hash.IS_PLAYER_PRESSING_HORN, Handle);
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
				return Function.Call<bool>(Hash.IS_PLAYER_PLAYING, Handle);
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
				return Function.Call<bool>(Hash.GET_PLAYER_INVINCIBLE, Handle);
			}
			set
			{
				Function.Call(Hash.SET_PLAYER_INVINCIBLE, Handle, value);
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
				Function.Call(Hash.SET_POLICE_IGNORE_PLAYER, Handle, value);
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
				Function.Call(Hash.SET_EVERYONE_IGNORE_PLAYER, Handle, value);
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
				Function.Call(Hash.SET_DISPATCH_COPS_FOR_PLAYER, Handle, value);
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
				Function.Call(Hash.SET_PLAYER_CAN_USE_COVER, Handle, value);
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
				return Function.Call<bool>(Hash.CAN_PLAYER_START_MISSION, Handle);
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
				Function.Call(Hash.GIVE_PLAYER_RAGDOLL_CONTROL, Handle, value);
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
				return Function.Call<bool>(Hash.IS_PLAYER_CONTROL_ON, Handle);
			}
			set
			{
				Function.Call(Hash.SET_PLAYER_CONTROL, Handle, value, 0);
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

			Function.Call(Hash.SET_PLAYER_MODEL, Handle, model.Hash);

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
				return Function.Call<float>(Hash.GET_PLAYER_SPRINT_TIME_REMAINING, Handle);
			}
		}

		/// <summary>
		/// Gets how much sprint stamina this <see cref="Player"/> currently has. 
		/// </summary>
		public float RemainingSprintStamina
		{
			get
			{
				return Function.Call<float>(Hash.GET_PLAYER_SPRINT_STAMINA_REMAINING, Handle);
			}
		}
		/// <summary>
		/// Gets how long this <see cref="Player"/> can stay underwater before they start losing health.
		/// </summary>
		public float RemainingUnderwaterTime
		{
			get
			{
				return Function.Call<float>(Hash.GET_PLAYER_UNDERWATER_TIME_REMAINING, Handle);
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
				return Function.Call<bool>(Hash.IS_SPECIAL_ABILITY_ACTIVE, Handle);
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
				return Function.Call<bool>(Hash.IS_SPECIAL_ABILITY_ENABLED, Handle);
			}
			set
			{
				Function.Call(Hash.ENABLE_SPECIAL_ABILITY, Handle, value);
			}
		}
		/// <summary>
		/// Charges the special ability for this <see cref="Player"/>.
		/// </summary>
		/// <param name="absoluteAmount">The absolute amount.</param>
		public void ChargeSpecialAbility(int absoluteAmount)
		{
			Function.Call(Hash.SPECIAL_ABILITY_CHARGE_ABSOLUTE, Handle, absoluteAmount, true);
		}
		/// <summary>
		/// Charges the special ability for this <see cref="Player"/>.
		/// </summary>
		/// <param name="normalizedRatio">The amount between <c>0.0f</c> and <c>1.0f</c></param>
		public void ChargeSpecialAbility(float normalizedRatio)
		{
			Function.Call(Hash.SPECIAL_ABILITY_CHARGE_NORMALIZED, Handle, normalizedRatio, true);
		}
		/// <summary>
		/// Refills the special ability for this <see cref="Player"/>.
		/// </summary>
		public void RefillSpecialAbility()
		{
			Function.Call(Hash.SPECIAL_ABILITY_FILL_METER, Handle, 1);
		}
		/// <summary>
		/// Depletes the special ability for this <see cref="Player"/>.
		/// </summary>
		public void DepleteSpecialAbility()
		{
			Function.Call(Hash.SPECIAL_ABILITY_DEPLETE_METER, Handle, 1);
		}

		/// <summary>
		/// Gets the last <see cref="Vehicle"/> this <see cref="Player"/> used.
		/// </summary>
		/// <remarks>returns <c>null</c> if the last vehicle doesn't exist.</remarks>
		public Vehicle LastVehicle
		{
			get
			{
				Vehicle veh = new Vehicle(Function.Call<int>(Hash.GET_PLAYERS_LAST_VEHICLE));
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
			return Function.Call<bool>(Hash.IS_PLAYER_FREE_AIMING_AT_ENTITY, Handle, entity.Handle);
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
				return Function.Call<bool>(Hash.IS_PLAYER_TARGETTING_ANYTHING, Handle);
			}
		}
        /// <summary>
        /// Gets the <see cref="Entity"/> this <see cref="Player"/> is targetting.
        /// </summary>
        /// <returns>The <see cref="Entity"/> if this <see cref="Player"/> is targetting any <see cref="Entity"/>; otherwise, <c>null</c></returns>
        [SecuritySafeCritical]
        public Entity GetTargetedEntity()
		{
			int entityHandle;

			unsafe
			{
				if (Function.Call<bool>(Hash.GET_ENTITY_PLAYER_IS_FREE_AIMING_AT, Handle, &entityHandle))
				{
					return Entity.FromHandle(entityHandle);
				}
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
			set { Function.Call(Hash.SET_PLAYER_FORCED_AIM, Handle, value); }
		}

		/// <summary>
		/// Prevents this <see cref="Player"/> firing this frame.
		/// </summary>
		public void DisableFiringThisFrame()
		{
			Function.Call(Hash.DISABLE_PLAYER_FIRING, Handle, 0);
		}
		/// <summary>
		/// Sets the run speed mult for this this <see cref="Player"/> this frame.
		/// </summary>
		/// <param name="mult">The factor - min: <c>0.0f</c>, default: <c>1.0f</c>, max: <c>1.499f</c>.</param>
		public void SetRunSpeedMultThisFrame(float mult)
		{
			if (mult > 1.499f)
			{
				mult = 1.499f;
			}

			Function.Call(Hash.SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER, Handle, mult);
		}
		/// <summary>
		/// Sets the swim speed mult for this this <see cref="Player"/> this frame.
		/// </summary>
		/// <param name="mult">The factor - min: <c>0.0f</c>, default: <c>1.0f</c>, max: <c>1.499f</c>.</param>
		public void SetSwimSpeedMultThisFrame(float mult)
		{
			if (mult > 1.499f)
			{
				mult = 1.499f;
			}

			Function.Call(Hash.SET_SWIM_MULTIPLIER_FOR_PLAYER, Handle, mult);
		}
		/// <summary>
		/// Makes this <see cref="Player"/> shoot fire bullets this frame.
		/// </summary>
		public void SetFireAmmoThisFrame()
		{
			Function.Call(Hash.SET_FIRE_AMMO_THIS_FRAME, Handle);
		}
		/// <summary>
		/// Makes this <see cref="Player"/> shoot explosive bullets this frame.
		/// </summary>
		public void SetExplosiveAmmoThisFrame()
		{
			Function.Call(Hash.SET_EXPLOSIVE_AMMO_THIS_FRAME, Handle);
		}
		/// <summary>
		/// Makes this <see cref="Player"/> have an explosive melee attack this frame.
		/// </summary>
		public void SetExplosiveMeleeThisFrame()
		{
			Function.Call(Hash.SET_EXPLOSIVE_MELEE_THIS_FRAME, Handle);
		}
		/// <summary>
		/// Lets this <see cref="Player"/> jump really high this frame.
		/// </summary>
		public void SetSuperJumpThisFrame()
		{
			Function.Call(Hash.SET_SUPER_JUMP_THIS_FRAME, Handle);
		}
		/// <summary>
		/// Blocks this <see cref="Player"/> from entering any <see cref="Vehicle"/> this frame.
		/// </summary>
		public void SetMayNotEnterAnyVehicleThisFrame()
		{
			Function.Call(Hash.SET_PLAYER_MAY_NOT_ENTER_ANY_VEHICLE, Handle);
		}
		/// <summary>
		/// Only lets this <see cref="Player"/> enter a specific <see cref="Vehicle"/> this frame.
		/// </summary>
		/// <param name="vehicle">The <see cref="Vehicle"/> this <see cref="Player"/> is allowed to enter.</param>
		public void SetMayOnlyEnterThisVehicleThisFrame(Vehicle vehicle)
		{
			Function.Call(Hash.SET_PLAYER_MAY_ONLY_ENTER_THIS_VEHICLE, Handle, vehicle.Handle);
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
                return Function.Call<int>(Hash.GET_PLAYER_SERVER_ID, this.Handle);
            }
        }
	}
}
