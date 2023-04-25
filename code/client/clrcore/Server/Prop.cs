using System;
using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
#if MONO_V2
	public sealed class Object : Entity, Shared.IObject
	{
		public Object(int handle) : base(handle)
		{
		}
	}
#else
	public sealed class Prop : Entity
	{
		public Prop(int handle) : base(handle)
		{
		}
	}
#endif
}
