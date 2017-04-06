using System;
using System.Linq;
using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
	public enum ForceType
	{
		MinForce,
		MaxForceRot,
		MinForce2,
		MaxForceRot2,
		ForceNoRot,
		ForceRotPlusForce
	}

	public abstract class Entity : PoolObject, IEquatable<Entity>, ISpatial
	{
		#region Fields
		private EntityBoneCollection _bones;
		#endregion

		public Entity(int handle) : base(handle)
		{
		}

		/// <summary>
		/// Gets the memory address where the <see cref="Entity"/> is stored in memory.
		/// </summary>
		public IntPtr MemoryAddress
		{
			get
			{
                // CFX-TODO
                return Function.Call<IntPtr>((Hash)MemoryAccess.GetHashKey("get_entity_address"), Handle);
				//return MemoryAccess.GetEntityAddress(Handle);
			}
		}

		/// <summary>
		/// Gets or sets the health of this <see cref="Entity"/> as an <see cref="int"/>.
		/// </summary>
		/// <value>
		/// The health from 0 - 100 as an integer.
		/// </value>
		/// <remarks>if you need to get or set the value strictly, use <see cref="HealthFloat"/> instead.</remarks>
		public int Health
		{
			get
			{
				return Function.Call<int>(Hash.GET_ENTITY_HEALTH, Handle) - 100;
			}
			set
			{
				Function.Call(Hash.SET_ENTITY_HEALTH, Handle, value + 100);
			}
		}
		/// <summary>
		/// Gets or sets the health of this <see cref="Entity"/> as a <see cref="float"/>.
		/// </summary>
		/// <value>
		/// The health in float.
		/// </value>
		public float HealthFloat
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}

				return MemoryAccess.ReadFloat(MemoryAddress + 640);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				MemoryAccess.WriteFloat(MemoryAddress + 640, value);
			}
		}
		/// <summary>
		/// Gets or sets the maximum health of this <see cref="Entity"/> as an <see cref="int"/>.
		/// </summary>
		/// <value>
		/// The maximum health from 0 - 100 as an integer.
		/// </value>
		/// <remarks>if you need to get or set the value strictly, use <see cref="MaxHealthFloat"/> instead.</remarks>
		public int MaxHealth
		{
			get
			{
				return Function.Call<int>(Hash.GET_ENTITY_MAX_HEALTH, Handle) - 100;
			}
			set
			{
				Function.Call(Hash.SET_ENTITY_MAX_HEALTH, Handle, value + 100);
			}
		}
		/// <summary>
		/// Gets or sets the maximum health of this <see cref="Entity"/> in float.
		/// </summary>
		/// <value>
		/// The maximum health in float.
		/// </value>
		public float MaxHealthFloat
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}

				return MemoryAccess.ReadFloat(MemoryAddress + 644);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				MemoryAccess.WriteFloat(MemoryAddress + 644, value);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Entity"/> is dead.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Entity"/> is dead; otherwise, <c>false</c>.
		/// </value>
		public bool IsDead
		{
			get
			{
				return Function.Call<bool>(Hash.IS_ENTITY_DEAD, Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Entity"/> is alive.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Entity"/> is alive; otherwise, <c>false</c>.
		/// </value>
		public bool IsAlive
		{
			get
			{
				return !IsDead;
			}
		}

		public Model Model
		{
			get
			{
				return new Model(Function.Call<int>(Hash.GET_ENTITY_MODEL, Handle));
			}
		}

		/// <summary>
		/// Gets or sets the position of this <see cref="Entity"/>.
		/// </summary>
		/// <value>
		/// The position in world space.
		/// </value>
		public virtual Vector3 Position
		{
			get
			{
				return Function.Call<Vector3>(Hash.GET_ENTITY_COORDS, Handle, 0);
			}
			set
			{
				Function.Call(Hash.SET_ENTITY_COORDS, Handle, value.X, value.Y, value.Z, 0, 0, 0, 1);
			}
		}
		/// <summary>
		/// Sets the position of this <see cref="Entity"/> without any offset.
		/// </summary>
		/// <value>
		/// The position in world space.
		/// </value>
		public Vector3 PositionNoOffset
		{
			set
			{
				Function.Call(Hash.SET_ENTITY_COORDS_NO_OFFSET, Handle, value.X, value.Y, value.Z, 1, 1, 1);
			}
		}
		/// <summary>
		/// Gets or sets the rotation of this <see cref="Entity"/>.
		/// </summary>
		/// <value>
		/// The yaw, pitch, roll rotation values.
		/// </value>
		public virtual Vector3 Rotation
		{
			get
			{
				return Function.Call<Vector3>(Hash.GET_ENTITY_ROTATION, Handle, 2);
			}
			set
			{
				Function.Call(Hash.SET_ENTITY_ROTATION, Handle, value.X, value.Y, value.Z, 2, 1);
			}
		}
		/// <summary>
		/// Gets or sets the quaternion of this <see cref="Entity"/>.
		/// </summary>
		public Quaternion Quaternion
		{
            [SecuritySafeCritical]
			get
			{
				float x;
				float y;
				float z;
				float w;
				unsafe
				{
					Function.Call(Hash.GET_ENTITY_QUATERNION, Handle, &x, &y, &z, &w);
				}

				return new Quaternion(x, y, z, w);
			}
			set
			{
				Function.Call(Hash.SET_ENTITY_QUATERNION, Handle, value.X, value.Y, value.Z, value.W);
			}
		}
		/// <summary>
		/// Gets or sets the heading of this <see cref="Entity"/>.
		/// </summary>
		/// <value>
		/// The heading in degrees.
		/// </value>
		public float Heading
		{
			get
			{
				return Function.Call<float>(Hash.GET_ENTITY_HEADING, Handle);
			}
			set
			{
				Function.Call<float>(Hash.SET_ENTITY_HEADING, Handle, value);
			}
		}
		/// <summary>
		/// Gets the vector that points above this <see cref="Entity"/>
		/// </summary>

		public Vector3 UpVector
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
                    return Vector3.Zero;//.RelativeTop;
				}
				return MemoryAccess.ReadVector3(MemoryAddress + 0x80);
			}
		}
		/// <summary>
		/// Gets the vector that points to the right of this <see cref="Entity"/>
		/// </summary>
		public Vector3 RightVector
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return Vector3.Right;
				}
				return MemoryAccess.ReadVector3(MemoryAddress + 0x60);
			}
		}
		/// <summary>
		/// Gets the vector that points in front of this <see cref="Entity"/>
		/// </summary>
		public Vector3 ForwardVector
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return Vector3.ForwardLH;
				}
				return MemoryAccess.ReadVector3(MemoryAddress + 0x70);
			}
		}

		/// <summary>
		/// Gets this <see cref="Entity"/>s matrix which stores position and rotation information.
		/// </summary>
		public Matrix Matrix
		{
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return new Matrix();
				}
				return MemoryAccess.ReadMatrix(memoryAddress + 96);
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> is frozen.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is position frozen; otherwise, <c>false</c>.
		/// </value>
		public bool IsPositionFrozen
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				return MemoryAccess.IsBitSet(MemoryAddress + 0x2E, 1);
			}
			set
			{
				Function.Call(Hash.FREEZE_ENTITY_POSITION, Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets the velocity of this <see cref="Entity"/>.
		/// </summary>
		public Vector3 Velocity
		{
			get
			{
				return Function.Call<Vector3>(Hash.GET_ENTITY_VELOCITY, Handle);
			}
			set
			{
				Function.Call(Hash.SET_ENTITY_VELOCITY, Handle, value.X, value.Y, value.Z);
			}
		}
		/// <summary>
		/// Gets the rotation velocity of this <see cref="Entity"/>.
		/// </summary>
		public Vector3 RotationVelocity
		{
			get
			{
				return Function.Call<Vector3>(Hash.GET_ENTITY_ROTATION_VELOCITY, Handle);
			}
		}
		/// <summary>
		/// Sets the maximum speed this <see cref="Entity"/> can move at.
		/// </summary>
		public float MaxSpeed
		{
			set
			{
				Function.Call(Hash.SET_ENTITY_MAX_SPEED, Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> has gravity.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> has gravity; otherwise, <c>false</c>.
		/// </value>
		public bool HasGravity
		{
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return true;
				}
				memoryAddress = MemoryAccess.ReadPtr(memoryAddress + 48);
				if (memoryAddress == IntPtr.Zero)
				{
					return true;
				}
				return !MemoryAccess.IsBitSet(memoryAddress + 26, 4);

			}
			set
			{
				Function.Call(Hash.SET_ENTITY_HAS_GRAVITY, Handle, value);
			}
		}
		/// <summary>
		/// Gets how high above ground this <see cref="Entity"/> is.
		/// </summary>
		public float HeightAboveGround
		{
			get
			{
				return Function.Call<float>(Hash.GET_ENTITY_HEIGHT_ABOVE_GROUND, Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating how submersed this <see cref="Entity"/> is, 1.0f means the whole entity is submerged.
		/// </summary>
		public float SubmersionLevel
		{
			get
			{
				return Function.Call<float>(Hash.GET_ENTITY_SUBMERGED_LEVEL, Handle);
			}
		}

		/// <summary>
		/// Gets or sets the level of detail distance of this <see cref="Entity"/>.
		/// </summary>
		public int LodDistance
		{
			get
			{
				return Function.Call<int>(Hash.GET_ENTITY_LOD_DIST, Handle);
			}
			set
			{
				Function.Call(Hash.SET_ENTITY_LOD_DIST, Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> is visible.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is visible; otherwise, <c>false</c>.
		/// </value>
		public bool IsVisible
		{
			get
			{
				return Function.Call<bool>(Hash.IS_ENTITY_VISIBLE, Handle);
			}
			set
			{
				Function.Call(Hash.SET_ENTITY_VISIBLE, Handle, value);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Entity"/> is occluded.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is occluded; otherwise, <c>false</c>.
		/// </value>
		public bool IsOccluded
		{
			get
			{
				return Function.Call<bool>(Hash.IS_ENTITY_OCCLUDED, Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Entity"/> is on screen.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is on screen; otherwise, <c>false</c>.
		/// </value>
		public bool IsOnScreen
		{
			get
			{
				return Function.Call<bool>(Hash.IS_ENTITY_ON_SCREEN, Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Entity"/> is rendered.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is rendered; otherwise, <c>false</c>.
		/// </value>
		public bool IsRendered
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				return MemoryAccess.IsBitSet(MemoryAddress + 176, 4);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Entity"/> is upright.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is upright; otherwise, <c>false</c>.
		/// </value>
		public bool IsUpright
		{
			get
			{
				return Function.Call<bool>(Hash.IS_ENTITY_UPRIGHT, Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Entity"/> is upside down.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is upside down; otherwise, <c>false</c>.
		/// </value>
		public bool IsUpsideDown
		{
			get
			{
				return Function.Call<bool>(Hash.IS_ENTITY_UPSIDEDOWN, Handle);
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Entity"/> is in the air.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Entity"/> is in the air; otherwise, <c>false</c>.
		/// </value>
		public bool IsInAir
		{

			get
			{
				return Function.Call<bool>(Hash.IS_ENTITY_IN_AIR, Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Entity"/> is in water.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is in water; otherwise, <c>false</c>.
		/// </value>
		public bool IsInWater
		{

			get
			{
				return Function.Call<bool>(Hash.IS_ENTITY_IN_WATER, Handle);
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> is persistent.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is persistent; otherwise, <c>false</c>.
		/// </value>
		public bool IsPersistent
		{
			get
			{
				return Function.Call<bool>(Hash.IS_ENTITY_A_MISSION_ENTITY, Handle);
			}
			set
			{
				if (value)
				{
					Function.Call(Hash.SET_ENTITY_AS_MISSION_ENTITY, Handle, true, false);
				}
				else
				{
					MarkAsNoLongerNeeded();
				}
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Entity"/> is on fire.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is on fire; otherwise, <c>false</c>.
		/// </value>
		public bool IsOnFire
		{
			get
			{
				return Function.Call<bool>(Hash.IS_ENTITY_ON_FIRE, Handle);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> is fire proof.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is fire proof; otherwise, <c>false</c>.
		/// </value>
		public bool IsFireProof
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				return MemoryAccess.IsBitSet(MemoryAddress + 392, 5);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				IntPtr address = MemoryAddress + 392;

				if (value)
				{
					MemoryAccess.SetBit(address, 5);
				}
				else
				{
					MemoryAccess.ClearBit(address, 5);
				}
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> is melee proof.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is melee proof; otherwise, <c>false</c>.
		/// </value>
		public bool IsMeleeProof
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				return MemoryAccess.IsBitSet(MemoryAddress + 392, 7);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				IntPtr address = MemoryAddress + 392;

				if (value)
				{
					MemoryAccess.SetBit(address, 7);
				}
				else
				{
					MemoryAccess.ClearBit(address, 7);
				}
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> is bullet proof.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is bullet proof; otherwise, <c>false</c>.
		/// </value>
		public bool IsBulletProof
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				return MemoryAccess.IsBitSet(MemoryAddress + 392, 4);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				IntPtr address = MemoryAddress + 392;

				if (value)
				{
					MemoryAccess.SetBit(address, 4);
				}
				else
				{
					MemoryAccess.ClearBit(address, 4);
				}
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> is explosion proof.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is explosion proof; otherwise, <c>false</c>.
		/// </value>
		public bool IsExplosionProof
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				return MemoryAccess.IsBitSet(MemoryAddress + 392, 11);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				IntPtr address = MemoryAddress + 392;

				if (value)
				{
					MemoryAccess.SetBit(address, 11);
				}
				else
				{
					MemoryAccess.ClearBit(address, 11);
				}
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> is collision proof.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is collision proof; otherwise, <c>false</c>.
		/// </value>
		public bool IsCollisionProof
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				return MemoryAccess.IsBitSet(MemoryAddress + 392, 6);
			}
			set
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return;
				}

				IntPtr address = MemoryAddress + 392;

				if (value)
				{
					MemoryAccess.SetBit(address, 6);
				}
				else
				{
					MemoryAccess.ClearBit(address, 6);
				}
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> is invincible.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is invincible; otherwise, <c>false</c>.
		/// </value>
		public bool IsInvincible
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				return MemoryAccess.IsBitSet(MemoryAddress + 392, 8);
			}
			set
			{
				Function.Call(Hash.SET_ENTITY_INVINCIBLE, Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> can only be damaged by <see cref="Player"/>s.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> can only be damaged by <see cref="Player"/>s; otherwise, <c>false</c>.
		/// </value>
		public bool IsOnlyDamagedByPlayer
		{
			get
			{
				if (MemoryAddress == IntPtr.Zero)
				{
					return false;
				}

				return MemoryAccess.IsBitSet(MemoryAddress + 392, 9);
			}
			set
			{
				Function.Call(Hash.SET_ENTITY_ONLY_DAMAGED_BY_PLAYER, Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets how opacque this <see cref="Entity"/> is.
		/// </summary>
		/// <value>
		/// 0 for completely see through, 255 for fully opacque
		/// </value>
		public int Opacity
		{
			get
			{
				return Function.Call<int>(Hash.GET_ENTITY_ALPHA, Handle);
			}
			set
			{
				Function.Call(Hash.SET_ENTITY_ALPHA, Handle, value, false);
			}
		}
		/// <summary>
		/// Resets the opacity, <seealso cref="Opacity"/>.
		/// </summary>
		public void ResetOpacity()
		{
			Function.Call(Hash.RESET_ENTITY_ALPHA, Handle);
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Entity"/> has collided with anything.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> has collided; otherwise, <c>false</c>.
		/// </value>
		/// <remarks><see cref="IsRecordingCollisions"/> must be <c>true</c> for this to work.</remarks>
		public bool HasCollided
		{
			get
			{
				return Function.Call<bool>(Hash.HAS_ENTITY_COLLIDED_WITH_ANYTHING, Handle);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> has collision.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> has collision; otherwise, <c>false</c>.
		/// </value>
		public bool IsCollisionEnabled
		{
			get
			{
				return !Function.Call<bool>(Hash._IS_ENTITY_COLLISON_DISABLED, Handle);
			}
			set
			{
				Function.Call(Hash.SET_ENTITY_COLLISION, Handle, value, false);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> is recording collisions.
		/// </summary>
		public bool IsRecordingCollisions
		{
			set
			{
				Function.Call(Hash.SET_ENTITY_RECORDS_COLLISIONS, Handle, value);
			}
		}
		/// <summary>
		/// Sets the collision between this <see cref="Entity"/> and another <see cref="Entity"/>
		/// </summary>
		/// <param name="entity">The <see cref="Entity"/> to set collision with</param>
		/// <param name="toggle">if set to <c>true</c> the 2 <see cref="Entity"/>s wont collide with each other.</param>
		public void SetNoCollision(Entity entity, bool toggle)
		{
			Function.Call(Hash.SET_ENTITY_NO_COLLISION_ENTITY, Handle, entity.Handle, toggle);
		}

		/// <summary>
		/// Determines whether this <see cref="Entity"/> has been damaged by a specified <see cref="Entity"/>.
		/// </summary>
		/// <param name="entity">The <see cref="Entity"/> to check</param>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> has been damaged by the specified <see cref="Entity"/>; otherwise, <c>false</c>.
		/// </returns>
		public bool HasBeenDamagedBy(Entity entity)
		{
			return Function.Call<bool>(Hash.HAS_ENTITY_BEEN_DAMAGED_BY_ENTITY, Handle, entity.Handle, 1);
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> has been damaged by a specific weapon].
		/// </summary>
		/// <param name="weapon">The weapon to check.</param>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> has been damaged by the specified weapon; otherwise, <c>false</c>.
		/// </returns>
		public virtual bool HasBeenDamagedBy(WeaponHash weapon)
		{
			return Function.Call<bool>(Hash.HAS_ENTITY_BEEN_DAMAGED_BY_WEAPON, Handle, weapon, 0);
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> has been damaged by any weapon.
		/// </summary>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> has been damaged by any weapon; otherwise, <c>false</c>.
		/// </returns>
		public virtual bool HasBeenDamagedByAnyWeapon()
		{
			return Function.Call<bool>(Hash.HAS_ENTITY_BEEN_DAMAGED_BY_WEAPON, Handle, 0, 2);
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> has been damaged by any melee weapon.
		/// </summary>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> has been damaged by any melee weapon; otherwise, <c>false</c>.
		/// </returns>
		public virtual bool HasBeenDamagedByAnyMeleeWeapon()
		{
			return Function.Call<bool>(Hash.HAS_ENTITY_BEEN_DAMAGED_BY_WEAPON, Handle, 0, 1);
		}
		/// <summary>
		/// Clears the last weapon damage this <see cref="Entity"/> received.
		/// </summary>
		public virtual void ClearLastWeaponDamage()
		{
			Function.Call(Hash.CLEAR_ENTITY_LAST_WEAPON_DAMAGE, Handle);
		}

		/// <summary>
		/// Determines whether this <see cref="Entity"/> is in a specified area
		/// </summary>
		/// <param name="minBounds">The minimum bounds.</param>
		/// <param name="maxBounds">The maximum bounds.</param>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> is in the specified area; otherwise, <c>false</c>.
		/// </returns>
		public bool IsInArea(Vector3 minBounds, Vector3 maxBounds)
		{
			return Function.Call<bool>(Hash.IS_ENTITY_IN_AREA, Handle, minBounds.X, minBounds.Y, minBounds.Z, maxBounds.X, maxBounds.Y, maxBounds.Z);
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> is in a specified angled area
		/// </summary>
		/// <param name="origin">The origin.</param>
		/// <param name="edge">The edge.</param>
		/// <param name="angle">The angle.</param>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> is in the specified angled area; otherwise, <c>false</c>.
		/// </returns>
		public bool IsInAngledArea(Vector3 origin, Vector3 edge, float angle)
		{
			return Function.Call<bool>(Hash.IS_ENTITY_IN_ANGLED_AREA, Handle, origin.X, origin.Y, origin.Z, edge.X, edge.Y, edge.Z, angle, false, true, false);
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> is in range of a specified position
		/// </summary>
		/// <param name="position">The position.</param>
		/// <param name="range">The maximum range.</param>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> is in range of the <paramref name="position"/>; otherwise, <c>false</c>.
		/// </returns>
		public bool IsInRangeOf(Vector3 position, float range)
		{
			return Vector3.Subtract(Position, position).LengthSquared() < range * range;
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> is near a specified <see cref="Entity"/>.
		/// </summary>
		/// <param name="entity">The <see cref="Entity"/> to check.</param>
		/// <param name="bounds">The max displacement from the <paramref name="entity"/>.</param>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> is near the <paramref name="entity"/>; otherwise, <c>false</c>.
		/// </returns>
		public bool IsNearEntity(Entity entity, Vector3 bounds)
		{
			return Function.Call<bool>(Hash.IS_ENTITY_AT_ENTITY, Handle, entity.Handle, bounds.X, bounds.Y, bounds.Z, false, true, false);
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> is touching an <see cref="Entity"/> with the <see cref="Model"/> <paramref name="model"/>.
		/// </summary>
		/// <param name="model">The <see cref="Model"/> to check</param>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> is touching a <paramref name="model"/>; otherwise, <c>false</c>.
		/// </returns>
		public bool IsTouching(Model model)
		{
			return Function.Call<bool>(Hash.IS_ENTITY_TOUCHING_MODEL, Handle, model.Hash);
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> is touching the <see cref="Entity"/> <paramref name="entity"/>.
		/// </summary>
		/// <param name="entity">The <see cref="Entity"/> to check.</param>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> is touching <paramref name="entity"/>; otherwise, <c>false</c>.
		/// </returns>
		public bool IsTouching(Entity entity)
		{
			return Function.Call<bool>(Hash.IS_ENTITY_TOUCHING_ENTITY, Handle, entity.Handle);
		}

		/// <summary>
		/// Gets the position in world coords of an offset relative this <see cref="Entity"/>
		/// </summary>
		/// <param name="offset">The offset from this <see cref="Entity"/>.</param>
		public Vector3 GetOffsetPosition(Vector3 offset)
		{
			return Function.Call<Vector3>(Hash.GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS, Handle, offset.X, offset.Y, offset.Z);
		}
		/// <summary>
		/// Gets the relative offset of this <see cref="Entity"/> from a world coords position
		/// </summary>
		/// <param name="worldCoords">The world coords.</param>
		public Vector3 GetPositionOffset(Vector3 worldCoords)
		{
			return Function.Call<Vector3>(Hash.GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS, Handle, worldCoords.X, worldCoords.Y, worldCoords.Z);
		}

		/// <summary>
		/// Gets a collection of the <see cref="EntityBone"/>s in this <see cref="Entity"/>
		/// </summary>
		public virtual EntityBoneCollection Bones
		{
			get
			{
				if (ReferenceEquals(_bones, null))
				{
					_bones = new EntityBoneCollection(this);
				}
				return _bones;
			}
		}

		/// <summary>
		/// Creates a <see cref="Blip"/> on this <see cref="Entity"/>
		/// </summary>
		public Blip AttachBlip()
		{
			return new Blip(Function.Call<int>(Hash.ADD_BLIP_FOR_ENTITY, Handle));
		}
		/// <summary>
		/// Gets the <see cref="Blip"/> attached to this <see cref="Entity"/>
		/// </summary>
		/// <remarks>returns <c>null</c> if no <see cref="Blip"/>s are attached to this <see cref="Entity"/></remarks>
		public Blip AttachedBlip
		{
			get
			{
				int handle = Function.Call<int>(Hash.GET_BLIP_FROM_ENTITY, Handle);

				if (Function.Call<bool>(Hash.DOES_BLIP_EXIST, handle))
				{
					return new Blip(handle);
				}

				return null;
			}
		}
		/// <summary>
		/// Gets an <c>array</c> of all <see cref="Blip"/>s attached to this <see cref="Entity"/>.
		/// </summary>
		public Blip[] AttachedBlips
		{
			get
			{
				return World.GetAllBlips().Where(x => Function.Call<int>(Hash.GET_BLIP_INFO_ID_ENTITY_INDEX, x) == Handle).ToArray();
			}
		}

		/// <summary>
		/// Attaches this <see cref="Entity"/> to a different <see cref="Entity"/>
		/// </summary>
		/// <param name="entity">The <see cref="Entity"/> to attach this <see cref="Entity"/> to.</param>
		/// <param name="position">The position relative to the <paramref name="entity"/> to attach this <see cref="Entity"/> to.</param>
		/// <param name="rotation">The rotation to apply to this <see cref="Entity"/> relative to the <paramref name="entity"/></param>
		public void AttachTo(Entity entity, Vector3 position = default(Vector3), Vector3 rotation = default(Vector3))
		{
			Function.Call(Hash.ATTACH_ENTITY_TO_ENTITY, Handle, entity.Handle, -1, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, 0, 0, 0, 0, 2, 1);
		}
		/// <summary>
		/// Attaches this <see cref="Entity"/> to a different <see cref="Entity"/>
		/// </summary>
		/// <param name="entityBone">The <see cref="EntityBone"/> to attach this <see cref="Entity"/> to.</param>
		/// <param name="position">The position relative to the <paramref name="entityBone"/> to attach this <see cref="Entity"/> to.</param>
		/// <param name="rotation">The rotation to apply to this <see cref="Entity"/> relative to the <paramref name="entityBone"/></param>
		public void AttachTo(EntityBone entityBone, Vector3 position = default(Vector3), Vector3 rotation = default(Vector3))
		{
			Function.Call(Hash.ATTACH_ENTITY_TO_ENTITY, Handle, entityBone.Owner.Handle, entityBone, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, 0, 0, 0, 0, 2, 1);
		}
		/// <summary>
		/// Detaches this <see cref="Entity"/> from any <see cref="Entity"/> it may be attached to.
		/// </summary>
		public void Detach()
		{
			Function.Call(Hash.DETACH_ENTITY, Handle, true, true);
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> is attached to any other <see cref="Entity"/>.
		/// </summary>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> is attached to another <see cref="Entity"/>; otherwise, <c>false</c>.
		/// </returns>
		public bool IsAttached()
		{
			return Function.Call<bool>(Hash.IS_ENTITY_ATTACHED, Handle);
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> is attached to the specified <see cref="Entity"/>.
		/// </summary>
		/// <param name="entity">The <see cref="Entity"/> to check if this <see cref="Entity"/> is attached to.</param>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> is attached to <paramref name="entity"/>; otherwise, <c>false</c>.
		/// </returns>
		public bool IsAttachedTo(Entity entity)
		{
			return Function.Call<bool>(Hash.IS_ENTITY_ATTACHED_TO_ENTITY, Handle, entity.Handle);
		}
		/// <summary>
		/// Gets the <see cref="Entity"/> this <see cref="Entity"/> is attached to.
		/// <remarks>returns <c>null</c> if this <see cref="Entity"/> isnt attached to any entity</remarks>
		/// </summary>
		public Entity GetEntityAttachedTo()
		{
			return FromHandle(Function.Call<int>(Hash.GET_ENTITY_ATTACHED_TO, Handle));
		}

		/// <summary>
		/// Applies a force to this <see cref="Entity"/>.
		/// </summary>
		/// <param name="direction">The direction to apply the force relative to world coords.</param>
		/// <param name="rotation">The rotation force to apply</param>
		/// <param name="forceType">Type of the force to apply.</param>
		public void ApplyForce(Vector3 direction, Vector3 rotation = default(Vector3), ForceType forceType = ForceType.MaxForceRot2)
		{
			Function.Call(Hash.APPLY_FORCE_TO_ENTITY, Handle, forceType, direction.X, direction.Y, direction.Z, rotation.X, rotation.Y, rotation.Z, false, false, true, true, false, true);
		}
		/// <summary>
		/// Applies a force to this <see cref="Entity"/>.
		/// </summary>
		/// <param name="direction">The direction to apply the force relative to this <see cref="Entity"/>s rotation</param>
		/// <param name="rotation">The rotation force to apply</param>
		/// <param name="forceType">Type of the force to apply.</param>
		public void ApplyForceRelative(Vector3 direction, Vector3 rotation = default(Vector3), ForceType forceType = ForceType.MaxForceRot2)
		{
			Function.Call(Hash.APPLY_FORCE_TO_ENTITY, Handle, forceType, direction.X, direction.Y, direction.Z, rotation.X, rotation.Y, rotation.Z, false, true, true, true, false, true);
		}

		/// <summary>
		/// Stops all particle effects attached to this <see cref="Entity"/>
		/// </summary>
		public void RemoveAllParticleEffects()
		{
			Function.Call(Hash.REMOVE_PARTICLE_FX_FROM_ENTITY, Handle);
		}

        /// <summary>
        /// Deletes this <see cref="Entity"/>
        /// </summary>
        [SecuritySafeCritical]
        public override void Delete()
        {
            _Delete();
        }

        [SecuritySafeCritical]
        private void _Delete()
		{
			Function.Call(Hash.SET_ENTITY_AS_MISSION_ENTITY, Handle, false, true);
			int handle = Handle;
			unsafe
			{
				Function.Call(Hash.DELETE_ENTITY, &handle);
			}
			Handle = handle;
		}
        /// <summary>
        /// Marks this <see cref="Entity"/> as no longer needed letting the game delete it when its too far away.
        /// </summary>
        [SecuritySafeCritical]
        public void MarkAsNoLongerNeeded()
        {
            _MarkAsNoLongerNeeded();
        }

        [SecuritySafeCritical]
        private void _MarkAsNoLongerNeeded()
		{
			Function.Call(Hash.SET_ENTITY_AS_MISSION_ENTITY, Handle, false, true);
			int handle = Handle;
			unsafe
			{
				Function.Call(Hash.SET_ENTITY_AS_NO_LONGER_NEEDED, &handle);
			}
			Handle = handle;
		}

		/// <summary>
		/// Creates a new instance of an <see cref="Entity"/> from the given handle.
		/// </summary>
		/// <param name="handle">The entity handle.</param>
		/// <returns>Returns a <see cref="Ped"/> if this handle corresponds to a Ped.
		/// Returns a <see cref="Vehicle"/> if this handle corresponds to a Vehicle.
		/// Returns a <see cref="Prop"/> if this handle corresponds to a Prop.
		/// Returns <c>null</c> if no <see cref="Entity"/> exists this the specified <paramref name="handle"/></returns>
		public static Entity FromHandle(int handle)
		{
			switch (Function.Call<int>(Hash.GET_ENTITY_TYPE, handle))
			{
				case 1:
					return new Ped(handle);
				case 2:
					return new Vehicle(handle);	
				case 3:
					return new Prop(handle); 
			}
			return null;
		}

		/// <summary>
		/// Determines whether this <see cref="Entity"/> exists.
		/// </summary>
		/// <returns><c>true</c> if this <see cref="Entity"/> exists; otherwise, <c>false</c></returns>
		public override bool Exists()
		{
			return Function.Call<bool>(Hash.DOES_ENTITY_EXIST, Handle);
		}
		/// <summary>
		/// Determines whether the <see cref="Entity"/> exists.
		/// </summary>
		/// <param name="entity">The <see cref="Entity"/> to check.</param>
		/// <returns><c>true</c> if the <see cref="Entity"/> exists; otherwise, <c>false</c></returns>
		public static bool Exists(Entity entity)
		{
			return !ReferenceEquals(entity, null) && entity.Exists();
		}
		/// <summary>
		/// Checks if two <see cref="Entity"/>s refer to the same <see cref="Entity"/>
		/// </summary>
		/// <param name="entity">The other <see cref="Entity"/>.</param>
		/// <returns><c>true</c> if they are the same <see cref="Entity"/>; otherwise, false</returns>
		public bool Equals(Entity entity)
		{
			return !ReferenceEquals(entity, null) && Handle == entity.Handle;
		}
		public override bool Equals(object obj)
		{
			return !ReferenceEquals(obj, null) && obj.GetType() == GetType() && Equals((Entity)obj);
		}

		public override int GetHashCode()
		{
			return Handle.GetHashCode();
		}

		public static bool operator ==(Entity left, Entity right)
		{
			return ReferenceEquals(left, null) ? ReferenceEquals(right, null) : left.Equals(right);
		}
		public static bool operator !=(Entity left, Entity right)
		{
			return !(left == right);
		}
	}
}
