using System;
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

		public float AngleRatio
		{
			get
			{
				return Function.Call<float>(Hash.GET_VEHICLE_DOOR_ANGLE_RATIO, _owner.Handle, Index);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_DOOR_CONTROL, _owner.Handle, Index, 1, value);
			}
		}
		public bool CanBeBroken
		{
			set
			{
				Function.Call(Hash._SET_VEHICLE_DOOR_BREAKABLE, _owner.Handle, Index, value);
			}
		}
		public bool IsOpen
		{
			get
			{
				return AngleRatio > 0;
			}
		}
		public bool IsFullyOpen
		{
			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_DOOR_FULLY_OPEN, _owner.Handle, Index);
			}
		}
		public bool IsBroken
		{
			get
			{
				return Function.Call<bool>(Hash.IS_VEHICLE_DOOR_DAMAGED, _owner.Handle, Index);
			}
		}
		public Vehicle Vehicle
		{
			get { return _owner; }
		}

		public void Open(bool loose = false, bool instantly = false)
		{
			Function.Call(Hash.SET_VEHICLE_DOOR_OPEN, _owner.Handle, Index, loose, instantly);
		}
		public void Close(bool instantly = false)
		{
			Function.Call(Hash.SET_VEHICLE_DOOR_SHUT, _owner.Handle, Index, instantly);
		}
		public void Break(bool stayInTheWorld = true)
		{
			Function.Call(Hash.SET_VEHICLE_DOOR_BROKEN, _owner.Handle, Index, !stayInTheWorld);
		}
	}
}
