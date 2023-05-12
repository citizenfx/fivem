#if MONO_V2
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{

	public sealed class VehicleWindow
	{
		#region Fields
		Vehicle _owner;

		#endregion

		internal VehicleWindow(Vehicle owner, VehicleWindowIndex index)
		{
			_owner = owner;
			Index = index;
		}

		public VehicleWindowIndex Index { get; private set; }

		public bool IsIntact
		{
			get
			{
				return API.IsVehicleWindowIntact(_owner.Handle, (int)Index);
			}
		}
		public Vehicle Vehicle
		{
			get { return _owner; }
		}

		public void Repair()
		{
			API.FixVehicleWindow(_owner.Handle, (int)Index);
		}
		public void Smash()
		{
			API.SmashVehicleWindow(_owner.Handle, (int)Index);
		}
		public void RollUp()
		{
			API.RollUpWindow(_owner.Handle, (int)Index);
		}
		public void RollDown()
		{
			API.RollDownWindow(_owner.Handle, (int)Index);
		}
		public void Remove()
		{
			API.RemoveVehicleWindow(_owner.Handle, (int)Index);
		}
	}
}
