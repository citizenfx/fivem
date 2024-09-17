using CitizenFX.Core.Native.Input;
using System;
using System.Security;

namespace CitizenFX.Core.Native
{
	internal static class CoreNatives
	{
		private static UIntPtr s_0xd233a168;
		private static UIntPtr s_0x5fa79b0f;
		private static UIntPtr s_0xd7664fd1;
		private static UIntPtr s_0x1e86f206;
		private static UIntPtr s_0xf4e2079d;
		private static UIntPtr s_0x637f4c75;
		private static UIntPtr s_0x8d50e33a;
		private static UIntPtr s_0x91310870;
		private static UIntPtr s_registerNuiCallback; // 0xc59b980c
		private static UIntPtr s_unregisterRawNuiCallback; // 0x7fb46432
#if IS_FXSERVER
		private static UIntPtr s_0x2f7a49e6;
		private static UIntPtr s_0x70b35890;
#else
		private static UIntPtr s_0x7fdd1128;
		private static UIntPtr s_0x128737ea;
#endif

		[SecuritySafeCritical]
		internal static unsafe void RegisterResourceAsEventHandler(CString eventName)
		{
			fixed (byte* p_eventName = eventName?.value)
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_eventName };
				ScriptContext.InvokeNative(ref s_0xd233a168, 0xd233a168, __data, 1); // REGISTER_RESOURCE_AS_EVENT_HANDLER
			}
		}

		[SecuritySafeCritical]
		internal static unsafe void RegisterNuiCallback(CString callbackName, InFunc handler)
		{
			fixed (byte* p_callbackType = callbackName?.value, p_handler = &handler.value[0])
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_callbackType, (ulong)p_handler };
				ScriptContext.InvokeNative(ref s_registerNuiCallback, 0xc59b980c, __data, 2); // REGISTER_NUI_CALLBACK
			}
		}
		
		
		[SecuritySafeCritical]
		internal static unsafe void RemoveNuiCallback(CString callbackName)
		{
			fixed (byte* p_callbackType = callbackName?.value)
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_callbackType };
				ScriptContext.InvokeNative(ref s_unregisterRawNuiCallback, 0x7fb46432, __data, 1); // UNREGISTER_RAW_NUI_CALLBACK
			}
		}

		[SecuritySafeCritical]
		public static unsafe void RegisterKeyMapping(CString commandString, CString description, CString defaultMapper, CString defaultParameter)
		{
			fixed (void* p_commandString = commandString?.value
				, p_description = description?.value
				, p_defaultMapper = defaultMapper?.value
				, p_defaultParameter = defaultParameter?.value)
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_commandString, (ulong)p_description, (ulong)p_defaultMapper, (ulong)p_defaultParameter };
				ScriptContext.InvokeNative(ref s_0xd7664fd1, 0xd7664fd1, __data, 4);
			}
		}
		
		[SecuritySafeCritical]
		internal static unsafe void RegisterCommand(CString commandName, InFunc handler, bool restricted)
		{
			fixed (byte* p_commandName = commandName?.value, p_handler = &handler.value[0])
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_commandName, (ulong)p_handler, N64.Val(restricted) };
				ScriptContext.InvokeNative(ref s_0x5fa79b0f, 0x5fa79b0f, __data, 3); // REGISTER_COMMAND
			}
		}

		[SecuritySafeCritical]
		internal static unsafe object GetStateBagValue(CString bagName, CString key)
		{
			fixed (void* p_bagName = bagName?.value, p_key = key?.value)
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_bagName, (ulong)p_key };
				ScriptContext.InvokeNative(ref s_0x637f4c75, 0x637f4c75, __data, 2);
				return ((OutPacket*)__data)->Deserialize(); // GET_STATE_BAG_VALUE
			}
		}

		[SecuritySafeCritical]
		internal static unsafe void SetStateBagValue(CString bagName, CString keyName, InPacket value, bool replicated)
		{
			fixed (void* p_bagName = bagName?.value, p_keyName = keyName?.value, p_valueData = value.value)
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_bagName, (ulong)p_keyName, (ulong)p_valueData, (ulong)value.value?.LongLength, N64.Val(replicated) };
				ScriptContext.InvokeNative(ref s_0x8d50e33a, 0x8d50e33a, __data, 5); // SET_STATE_BAG_VALUE
			}
		}

		[SecuritySafeCritical]
		internal static unsafe void DeleteFunctionReference(CString referenceIdentity)
		{
			fixed (void* p_referenceIdentity = referenceIdentity?.value)
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_referenceIdentity };
				ScriptContext.InvokeNative(ref s_0x1e86f206, 0x1e86f206, __data, 1); // DELETE_FUNCTION_REFERENCE
			}
		}

		[SecuritySafeCritical]
		internal static unsafe byte[] DuplicateFunctionReference(CString referenceIdentity)
		{
			fixed (void* p_referenceIdentity = referenceIdentity?.value)
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_referenceIdentity };
				ScriptContext.InvokeNative(ref s_0xf4e2079d, 0xf4e2079d, __data, 1); // DUPLICATE_FUNCTION_REFERENCE
				return (byte[])*(OutString*)__data;
			}
		}

		[SecuritySafeCritical]
		internal unsafe static void TriggerEventInternal(CString eventName, InPacket args)
		{
			fixed (byte* p_eventName = eventName?.value, p_args = args.value)
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_eventName, (ulong)p_args, (ulong)args.value?.LongLength };
				ScriptContext.InvokeNative(ref s_0x91310870, 0x91310870, __data, 3); // TRIGGER_EVENT_INTERNAL
			}
		}

#if IS_FXSERVER
		[SecuritySafeCritical]
		internal unsafe static void TriggerClientEventInternal(CString eventName, CString target, InPacket args)
		{
			fixed (byte* p_eventName = eventName?.value, p_target = target?.value, p_args = args.value)
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_eventName, (ulong)p_target, (ulong)p_args, (ulong)args.value?.LongLength };
				ScriptContext.InvokeNative(ref s_0x2f7a49e6, 0x2f7a49e6, __data, 4); // TRIGGER_CLIENT_EVENT_INTERNAL
			}
		}

		[SecuritySafeCritical]
		internal unsafe static void TriggerLatentClientEventInternal(CString eventName, CString target, InPacket args, int bytesPerSecond)
		{
			fixed (byte* p_eventName = eventName?.value, p_target = target?.value, p_args = args.value)
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_eventName, (ulong)p_target, (ulong)p_args, (ulong)args.value?.LongLength, N64.Val(bytesPerSecond) };
				ScriptContext.InvokeNative(ref s_0x70b35890, 0x70b35890, __data, 5); // TRIGGER_LATENT_CLIENT_EVENT_INTERNAL
			}
		}
#else
		[SecuritySafeCritical]
		internal unsafe static void TriggerServerEventInternal(CString eventName, InPacket args)
		{
			fixed (byte* p_eventName = eventName?.value, p_args = args.value)
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_eventName, (ulong)p_args, (ulong)args.value?.LongLength };
				ScriptContext.InvokeNative(ref s_0x7fdd1128, 0x7fdd1128, __data, 3); // TRIGGER_SERVER_EVENT_INTERNAL
			}
		}

		[SecuritySafeCritical]
		internal unsafe static void TriggerLatentServerEventInternal(CString eventName, InPacket args, int bytesPerSecond)
		{
			fixed (byte* p_eventName = eventName?.value, p_args = args.value)
			{
				ulong* __data = stackalloc ulong[] { (ulong)p_eventName, (ulong)p_args, (ulong)args.value?.LongLength, N64.Val(bytesPerSecond) };
				ScriptContext.InvokeNative(ref s_0x128737ea, 0x128737ea, __data, 4); // TRIGGER_LATENT_SERVER_EVENT_INTERNAL
			}
		}
#endif
	}

	public abstract class Argument
	{
		public static implicit operator Argument(string v) => new Input.CString(v);
		public static implicit operator Argument(CString v) => new Input.CString(v);

		public static implicit operator Argument(in Vector3 v) => new Input.Vector3(v);
		public static implicit operator Argument(in Vector4 v) => new Input.Vector4(v);
		public static implicit operator Argument(in Quaternion v) => new Input.Vector4(v);

		public static implicit operator Argument(bool v) => new Input.Primitive(N64.Val(v));
		public static implicit operator Argument(Enum v) => new Input.Primitive(Convert.ToUInt64(v));

		public static implicit operator Argument(byte v) => new Input.Primitive(N64.Val(v));
		public static implicit operator Argument(ushort v) => new Input.Primitive(N64.Val(v));
		public static implicit operator Argument(uint v) => new Input.Primitive(N64.Val(v));
		public static implicit operator Argument(ulong v) => new Input.Primitive(N64.Val(v));

		public static implicit operator Argument(sbyte v) => new Input.Primitive(N64.Val(v));
		public static implicit operator Argument(short v) => new Input.Primitive(N64.Val(v));
		public static implicit operator Argument(int v) => new Input.Primitive(N64.Val(v));
		public static implicit operator Argument(long v) => new Input.Primitive(N64.Val(v));

		public static implicit operator Argument(float v) => new Input.Primitive(N64.Val(v));
		public static implicit operator Argument(double v) => new Input.Primitive(N64.Val(v));
		
		[SecurityCritical] public static unsafe implicit operator Argument(void* v) => new Input.Primitive((ulong)v);

		[SecurityCritical] internal abstract unsafe void PushNativeValue(ref CustomNativeInvoker.CustomInvocation ctx);
		[SecurityCritical] internal virtual unsafe void PullNativeValue(ref CustomNativeInvoker.CustomInvocation ctx)
			=> ctx.m_offset++;
	}

	namespace Input
	{
		public class Primitive : Argument
		{
			protected ulong m_nativeValue;

			// solely here for overriders
			public virtual ulong NativeValue { get => m_nativeValue; set => m_nativeValue = value; }

			public Primitive(ulong value) => this.m_nativeValue = value;
			public Primitive() { }

			[SecurityCritical]
			internal override unsafe void PushNativeValue(ref CustomNativeInvoker.CustomInvocation ctx)
			{
				ctx.m_ctx.functionDataPtr[ctx.m_ctx.numArguments++] = m_nativeValue;
			}
		}

		public sealed class CString : Argument
		{
			private CitizenFX.Core.CString m_cstr;

			internal CString(CitizenFX.Core.CString value) => m_cstr = value;

			[SecurityCritical]
			internal override unsafe void PushNativeValue(ref CustomNativeInvoker.CustomInvocation ctx)
			{
				fixed (byte* ptr = m_cstr?.value)
				{
					ctx.m_ctx.functionDataPtr[ctx.m_ctx.numArguments++] = (ulong)ptr;
					ctx.PushPinAndInvoke();
				}
			}
		}

		public sealed class Vector3 : Argument
		{
			private readonly unsafe ulong x;
			private readonly unsafe ulong y;
			private readonly unsafe ulong z;

			public Vector3(in Core.Vector3 v)
			{
				x = N64.Val(v.X);
				y = N64.Val(v.Y);
				z = N64.Val(v.Z);
			}

			[SecurityCritical]
			internal override unsafe void PushNativeValue(ref CustomNativeInvoker.CustomInvocation ctx)
			{
				ulong* data = ctx.m_ctx.functionDataPtr + ctx.m_ctx.numArguments;
				ctx.m_ctx.numArguments += 3;

				data[0] = x;
				data[1] = y;
				data[2] = z;
			}

			[SecurityCritical]
			internal override unsafe void PullNativeValue(ref CustomNativeInvoker.CustomInvocation ctx)
				=> ctx.m_offset += 3;
		}

		public sealed class Vector4 : Argument
		{
			private readonly unsafe ulong x;
			private readonly unsafe ulong y;
			private readonly unsafe ulong z;
			private readonly unsafe ulong w;

			public Vector4(in Core.Vector4 v)
			{
				x = N64.Val(v.X);
				y = N64.Val(v.Y);
				z = N64.Val(v.Z);
				w = N64.Val(v.W);
			}

			public Vector4(in Quaternion v)
			{
				x = N64.Val(v.X);
				y = N64.Val(v.Y);
				z = N64.Val(v.Z);
				w = N64.Val(v.W);
			}

			[SecurityCritical]
			internal override unsafe void PushNativeValue(ref CustomNativeInvoker.CustomInvocation ctx)
			{
				ulong* data = ctx.m_ctx.functionDataPtr + ctx.m_ctx.numArguments;
				ctx.m_ctx.numArguments += 4;

				data[0] = x;
				data[1] = y;
				data[2] = z;
				data[3] = w;
			}
		}
	}

	namespace Output
	{
		public class Primitive : Argument
		{
			protected ulong m_nativeValue;

			public Primitive() { }

			public static implicit operator bool(Primitive v) => N64.To_bool(v.m_nativeValue);

			public static implicit operator uint(Primitive v) => N64.To_uint(v.m_nativeValue);
			public static implicit operator ulong(Primitive v) => N64.To_ulong(v.m_nativeValue);

			public static implicit operator int(Primitive v) => N64.To_int(v.m_nativeValue);
			public static implicit operator long(Primitive v) => N64.To_long(v.m_nativeValue);

			public static implicit operator float(Primitive v) => N64.To_float(v.m_nativeValue);
			public static implicit operator double(Primitive v) => N64.To_double(v.m_nativeValue);

			[SecurityCritical]
			internal override unsafe void PushNativeValue(ref CustomNativeInvoker.CustomInvocation ctx)
			{
				ctx.m_ctx.functionDataPtr[ctx.m_ctx.numArguments++] = m_nativeValue;
			}
			
			[SecurityCritical]
			internal override unsafe void PullNativeValue(ref CustomNativeInvoker.CustomInvocation ctx)
			{
				m_nativeValue = ctx.m_ctx.functionDataPtr[ctx.m_offset++];
			}
		}

		public sealed class Vector4 : Argument
		{
			private Core.Vector4 m_vector;

			public Vector4() { }

			public static implicit operator Core.Vector2(Vector4 v) => (Core.Vector2)v.m_vector;
			public static implicit operator Core.Vector3(Vector4 v) => (Core.Vector3)v.m_vector;
			public static implicit operator Core.Vector4(Vector4 v) => v.m_vector;
			public static implicit operator Core.Quaternion(Vector4 v) => new Core.Quaternion(v.m_vector.X, v.m_vector.Y, v.m_vector.Z, v.m_vector.W);

			[SecurityCritical]
			internal override unsafe void PushNativeValue(ref CustomNativeInvoker.CustomInvocation ctx)
			{
				fixed (Core.Vector4* ptr = &m_vector)
				{
					ctx.m_ctx.functionDataPtr[ctx.m_ctx.numArguments++] = (ulong)ptr;
					ctx.PushPinAndInvoke();
				}
			}
		}
	}
}
