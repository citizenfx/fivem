using System;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
{

	public sealed class VehicleToggleMod
	{
		#region Fields

		Vehicle _owner;

		#endregion

		internal VehicleToggleMod(Vehicle owner, VehicleToggleModType modType)
		{
			_owner = owner;
			ModType = modType;
		}

		public VehicleToggleModType ModType { get; private set; }

		public bool IsInstalled
		{
			get { return Function.Call<bool>(Hash.IS_TOGGLE_MOD_ON, _owner.Handle, ModType); }
			set { Function.Call(Hash.TOGGLE_VEHICLE_MOD, _owner.Handle, ModType, value); }
		}

		public string LocalizedModTypeName
		{
			get { return Function.Call<string>(Hash.GET_MOD_SLOT_NAME, _owner.Handle, ModType); }
		}

		public Vehicle Vehicle
		{
			get { return _owner; }
		}

		public void Remove()
		{
			Function.Call(Hash.REMOVE_VEHICLE_MOD, _owner.Handle, ModType);
		}
	}
}
