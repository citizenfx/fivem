#if MONO_V2
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
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
				return API.GetVehicleDoorAngleRatio(_owner.Handle, (int)Index);
			}
			set
			{
				API.SetVehicleDoorControl(_owner.Handle, (int)Index, 1, value);
			}
		}
		public bool CanBeBroken
		{
			set
			{
				API.SetVehicleDoorBreakable(_owner.Handle, (int)Index, value);
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
				return API.IsVehicleDoorFullyOpen(_owner.Handle, (int)Index);
			}
		}
		public bool IsBroken
		{
			get
			{
				return API.IsVehicleDoorDamaged(_owner.Handle, (int)Index);
			}
		}
		public Vehicle Vehicle
		{
			get { return _owner; }
		}

		public void Open(bool loose = false, bool instantly = false)
		{
			API.SetVehicleDoorOpen(_owner.Handle, (int)Index, loose, instantly);
		}
		public void Close(bool instantly = false)
		{
			API.SetVehicleDoorShut(_owner.Handle, (int)Index, instantly);
		}
		public void Break(bool stayInTheWorld = true)
		{
			API.SetVehicleDoorBroken(_owner.Handle, (int)Index, !stayInTheWorld);
		}
	}
}
