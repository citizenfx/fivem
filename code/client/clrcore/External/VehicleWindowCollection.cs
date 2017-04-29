using CitizenFX.Core.Native;
using System;
using System.Collections.Generic;

namespace CitizenFX.Core
{
	public enum VehicleWindowIndex
	{
		FrontRightWindow = 1,
		FrontLeftWindow = 0,
		BackRightWindow = 3,
		BackLeftWindow = 2,
		//Exe lists 8 possible windows
		ExtraWindow1 = 4,
		ExtraWindow2 = 5,
		ExtraWindow3 = 6,
		ExtraWindow4 = 7
	}

	public sealed class VehicleWindowCollection
	{
		#region Fields

		Vehicle _owner;
		readonly Dictionary<VehicleWindowIndex, VehicleWindow> _vehicleWindows = new Dictionary<VehicleWindowIndex, VehicleWindow>();

		#endregion

		internal VehicleWindowCollection(Vehicle owner)
		{
			_owner = owner;
		}


		public VehicleWindow this[VehicleWindowIndex index]
		{
			get
			{
				VehicleWindow vehicleWindow = null;

				if (!_vehicleWindows.TryGetValue(index, out vehicleWindow))
				{
					vehicleWindow = new VehicleWindow(_owner, index);
					_vehicleWindows.Add(index, vehicleWindow);
				}

				return vehicleWindow;
			}
		}

		public bool AreAllWindowsIntact
		{
			get { return Function.Call<bool>(Hash.ARE_ALL_VEHICLE_WINDOWS_INTACT, _owner.Handle); }
		}

		public void RollDownAllWindows()
		{
			Function.Call(Hash.ROLL_DOWN_WINDOWS, _owner.Handle);
		}
	}
}
