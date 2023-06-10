
namespace CitizenFX.Core
{
	/// <summary>
	/// Reference to a resource that may or may not exist, can be used as a shortcut to call its exports
	/// <see cref="Resource.Current"/> contains the information of this script's resource
	/// </summary>
	/// <remarks>Doesn't guarantee the resource is initialized, active, or has an exports ready.</remarks>
	public class Resource
	{
		/// <summary>
		/// Can be used to compile against server or client code
		/// </summary>
#if IS_FXSERVER
		public const bool IsServer = true;
#else
		public const bool IsServer = false;
#endif

		/// <summary>
		/// Get this script's resource information
		/// </summary>
		/// <remarks>Don't use its <see cref="Exports"/> as that'll be quite slow, unless needed, e.g.: set in another assembly.</remarks>
		public static Resource Current { get; internal set; }

		/// <summary>
		/// The resource's name
		/// </summary>
		public string Name { get; }

		/// <summary>
		/// Can be used to call Export functions on this resource
		/// </summary>
		/// <remarks>Can fail just like calling exports normally as this type doesn't guarantee the resource exists,
		/// is initialized, active, or even has an export with that name.</remarks>
		public ResourceExports Exports { get; }

		/// <summary>
		/// Creates a reference to a resource that may or may not exist
		/// </summary>
		/// <param name="name">name of the resource we want to reference</param>
		/// <remarks>Doesn't guarantee the resource is initialized, active, or has an exports ready.</remarks>
		public Resource(string name)
		{
			Name = name;
			Exports = new ResourceExports(name);
		}
	}
}
