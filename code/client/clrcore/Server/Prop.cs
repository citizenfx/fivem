#if MONO_V2
namespace CitizenFX.Server
{
	public sealed class Object : Entity, Shared.IObject
	{
		public Object(int handle) : base(handle)
		{
		}
	}
#else
namespace CitizenFX.Core
{
	public sealed class Prop : Entity
	{
		public Prop(int handle) : base(handle)
		{
		}
	}
#endif
}
