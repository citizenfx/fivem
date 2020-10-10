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
				return API.GetEntityHealth(Handle) - 100;
			}
			set
			{
				API.SetEntityHealth(Handle, value + 100);
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
				return API.GetEntityMaxHealth(Handle) - 100;
			}
			set
			{
				API.SetEntityMaxHealth(Handle, value + 100);
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
				return API.IsEntityDead(Handle);
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
				return new Model(API.GetEntityModel(Handle));
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
				return API.GetEntityCoords(Handle, false);
			}
			set
			{
				API.SetEntityCoords(Handle, value.X, value.Y, value.Z, false, false, false, true);
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
				API.SetEntityCoordsNoOffset(Handle, value.X, value.Y, value.Z, true, true, true);
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
				return API.GetEntityRotation(Handle, 2);
			}
			set
			{
				API.SetEntityRotation(Handle, value.X, value.Y, value.Z, 2, true);
			}
		}
		/// <summary>
		/// Gets or sets the quaternion of this <see cref="Entity"/>.
		/// </summary>
		public Quaternion Quaternion
		{
			get
			{
				float x = 0f;
				float y = 0f;
				float z = 0f;
				float w = 0f;

				API.GetEntityQuaternion(Handle, ref x, ref y, ref z, ref w);

				return new Quaternion(x, y, z, w);
			}
			set
			{
				API.SetEntityQuaternion(Handle, value.X, value.Y, value.Z, value.W);
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
				return API.GetEntityHeading(Handle);
			}
			set
			{
				API.SetEntityHeading(Handle, value);
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
				API.FreezeEntityPosition(Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets the velocity of this <see cref="Entity"/>.
		/// </summary>
		public Vector3 Velocity
		{
			get
			{
				return API.GetEntityVelocity(Handle);
			}
			set
			{
				API.SetEntityVelocity(Handle, value.X, value.Y, value.Z);
			}
		}
		/// <summary>
		/// Gets the rotation velocity of this <see cref="Entity"/>.
		/// </summary>
		public Vector3 RotationVelocity
		{
			get
			{
				return API.GetEntityRotationVelocity(Handle);
			}
		}
		/// <summary>
		/// Sets the maximum speed this <see cref="Entity"/> can move at.
		/// </summary>
		public float MaxSpeed
		{
			set
			{
				API.SetEntityMaxSpeed(Handle, value);
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
				API.SetEntityHasGravity(Handle, value);
			}
		}
		/// <summary>
		/// Gets how high above ground this <see cref="Entity"/> is.
		/// </summary>
		public float HeightAboveGround
		{
			get
			{
				return API.GetEntityHeightAboveGround(Handle);
			}
		}
		/// <summary>
		/// Gets a value indicating how submersed this <see cref="Entity"/> is, 1.0f means the whole entity is submerged.
		/// </summary>
		public float SubmersionLevel
		{
			get
			{
				return API.GetEntitySubmergedLevel(Handle);
			}
		}

		/// <summary>
		/// Gets or sets the level of detail distance of this <see cref="Entity"/>.
		/// </summary>
		public int LodDistance
		{
			get
			{
				return API.GetEntityLodDist(Handle);
			}
			set
			{
				API.SetEntityLodDist(Handle, value);
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
				return API.IsEntityVisible(Handle);
			}
			set
			{
				API.SetEntityVisible(Handle, value, false);
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
				return API.IsEntityOccluded(Handle);
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
				return API.IsEntityOnScreen(Handle);
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
				return API.IsEntityUpright(Handle, 0f);
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
				return API.IsEntityUpsidedown(Handle);
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
				return API.IsEntityInAir(Handle);
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
				return API.IsEntityInWater(Handle);
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
				return API.IsEntityAMissionEntity(Handle);
			}
			set
			{
				if (value)
				{
					API.SetEntityAsMissionEntity(Handle, true, false);
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
				return API.IsEntityOnFire(Handle);
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
				API.SetEntityInvincible(Handle, value);
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
				API.SetEntityOnlyDamagedByPlayer(Handle, value);
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
				return API.GetEntityAlpha(Handle);
			}
			set
			{
				API.SetEntityAlpha(Handle, value, 0); // p2 used to be false
			}
		}
		/// <summary>
		/// Resets the opacity, <seealso cref="Opacity"/>.
		/// </summary>
		public void ResetOpacity()
		{
			API.ResetEntityAlpha(Handle);
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
				return API.HasEntityCollidedWithAnything(Handle);
			}
		}

		/// <summary>
		/// Gets the material this entity is currently brushing up against. Only works
		/// for the material the entity is facing towards.
		/// </summary>
		public MaterialHash MaterialCollidingWith
		{
			get
			{
				return (MaterialHash) API.GetLastMaterialHitByEntity(Handle);
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
				return !API.GetEntityCollisonDisabled(Handle);
			}
			set
			{
				API.SetEntityCollision(Handle, value, false);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> is recording collisions.
		/// </summary>
		public bool IsRecordingCollisions
		{
			set
			{
				API.SetEntityRecordsCollisions(Handle, value);
			}
		}
		/// <summary>
		/// Sets the collision between this <see cref="Entity"/> and another <see cref="Entity"/>
		/// </summary>
		/// <param name="entity">The <see cref="Entity"/> to set collision with</param>
		/// <param name="toggle">if set to <c>true</c> the 2 <see cref="Entity"/>s wont collide with each other.</param>
		public void SetNoCollision(Entity entity, bool toggle)
		{
			API.SetEntityNoCollisionEntity(Handle, entity.Handle, toggle);
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
			return API.HasEntityBeenDamagedByEntity(Handle, entity.Handle, true);
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
			return API.HasEntityBeenDamagedByWeapon(Handle, (uint)weapon, 0);
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> has been damaged by any weapon.
		/// </summary>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> has been damaged by any weapon; otherwise, <c>false</c>.
		/// </returns>
		public virtual bool HasBeenDamagedByAnyWeapon()
		{
			return API.HasEntityBeenDamagedByWeapon(Handle, 0, 2);
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> has been damaged by any melee weapon.
		/// </summary>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> has been damaged by any melee weapon; otherwise, <c>false</c>.
		/// </returns>
		public virtual bool HasBeenDamagedByAnyMeleeWeapon()
		{
			return API.HasEntityBeenDamagedByWeapon(Handle, 0, 1);
		}
		/// <summary>
		/// Clears the last weapon damage this <see cref="Entity"/> received.
		/// </summary>
		public virtual void ClearLastWeaponDamage()
		{
			API.ClearEntityLastWeaponDamage(Handle);
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
			return API.IsEntityInArea(Handle, minBounds.X, minBounds.Y, minBounds.Z, maxBounds.X, maxBounds.Y, maxBounds.Z, false, false, 0);
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
			return API.IsEntityInAngledArea(Handle, origin.X, origin.Y, origin.Z, edge.X, edge.Y, edge.Z, angle, false, true, 0);
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
			return API.IsEntityAtEntity(Handle, entity.Handle, bounds.X, bounds.Y, bounds.Z, false, true, 0);
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
			return API.IsEntityTouchingModel(Handle, (uint)model.Hash);
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
			return API.IsEntityTouchingEntity(Handle, entity.Handle);
		}

		/// <summary>
		/// Gets the position in world coords of an offset relative this <see cref="Entity"/>
		/// </summary>
		/// <param name="offset">The offset from this <see cref="Entity"/>.</param>
		public Vector3 GetOffsetPosition(Vector3 offset)
		{
			return API.GetOffsetFromEntityInWorldCoords(Handle, offset.X, offset.Y, offset.Z);
		}
		/// <summary>
		/// Gets the relative offset of this <see cref="Entity"/> from a world coords position
		/// </summary>
		/// <param name="worldCoords">The world coords.</param>
		public Vector3 GetPositionOffset(Vector3 worldCoords)
		{
			return API.GetOffsetFromEntityGivenWorldCoords(Handle, worldCoords.X, worldCoords.Y, worldCoords.Z);
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
			return new Blip(API.AddBlipForEntity(Handle));
		}
		/// <summary>
		/// Gets the <see cref="Blip"/> attached to this <see cref="Entity"/>
		/// </summary>
		/// <remarks>returns <c>null</c> if no <see cref="Blip"/>s are attached to this <see cref="Entity"/></remarks>
		public Blip AttachedBlip
		{
			get
			{
				int handle = API.GetBlipFromEntity(Handle);

				if (API.DoesBlipExist(handle))
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
				return World.GetAllBlips().Where(x => x.Entity != null && x.Entity.Exists() && x.Entity.Handle == Handle).ToArray();
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
			API.AttachEntityToEntity(Handle, entity.Handle, -1, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, false, false, false, false, 2, true);
		}
		/// <summary>
		/// Attaches this <see cref="Entity"/> to a different <see cref="Entity"/>
		/// </summary>
		/// <param name="entityBone">The <see cref="EntityBone"/> to attach this <see cref="Entity"/> to.</param>
		/// <param name="position">The position relative to the <paramref name="entityBone"/> to attach this <see cref="Entity"/> to.</param>
		/// <param name="rotation">The rotation to apply to this <see cref="Entity"/> relative to the <paramref name="entityBone"/></param>
		public void AttachTo(EntityBone entityBone, Vector3 position = default(Vector3), Vector3 rotation = default(Vector3))
		{
			API.AttachEntityToEntity(Handle, entityBone.Owner.Handle, entityBone, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, false, false, false, false, 2, true);
		}
		/// <summary>
		/// Detaches this <see cref="Entity"/> from any <see cref="Entity"/> it may be attached to.
		/// </summary>
		public void Detach()
		{
			API.DetachEntity(Handle, true, true);
		}
		/// <summary>
		/// Determines whether this <see cref="Entity"/> is attached to any other <see cref="Entity"/>.
		/// </summary>
		/// <returns>
		///   <c>true</c> if this <see cref="Entity"/> is attached to another <see cref="Entity"/>; otherwise, <c>false</c>.
		/// </returns>
		public bool IsAttached()
		{
			return API.IsEntityAttached(Handle);
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
			return API.IsEntityAttachedToEntity(Handle, entity.Handle);
		}
		/// <summary>
		/// Gets the <see cref="Entity"/> this <see cref="Entity"/> is attached to.
		/// <remarks>returns <c>null</c> if this <see cref="Entity"/> isnt attached to any entity</remarks>
		/// </summary>
		public Entity GetEntityAttachedTo()
		{
			return FromHandle(API.GetEntityAttachedTo(Handle));
		}

		/// <summary>
		/// Applies a force to this <see cref="Entity"/>.
		/// </summary>
		/// <param name="direction">The direction to apply the force relative to world coords.</param>
		/// <param name="rotation">The rotation force to apply</param>
		/// <param name="forceType">Type of the force to apply.</param>
		public void ApplyForce(Vector3 direction, Vector3 rotation = default(Vector3), ForceType forceType = ForceType.MaxForceRot2)
		{
			API.ApplyForceToEntity(Handle, (int)forceType, direction.X, direction.Y, direction.Z, rotation.X, rotation.Y, rotation.Z, 0, false, true, true, false, true);
		}
		/// <summary>
		/// Applies a force to this <see cref="Entity"/>.
		/// </summary>
		/// <param name="direction">The direction to apply the force relative to this <see cref="Entity"/>s rotation</param>
		/// <param name="rotation">The rotation force to apply</param>
		/// <param name="forceType">Type of the force to apply.</param>
		public void ApplyForceRelative(Vector3 direction, Vector3 rotation = default(Vector3), ForceType forceType = ForceType.MaxForceRot2)
		{
			API.ApplyForceToEntity(Handle, (int)forceType, direction.X, direction.Y, direction.Z, rotation.X, rotation.Y, rotation.Z, 0, true, true, true, false, true);
		}

		/// <summary>
		/// Stops all particle effects attached to this <see cref="Entity"/>
		/// </summary>
		public void RemoveAllParticleEffects()
		{
			API.RemoveParticleFxFromEntity(Handle);
		}

		/// <summary>
		/// Gets the network ID of this <see cref="Entity"/>
		/// </summary>
		public int NetworkId
		{
			get
			{
				return API.NetworkGetNetworkIdFromEntity(Handle);
			}
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
			// prevent the game from crashing if this is called on the player ped.
			if (Handle != Game.PlayerPed.Handle)
			{
				API.SetEntityAsMissionEntity(Handle, false, true);
				int handle = Handle;
				API.DeleteEntity(ref handle);
				Handle = handle;
			}
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
			API.SetEntityAsMissionEntity(Handle, false, true);

			int handle = Handle;

			API.SetEntityAsNoLongerNeeded(ref handle);

			Handle = handle;
		}

		/// <summary>
		/// Gets the <see cref="StateBag"/> of this <see cref="Entity"/>
		/// </summary>
		public StateBag State
		{
			get
			{
				return new StateBag("entity:" + NetworkId);
			}
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
			switch (API.GetEntityType(handle))
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
		/// Creates a new instance of an <see cref="Entity"/> from the given network ID.
		/// </summary>
		/// <param name="networkId">The network ID of the entity.</param>
		/// <returns>Returns a <see cref="Ped"/> if this network ID corresponds to a Ped.
		/// Returns a <see cref="Vehicle"/> if this network ID corresponds to a Vehicle.
		/// Returns a <see cref="Prop"/> if this network ID corresponds to a Prop.
		/// Returns <c>null</c> if no <see cref="Entity"/> exists for the specified <paramref name="networkId"/></returns>
		public static Entity FromNetworkId(int networkId)
		{
			return Entity.FromHandle(API.NetworkGetEntityFromNetworkId(networkId));
		}

		/// <summary>
		/// Determines whether this <see cref="Entity"/> exists.
		/// </summary>
		/// <returns><c>true</c> if this <see cref="Entity"/> exists; otherwise, <c>false</c></returns>
		public override bool Exists()
		{
			return API.DoesEntityExist(Handle);
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
