#if MONO_V2
using CitizenFX.Core;
using INativeValue = CitizenFX.Core.Native.Input.Primitive;

namespace CitizenFX.Server
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{
	public interface ISpatial
	{
		Vector3 Position { get; set; }
		Vector3 Rotation { get; set; }
	}

	public interface IExistable
	{
		bool Exists();
	}

	public interface IDeletable : IExistable
	{
		void Delete();
	}

	// reimplement "IDeletable" when API.DoesEntityExist(), API.SetEntityAsMissionEntity() and API.DeleteEntity() are implemented
	public abstract class PoolObject : INativeValue
	{
		protected PoolObject(int handle)
		{
			Handle = handle;
		}

		public int Handle { get; protected set; }
		public override ulong NativeValue
		{
			get { return (ulong)Handle; }
			set { Handle = unchecked((int)value); }
		}

		//public abstract bool Exists();

		//public abstract void Delete();
	}
}
