using CitizenFX.Core;

namespace CitizenFX.Shared
{
	public enum EntityType
	{
		None = 0,
		Ped = 1,
		Vehicle = 2,
		Object = 3
	}

	public interface IEntity
	{
		/// <summary>
		/// Gets or sets the position of this <see cref="IEntity"/>.
		/// </summary>
		/// <value>
		/// The position in world space.
		/// </value>
		Vector3 Position { get; set; }

		/// <summary>
		/// Gets or sets the rotation of this <see cref="IEntity"/>.
		/// </summary>
		/// <value>
		/// The yaw, pitch, roll rotation values.
		/// </value>
		Vector3 Rotation { get; set; }

		/// <summary>
		/// Gets or sets the heading of this <see cref="IEntity"/>.
		/// </summary>
		/// <value>
		/// The heading in degrees.
		/// </value>
		float Heading { get; set; }

		/// <summary>
		/// Sets a value indicating whether this <see cref="IEntity"/> should be frozen.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="IEntity"/> position should be frozen; otherwise, <c>false</c>.
		/// </value>
		bool IsPositionFrozen { set; }

		/// <summary>
		/// Gets or sets the velocity of this <see cref="IEntity"/>.
		/// </summary>
		Vector3 Velocity { get; set; }
	
		// RDR doesn't have a known native for rotation velocity.
#if !IS_RDR3
		/// <summary>
		/// Gets the rotation velocity of this <see cref="IEntity"/>.
		/// </summary>
		Vector3 RotationVelocity { get; }
#endif

		/// <summary>
		/// Gets the model of the this <see cref="IEntity"/>.
		/// </summary>
		uint Model { get; }

		/// <summary>
		/// Gets the network owner of the this <see cref="IEntity"/>.
		/// </summary>
		/// <returns>Returns the <see cref="Player"/> of the network owner.
		/// Returns <c>null</c> if this <see cref="IEntity"/> is in an unowned state.</returns>
		Player Owner { get; }

		/// <summary>
		/// Gets the network ID of the this <see cref="IEntity"/>.
		/// </summary>
		int NetworkId { get; }

		/// <summary>
		/// Gets the type of this <see cref="IEntity"/>.
		/// </summary>
		/// <returns>Returns 1 if this <see cref="IEntity"/> is a Ped.
		/// Returns 2 if this <see cref="IEntity"/> is a Vehicle.
		/// Returns 3 if this <see cref="IEntity"/> is a Prop.</returns>
		EntityType Type { get; }

		/// <summary>
		/// Gets the <see cref="StateBag"/> of this <see cref="IEntity"/>
		/// </summary>
		StateBag State { get; }
	}
}
