using System;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
{

	public sealed class VehicleWheel
	{
		#region Fields
		Vehicle _owner;
		#endregion

		internal VehicleWheel(Vehicle owner, int index)
		{
			_owner = owner;
			Index = index;
		}

		public int Index { get; private set; }

		public Vehicle Vehicle
		{
			get { return _owner; }
		}

		public void Burst()
		{
			Function.Call(Hash.SET_VEHICLE_TYRE_BURST, _owner.Handle, Index, true, 1000f);
		}
		public void Fix()
		{
			Function.Call(Hash.SET_VEHICLE_TYRE_FIXED, _owner.Handle, Index);
		}
	}
}
