using CitizenFX.Core.Native;
using System;
using System.Collections.Generic;
using System.Security;

namespace CitizenFX.Core
{
	public delegate object LocalFunction(params object[] args);

	internal sealed class _LocalFunction : _ExternalFunction, IDisposable
	{
		internal _LocalFunction(string reference) : base(reference, repeatable: true)
		{
			CoreNatives.DuplicateFunctionReference(reference); // increment the refcount
		}

		public static Callback Create(string reference)
		{
			var localFunctionReference = new _LocalFunction(reference);
			return new Callback(localFunctionReference.Invoke);
		}

		~_LocalFunction()
		{
			FreeNativeReference();
		}

		public void Dispose()
		{
			FreeNativeReference();
			GC.SuppressFinalize(this);
		}

		private void FreeNativeReference()
		{
			CoreNatives.DeleteFunctionReference(m_reference);
		}

		public byte[] Duplicate()
		{
			return CoreNatives.DuplicateFunctionReference(m_reference);
		}

		internal unsafe Coroutine<object> Invoke(object[] args)
		{
			object[] returnData = CoreNatives.InvokeFunctionReference(m_reference, args);
			if (returnData != null && returnData.Length > 0)
			{
				var result = returnData[0];

				if (result is IDictionary<string, object> dict
					&& dict.TryGetValue("__cfx_async_retval", out object requestAsyncCallback)
					&& requestAsyncCallback is Callback callbackRequested)
				{
					var c = new Coroutine<object>();
					callbackRequested(new Action<object, object>((a, e) =>
					{
						if (e != null)
							c.SetException(new Exception(e.ToString()));
						else
							c.Complete(a);
					}));

					return c;
				}

				return Coroutine<object>.Completed(result);
			}

			return null;
		}
	}
}
