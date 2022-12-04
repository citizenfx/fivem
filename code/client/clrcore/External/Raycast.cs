#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{
	public struct RaycastResult
	{
		public RaycastResult(int handle) : this()
		{
			Vector3 hitPositionArg = new Vector3();
			bool hitSomethingArg = false;
			int entityHandleArg = 0;
			Vector3 surfaceNormalArg = new Vector3();
			uint materialArg = 0;

			Result = API.GetShapeTestResultEx( handle, ref hitSomethingArg, ref hitPositionArg, ref surfaceNormalArg, ref materialArg, ref entityHandleArg );

			DitHit = hitSomethingArg;
			HitPosition = hitPositionArg;
			SurfaceNormal = surfaceNormalArg;
			HitEntity = Entity.FromHandle( entityHandleArg );
			Material = (MaterialHash) materialArg;
		}

		/// <summary>
		/// Gets the <see cref="Entity" /> this raycast collided with.
		/// <remarks>Returns <c>null</c> if the raycast didnt collide with any <see cref="Entity"/>.</remarks>
		/// </summary>
		public Entity HitEntity { get; private set; }
		/// <summary>
		/// Gets the world coordinates where this raycast collided.
		/// <remarks>Returns <see cref="Vector3.Zero"/> if the raycast didnt collide with anything.</remarks>
		/// </summary>
		public Vector3 HitPosition { get; private set; }
		/// <summary>
		/// Gets the normal of the surface where this raycast collided.
		/// <remarks>Returns <see cref="Vector3.Zero"/> if the raycast didnt collide with anything.</remarks>
		/// </summary>
		public Vector3 SurfaceNormal { get; private set; }

		/// <summary>
		/// Gets a value indicating whether this raycast collided with anything.
		/// </summary>
		public bool DitHit { get; private set; }
		/// <summary>
		/// Gets a value indicating whether this raycast collided with any <see cref="Entity"/>.
		/// </summary>
		public bool DitHitEntity
		{
			get
			{
				return !ReferenceEquals(HitEntity, null);
			}
		}

		/// <summary>
		/// Gets a value indicating the material type of the collision.
		/// </summary>
		public MaterialHash Material { get; private set; }

		public int Result { get; private set; }
	}
}
