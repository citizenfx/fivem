#if MONO_V2
namespace CitizenFX.Server
{
	public sealed class Vehicle : Entity, Shared.IVehicle
#else
namespace CitizenFX.Core
{
	public sealed class Vehicle : Entity
#endif
	{
		public Vehicle(int handle) : base(handle)
		{
		}
	}
}
