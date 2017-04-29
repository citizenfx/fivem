namespace CitizenFX.Core
{
	public interface IInternalHost
	{
		void InvokeNative(ref fxScriptContext context);
	}
}