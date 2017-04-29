using CitizenFX.Core.Native;

namespace CitizenFX.Core
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

		public abstract bool Exists();

		public abstract void Delete();
	}
}
