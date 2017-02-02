using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
	public struct RaycastResult
	{
        [SecuritySafeCritical]
        public RaycastResult(int handle) : this()
		{
			InitializeCritical(handle);
		}

		[SecurityCritical]
		private void InitializeCritical(int handle)
		{
			NativeVector3 hitPositionArg;
			bool hitSomethingArg;
			int entityHandleArg;
			NativeVector3 surfaceNormalArg;
			unsafe
			{
				Result = Function.Call<int>(Hash.GET_SHAPE_TEST_RESULT, handle, &hitSomethingArg, &hitPositionArg, &surfaceNormalArg, &entityHandleArg);
			}

			DitHit = hitSomethingArg;
			HitPosition = hitPositionArg;
			SurfaceNormal = surfaceNormalArg;
			HitEntity = Entity.FromHandle(entityHandleArg);
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

		public int Result { get; private set; }
	}
}
