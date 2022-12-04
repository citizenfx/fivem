using System;
using System.Collections.Generic;
using System.Linq;

#if MONO_V2
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
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

		public VehicleWindow[] GetAllWindows()
		{
			return
				Enum.GetValues(typeof(VehicleWindowIndex)).Cast<VehicleWindowIndex>().Where(HasWindow).Select(windowIndex => this[windowIndex]).ToArray();
		}

		public bool AreAllWindowsIntact
		{
			get { return API.AreAllVehicleWindowsIntact(_owner.Handle); }
		}

		public void RollDownAllWindows()
		{
			foreach(VehicleWindow vehicleWindow in this.GetAllWindows())
			{
				vehicleWindow.RollDown();
			}
		}

		public void RollUpAllWindows()
		{
			foreach (VehicleWindow vehicleWindow in this.GetAllWindows())
			{
				vehicleWindow.RollUp();
			}
		}

		public bool HasWindow(VehicleWindowIndex window)
		{
			switch (window)
			{
				case VehicleWindowIndex.FrontLeftWindow:
					return _owner.Bones.HasBone("window_lf");
				case VehicleWindowIndex.FrontRightWindow:
					return _owner.Bones.HasBone("window_rf");
				case VehicleWindowIndex.BackLeftWindow:
					return _owner.Bones.HasBone("window_lr");
				case VehicleWindowIndex.BackRightWindow:
					return _owner.Bones.HasBone("window_rr");
				default:
					return false;
			}
		}
	}
}
