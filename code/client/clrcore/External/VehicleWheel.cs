#if MONO_V2
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
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
			API.SetVehicleTyreBurst(_owner.Handle, Index, true, 1000f);
		}
		public void Fix()
		{
			API.SetVehicleTyreFixed(_owner.Handle, Index);
		}
	}
}
