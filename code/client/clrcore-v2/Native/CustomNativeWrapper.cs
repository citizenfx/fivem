using CitizenFX.Core.Native;

#if IS_FXSERVER
namespace CitizenFX.Server.Native
#elif GTA_FIVE
namespace CitizenFX.FiveM.Native
#elif IS_RDR3
namespace CitizenFX.RedM.Native
#elif GTA_NY
namespace CitizenFX.LibertyM.Native
#else
#error namespace is missing
#endif
{
#if NATIVE_HASHES_INCLUDE
	public static partial class Natives
	{
		public static T Call<T>(Hash hash, params Argument[] arguments) => CustomNativeInvoker.Call<T>((ulong)hash, arguments);
		public static void Call(Hash hash, params Argument[] arguments) => CustomNativeInvoker.Call((ulong)hash, arguments);
		public static T Call<T>(ulong hash, params Argument[] arguments) => CustomNativeInvoker.Call<T>(hash, arguments);
		public static void Call(ulong hash, params Argument[] arguments) => CustomNativeInvoker.Call(hash, arguments);
	}
#endif
}
