using System;
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
		public Entity(int handle) : base(handle)
		{
		}

        /// <summary>
		/// Gets the health of this <see cref="Entity"/> as an <see cref="int"/>.
		/// </summary>
		/// <value>
		/// The health from 0 - 100 as an integer.
		/// </value>
		public int Health
        {
            get
            {
                return API.GetEntityHealth(Handle) - 100;
            }
        }

        /// <summary>
        /// Gets the maximum health of this <see cref="Entity"/> as an <see cref="int"/>.
        /// </summary>
        /// <value>
        /// The maximum health from 0 - 100 as an integer.
        /// </value>
        public int MaxHealth
        {
            get
            {
                return API.GetEntityMaxHealth(Handle) - 100;
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
                // TODO: Replace with IsEntityDead() if that becomes available server side
                return this.Health == 0;
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
				return API.GetEntityCoords(this.Handle);
			}
			set
			{
				API.SetEntityCoords(this.Handle, value.X, value.Y, value.Z, false, false, false, true);
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
				return API.GetEntityRotation(this.Handle);
			}
			set
			{
				API.SetEntityRotation(this.Handle, value.X, value.Y, value.Z, 2, true);
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
				return API.GetEntityHeading(this.Handle);
			}
			set
			{
				API.SetEntityHeading(this.Handle, value);
			}
		}

		/// <summary>
		/// Sets a value indicating whether this <see cref="Entity"/> should be frozen.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> position should be frozen; otherwise, <c>false</c>.
		/// </value>
		public bool IsPositionFrozen
		{
			set
			{
				API.FreezeEntityPosition(this.Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets the velocity of this <see cref="Entity"/>.
		/// </summary>
		public Vector3 Velocity
		{
			get
			{
				return API.GetEntityVelocity(this.Handle);
			}
			set
			{
				API.SetEntityVelocity(this.Handle, value.X, value.Y, value.Z);
			}
		}
		/// <summary>
		/// Gets the rotation velocity of this <see cref="Entity"/>.
		/// </summary>
		public Vector3 RotationVelocity
		{
			get
			{
				return API.GetEntityRotationVelocity(this.Handle);
			}
		}

		/// <summary>
		/// Gets the model of the this <see cref="Entity"/>.
		/// </summary>
		public int Model
		{
			get
			{
				return API.GetEntityModel(this.Handle);
			}
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

        /// CM-TODO Add after adding Blip
        /// <summary>
        /// Creates a <see cref="Blip"/> on this <see cref="Entity"/>
        /// </summary>
        public Blip AttachBlip()
        {
            return new Blip(API.AddBlipForEntity(Handle));
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
		/// Gets the network owner of the this <see cref="Entity"/>.
		/// </summary>
		/// <returns>Returns the <see cref="Player"/> of the network owner.
		/// Returns <c>null</c> if this <see cref="Entity"/> is in an unowned state.</returns>
		public Player Owner
		{
			get
			{
				int playerHandle = API.NetworkGetEntityOwner(this.Handle);

				return playerHandle == -1 ? null : new Player(Convert.ToString(playerHandle));
			}
		}

		/// <summary>
		/// Gets the network ID of the this <see cref="Entity"/>.
		/// </summary>
		public int NetworkId
		{
			get
			{
				return API.NetworkGetNetworkIdFromEntity(this.Handle);
			}
		}

        /// <summary>
		/// Deletes this <see cref="Entity"/>
		/// </summary>
        [SecuritySafeCritical]
        private void Delete()
        {
            // prevent the game from crashing if this is called on the player ped.
            if (!API.IsPedAPlayer(this.Handle))
            {
                API.DeleteEntity(this.Handle);
            }
        }

		/// <summary>
		/// Gets the type of this <see cref="Entity"/>.
		/// </summary>
		/// <returns>Returns 1 if this <see cref="Entity"/> is a Ped.
		/// Returns 2 if this <see cref="Entity"/> is a Vehicle.
		/// Returns 3 if this <see cref="Entity"/> is a Prop.</returns>
		public int Type
		{
			get
			{
				return API.GetEntityType(this.Handle);
			}
		}

		/// <summary>
		/// Gets the <see cref="StateBag"/> of this <see cref="Entity"/>
		/// </summary>
		public StateBag State
		{
			get
			{
				API.EnsureEntityStateBag(this.Handle);
				return new StateBag("entity:" + this.NetworkId);
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
		/// <param name="networkId">The entity network ID.</param>
		/// <returns>Returns a <see cref="Ped"/> if this network ID corresponds to a Ped.
		/// Returns a <see cref="Vehicle"/> if this network ID corresponds to a Vehicle.
		/// Returns a <see cref="Prop"/> if this network ID corresponds to a Prop.
		/// Returns <c>null</c> if no <see cref="Entity"/> exists this the specified <paramref name="networkId"/></returns>
		public static Entity FromNetworkId(int networkId)
		{
			int handle = API.NetworkGetEntityFromNetworkId(networkId);

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
        /// Determines whether this <see cref="Entity"/> exists.
        /// </summary>
        /// <returns><c>true</c> if this <see cref="Entity"/> exists; otherwise, <c>false</c></returns>
        public bool Exists()
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
			return !ReferenceEquals(entity, null) && this.Handle == entity.Handle;
		}
		public override bool Equals(object obj)
		{
			return !ReferenceEquals(obj, null) && obj.GetType() == this.GetType() && this.Equals((Entity)obj);
		}

		public override int GetHashCode()
		{
			return this.Handle.GetHashCode();
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
