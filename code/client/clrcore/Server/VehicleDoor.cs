using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	public sealed class VehicleDoor
	{
		#region Fields
		Vehicle _owner;
		#endregion

		internal VehicleDoor(Vehicle owner, VehicleDoorIndex index)
		{
			_owner = owner;
			Index = index;
		}

		public VehicleDoorIndex Index { get; private set; }

		public Vehicle Vehicle
		{
			get { return _owner; }
		}

		public void Break(bool stayInTheWorld = true)
		{
			API.SetVehicleDoorBroken(_owner.Handle, (int)Index, !stayInTheWorld);
		}
	}
}