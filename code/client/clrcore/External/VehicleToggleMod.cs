#if MONO_V2
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
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
			get { return API.IsToggleModOn(_owner.Handle, (int)ModType); }
			set { API.ToggleVehicleMod(_owner.Handle, (int)ModType, value); }
		}

		public string LocalizedModTypeName
		{
			get { return API.GetModSlotName(_owner.Handle, (int)ModType); }
		}

		public Vehicle Vehicle
		{
			get { return _owner; }
		}

		public void Remove()
		{
			API.RemoveVehicleMod(_owner.Handle, (int)ModType);
		}
	}
}
