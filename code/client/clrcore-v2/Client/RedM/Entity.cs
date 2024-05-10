using System;
using CitizenFX.Core;
using CitizenFX.Shared;
using CitizenFX.RedM.Native;

namespace CitizenFX.RedM
{
	public abstract class Entity : PoolObject, ISpatial, IEquatable<Entity>, IEntity
	{
		public Entity(int handle) : base(handle)
		{
		
		}
		
		/// <summary>
		/// Gets or sets the health of this <see cref="Entity"/>
		/// </summary>
		/// <value>
		/// The health from 0 - 500 as an integer.
		/// </value>
		public int Health
		{
			get => Natives.GetEntityHealth(Handle) - 100;
			set => Natives.SetEntityHealth(Handle, value + 100, 0);
		}
		
		/// <summary>
		/// Gets or sets the position of this <see cref="Entity"/>.
		/// </summary>
		/// <value>
		/// The position in world space.
		/// </value>
		public Vector3 Position
		{
			get => Natives.GetEntityCoords(Handle, true, false);
			set => Natives.SetEntityCoords(Handle, value.X, value.Y, value.Z, false, false, false, false);
		}
		
		/// <summary>
		/// Gets or sets the rotation of this <see cref="Entity"/>.
		/// </summary>
		/// <value>
		/// The yaw, pitch, roll rotation values.
		/// </value>
		public Vector3 Rotation
		{
			get => Natives.GetEntityRotation(Handle, 2);
			set => Natives.SetEntityRotation(Handle, value.X, value.Y, value.Z, 2, true);
		}

		/// <summary>
		/// Gets or sets the heading of this <see cref="Entity"/>.
		/// </summary>
		/// <value>
		/// The heading in degrees.
		/// </value>
		public float Heading
		{
			get => Natives.GetEntityHeading(Handle);
			set => Natives.SetEntityHeading(Handle, value);
		}

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Entity"/> is frozen.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Entity"/> is position frozen; otherwise, <c>false</c>.
		/// </value>
		public bool IsPositionFrozen
		{
			// TODO: Change this back to the native call whenever `IS_ENTITY_FROZEN` returns a bool instead of a long
			get => Natives.Call<bool>(Hash._IS_ENTITY_FROZEN, Handle);
			set => Natives.FreezeEntityPosition(Handle, value);
		}
		
		
		/// <summary>
		/// Gets or sets the velocity of this <see cref="Entity"/>.
		/// </summary>
		public Vector3 Velocity
		{
			get => Natives.GetEntityVelocity(Handle, 0);
			set => Natives.SetEntityVelocity(Handle, value.X, value.Y, value.Z);
		}
		
		/// <summary>
		/// Gets the current model of this entity
		/// </summary>
		public Model Model => new Model(Natives.GetEntityModel(Handle));
		
		uint Shared.IEntity.Model => Natives.GetEntityModel(Handle);

		/// <summary>
		/// Gets the current network owner of this entity
		/// </summary>
		public Shared.Player Owner => new Player(Natives.NetworkGetEntityOwner(Handle));

		/// <summary>
		/// Gets the network id of this <see cref="Entity" />
		/// </summary>
		public int NetworkId => Natives.NetworkGetNetworkIdFromEntity(Handle);
		
		/// <summary>
		/// Gets if the current <see cref="Entity" /> is networked.
		/// </summary>
		public bool IsNetworked => Natives.NetworkGetEntityIsNetworked(Handle);
		
		public EntityType Type => (EntityType)Natives.GetEntityType(Handle);
		
		/// <summary>
		/// Gets the <see cref="StateBag"/> of this <see cref="Entity"/>
		/// </summary>
		public StateBag State
		{
			get
			{
				if (!Natives.NetworkGetEntityIsNetworked(Handle))
				{
					Natives.EnsureEntityStateBag(Handle);

					return new StateBag($"localEntity:{Handle}");
				}

				return new StateBag("entity:" + NetworkId);
			}
		}

		/// <summary>
		/// Determines whether this <see cref="Entity"/> exists.
		/// </summary>
		/// <returns><c>true</c> if this <see cref="Entity"/> exists; otherwise, <c>false</c></returns>
		public override bool Exists()
		{
			return Natives.DoesEntityExist(Handle);
		}
		
		/// <summary>
		/// Deletes this <see cref="Entity"/>
		/// </summary>
		public override void Delete()
		{
			Natives.SetEntityAsMissionEntity(Handle, false, true);
			int handle = Handle;
			Natives.DeleteEntity(ref handle);
			Handle = handle;
		}
		
		public bool Equals(Entity entity)
		{
			return !ReferenceEquals(entity, null) && Handle == entity.Handle;
		}
	}
}
