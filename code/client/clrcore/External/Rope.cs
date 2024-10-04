using System;

#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{
	public enum RopeType
	{
		ThickRope = 4,
		ThinMetalWire = 5,
	}

	public sealed class Rope : PoolObject, IEquatable<Rope>
	{
		public Rope(int handle) : base(handle)
		{
		}

		public float Length
		{
			get
			{
				return API.GetRopeLength(Handle);
			}
			set
			{
				API.RopeForceLength(Handle, value);
			}
		}
		public int VertexCount
		{
			get
			{
				return API.GetRopeVertexCount(Handle);
			}
		}
		public void ResetLength(bool reset)
		{
			API.RopeResetLength(Handle, reset ? 1f : Length);
		}

		public void ActivatePhysics()
		{
			API.ActivatePhysics(Handle);
		}

		public void AttachEntity(Entity entity)
		{
			AttachEntity(entity, Vector3.Zero);
		}
		public void AttachEntity(Entity entity, Vector3 position)
		{
			API.AttachRopeToEntity(Handle, entity.Handle, position.X, position.Y, position.Z, false);
		}
		public void AttachEntities(Entity entityOne, Entity entityTwo, float length)
		{
			AttachEntities(entityOne, Vector3.Zero, entityTwo, Vector3.Zero, length);
		}
		public void AttachEntities(Entity entityOne, Vector3 positionOne, Entity entityTwo, Vector3 positionTwo, float length)
		{
			API.AttachEntitiesToRope(Handle, entityOne.Handle, entityTwo.Handle, positionOne.X, positionOne.Y, positionOne.Z, positionTwo.X, positionTwo.Y, positionTwo.Z, length, false, false, null, null);
		}
		public void DetachEntity(Entity entity)
		{
			API.DetachRopeFromEntity(Handle, entity.Handle);
		}

		public void PinVertex(int vertex, Vector3 position)
		{
			API.PinRopeVertex(Handle, vertex, position.X, position.Y, position.Z);
		}
		public void UnpinVertex(int vertex)
		{
			API.UnpinRopeVertex(Handle, vertex);
		}
		public Vector3 GetVertexCoord(int vertex)
		{
			return API.GetRopeVertexCoord(Handle, vertex);
		}

		public override void Delete()
		{
			int handle = Handle;
			API.DeleteRope(ref handle);
			Handle = handle;
		}

		public override bool Exists()
		{
			int handle = Handle;
			return API.DoesRopeExist(ref handle);
		}
		public static bool Exists(Rope rope)
		{
			return !ReferenceEquals(rope, null) && rope.Exists();
		}

		public bool Equals(Rope rope)
		{
			return !ReferenceEquals(rope, null) && Handle == rope.Handle;
		}
		public override bool Equals(object obj)
		{
			return !ReferenceEquals(obj, null) && obj.GetType() == GetType() && Equals((Rope)obj);
		}

		public sealed override int GetHashCode()
		{
			return Handle;
		}

		public static bool operator ==(Rope left, Rope right)
		{
			return ReferenceEquals(left, null) ? ReferenceEquals(right, null) : left.Equals(right);
		}
		public static bool operator !=(Rope left, Rope right)
		{
			return !(left == right);
		}
	}
}
