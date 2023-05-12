// Mono has support for this, just missing some definitions
#if OS_WIN

namespace System.Runtime.CompilerServices
{
	public sealed class AsyncMethodBuilderAttribute : Attribute
	{
		public AsyncMethodBuilderAttribute(Type taskLike)
		{
		}
	}
}
#endif
