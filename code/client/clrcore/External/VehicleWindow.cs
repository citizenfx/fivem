using System;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
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
				return Function.Call<bool>(Hash.IS_VEHICLE_WINDOW_INTACT, _owner.Handle, Index);
			}
		}
		public Vehicle Vehicle
		{
			get { return _owner; }
		}

		public void Repair()
		{
			Function.Call(Hash.FIX_VEHICLE_WINDOW, _owner.Handle, Index);
		}
		public void Smash()
		{
			Function.Call(Hash.SMASH_VEHICLE_WINDOW, _owner.Handle, Index);
		}
		public void RollUp()
		{
			Function.Call(Hash.ROLL_UP_WINDOW, _owner.Handle, Index);
		}
		public void RollDown()
		{
			Function.Call(Hash.ROLL_DOWN_WINDOW, _owner.Handle, Index);
		}
		public void Remove()
		{
			Function.Call(Hash.REMOVE_VEHICLE_WINDOW, _owner.Handle, Index);
		}
	}
}
