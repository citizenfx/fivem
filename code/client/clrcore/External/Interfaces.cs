#if MONO_V2
using CitizenFX.Core;
using INativeValue = CitizenFX.Core.Native.Input.Primitive;

namespace CitizenFX.FiveM
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

	public abstract class PoolObject : INativeValue, IDeletable
	{
#if !MONO_V2
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
#else
		protected PoolObject(int handle) : base(unchecked((ulong)handle))
		{ }

		public int Handle
		{
			get => unchecked((int)m_nativeValue);
			protected set => m_nativeValue = unchecked((ulong)value);
		}
#endif

		public abstract bool Exists();

		public abstract void Delete();
	}
}
