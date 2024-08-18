using System;

#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.Server.Native.Natives;
using Prop = CitizenFX.Server.Object;
using compat_int_uint = System.UInt32;
using compat_int_entity_type = CitizenFX.Shared.EntityType;
#else
using CitizenFX.Core.Native;
using compat_int_uint = System.Int32;
using compat_int_entity_type = System.Int32;
#endif

#if MONO_V2
namespace CitizenFX.Server
{
	public abstract class Entity : PoolObject, IEquatable<Entity>, ISpatial, Shared.IEntity
#else
namespace CitizenFX.Core
{
	public abstract class Entity : PoolObject, IEquatable<Entity>, ISpatial
#endif
	{
		public Entity(int handle) : base(handle)
		{
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
		public compat_int_uint Model
		{
			get
			{
				return unchecked((compat_int_uint)API.GetEntityModel(this.Handle));
			}
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

#if MONO_V2
		Shared.Player Shared.IEntity.Owner => Owner;
#endif

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
		/// Gets the type of this <see cref="Entity"/>.
		/// </summary>
		/// <returns>Returns 1 if this <see cref="Entity"/> is a Ped.
		/// Returns 2 if this <see cref="Entity"/> is a Vehicle.
		/// Returns 3 if this <see cref="Entity"/> is a Prop.</returns>
		public compat_int_entity_type Type => (compat_int_entity_type)API.GetEntityType(this.Handle);

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
