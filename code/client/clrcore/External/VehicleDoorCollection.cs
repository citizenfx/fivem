using System;
using System.Collections.Generic;

namespace CitizenFX.Core
{
	public enum VehicleDoorIndex
	{
		FrontLeftDoor,
		FrontRightDoor,
		BackLeftDoor,
		BackRightDoor,
		Hood,
		Trunk
	}

	public sealed class VehicleDoorCollection
	{
		#region Fields

		Vehicle _owner;
		readonly Dictionary<VehicleDoorIndex, VehicleDoor> _vehicleDoors = new Dictionary<VehicleDoorIndex, VehicleDoor>();

		#endregion

		internal VehicleDoorCollection(Vehicle owner)
		{
			_owner = owner;
		}

		public VehicleDoor this[VehicleDoorIndex index]
		{
			get
			{
				if (!_vehicleDoors.TryGetValue(index, out VehicleDoor vehicleDoor))
				{
					vehicleDoor = new VehicleDoor(_owner, index);
					_vehicleDoors.Add(index, vehicleDoor);
				}

				return vehicleDoor;
			}
		}

		public bool HasDoor(VehicleDoorIndex door)
		{
			switch (door)
			{
				case VehicleDoorIndex.FrontLeftDoor:
					return _owner.Bones.HasBone("door_dside_f");
				case VehicleDoorIndex.FrontRightDoor:
					return _owner.Bones.HasBone("door_pside_f");
				case VehicleDoorIndex.BackLeftDoor:
					return _owner.Bones.HasBone("door_dside_r");
				case VehicleDoorIndex.BackRightDoor:
					return _owner.Bones.HasBone("door_pside_r");
				case VehicleDoorIndex.Hood:
					return _owner.Bones.HasBone("bonnet");
				case VehicleDoorIndex.Trunk:
					return _owner.Bones.HasBone("boot");
			}
			return false;
		}

		public VehicleDoor[] GetAll()
		{
			var result = new List<VehicleDoor>();
			foreach (VehicleDoorIndex doorindex in Enum.GetValues(typeof(VehicleDoorIndex)))
			{
				if (HasDoor(doorindex))
					result.Add(this[doorindex]);
			}
			return result.ToArray();
		}

		public IEnumerator<VehicleDoor> GetEnumerator()
		{
			return (GetAll() as IEnumerable<VehicleDoor>).GetEnumerator();
		}

	}
}
