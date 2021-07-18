using System.Collections.Generic;

namespace CitizenFX.Core
{
	public enum VehicleDoorIndex
	{
		FrontRightDoor = 1,
		FrontLeftDoor = 0,
		BackRightDoor = 3,
		BackLeftDoor = 2,
		Hood = 4,
		Trunk = 5
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
				VehicleDoor vehicleDoor = null;

				if (!_vehicleDoors.TryGetValue(index, out vehicleDoor))
				{
					vehicleDoor = new VehicleDoor(_owner, index);
					_vehicleDoors.Add(index, vehicleDoor);
				}

				return vehicleDoor;
			}
		}
	}
}
