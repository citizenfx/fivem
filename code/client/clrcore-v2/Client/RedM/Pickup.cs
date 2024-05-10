using System;
using CitizenFX.Core;
using CitizenFX.RedM.Native;

namespace CitizenFX.RedM
{
	public class Pickup : PoolObject, IEquatable<Pickup>
	{
		public Pickup(int handle) : base(handle)
		{
		}
		
		public Vector3 Position => Natives.GetPickupCoords(Handle);

		public bool IsCollected => Natives.HasPickupBeenCollected(Handle);

		public override void Delete()
		{
			Natives.RemovePickup(Handle);
		}

		public override bool Exists()
		{
			return Natives.DoesPickupExist(Handle);
		}
		
		public static bool Exists(Pickup pickup)
		{
			return !ReferenceEquals(pickup, null) && pickup.Exists();
		}
		
		public bool ObjectExists => Natives.DoesPickupObjectExist(Handle);

		public bool Equals(Pickup pickup)
		{
			return !ReferenceEquals(pickup, null) && Handle == pickup.Handle;
		}
		public sealed override bool Equals(object obj)
		{
			return !ReferenceEquals(obj, null) && obj.GetType() == GetType() && Equals((Pickup)obj);
		}

		public sealed override int GetHashCode()
		{
			return Handle;
		}

		public static bool operator ==(Pickup left, Pickup right)
		{
			return ReferenceEquals(left, null) ? ReferenceEquals(right, null) : left.Equals(right);
		}
		public static bool operator !=(Pickup left, Pickup right)
		{
			return !(left == right);
		}
	
	}
}
