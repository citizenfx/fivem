using CitizenFX.Core.Native;
using System;
using System.Collections.Generic;
using System.Security;

#if MONO_V2
namespace CitizenFX.FiveM
#else
namespace CitizenFX.Core
#endif
{
	public sealed class VehicleWheelCollection
	{
		#region Fields
		Vehicle _owner;
		readonly Dictionary<int, VehicleWheel> _vehicleWheels = new Dictionary<int, VehicleWheel>();
		#endregion

		internal VehicleWheelCollection(Vehicle owner)
		{
			_owner = owner;
		}

		public VehicleWheel this[int index]
		{
			get
			{
				VehicleWheel vehicleWheel = null;

				if (!_vehicleWheels.TryGetValue(index, out vehicleWheel))
				{
					vehicleWheel = new VehicleWheel(_owner, index);
					_vehicleWheels.Add(index, vehicleWheel);
				}

				return vehicleWheel;
			}
		}

		public int Count
		{
#if MONO_V2
			[SecuritySafeCritical] get => MemoryAccess.ReadIfNotNull(_owner.MemoryAddress, Game.Version >= GameVersion.v1_0_372_2_Steam ? 0xAA8 : 0xA88, 0);
#else
			get
			{
				if (_owner.MemoryAddress == IntPtr.Zero)
				{
					return 0;
				}

				int offset = Game.Version >= GameVersion.v1_0_372_2_Steam ? 0xAA8 : 0xA88;

				return MemoryAccess.ReadInt(_owner.MemoryAddress + offset);
			}
#endif
		}
	}
}
