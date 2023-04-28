using System;
using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
	public sealed class Vehicle : Entity
#if MONO_V2
		, Shared.IVehicle
#endif
	{
		public Vehicle(int handle) : base(handle)
		{
		}
	}
}
