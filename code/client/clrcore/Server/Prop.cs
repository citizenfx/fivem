#if MONO_V2
namespace CitizenFX.Server
#else
namespace CitizenFX.Core
#endif
{
	public sealed class Prop : Entity
#if MONO_V2
	, Shared.IObject
#endif
{
		public Prop(int handle) : base(handle)
		{
		}
	}
}
