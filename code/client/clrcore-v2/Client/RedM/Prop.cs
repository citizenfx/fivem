namespace CitizenFX.RedM
{
	public sealed class Prop : Entity, Shared.IObject
	{
		public Prop(int handle) : base(handle)
		{
		}

		/// <value>
		/// Returns the <see cref="Prop" /> or null if <paramref name="handle"/> is 0
		/// </value>
		public static Prop GetPropOrNull(int handle) {
			return handle == 0 ? null : new Prop(handle);
		}
	}
}
