namespace CitizenFX.Core
{
	internal interface IInternalHost
	{
		void InvokeNative(ref fxScriptContext context);
	}
}
