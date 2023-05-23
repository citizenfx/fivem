#if IS_FXSERVER && !DYN_FUNC_NO_CALLI
#define DYN_FUNC_CALLI
#endif


using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Security;

namespace CitizenFX.Core
{
	public delegate object DynFunc(Remote remote, params object[] arguments);

	public static class Func
	{
		private static readonly Dictionary<MethodInfo, MethodInfo> s_wrappedMethods = new Dictionary<MethodInfo, MethodInfo>();
		private static readonly Dictionary<MethodInfo, MethodInfo> s_dynfuncMethods = new Dictionary<MethodInfo, MethodInfo>();

		/// <summary>
		/// Creates a new dynamic invokable function
		/// </summary>
		/// <param name="type">the type which should contain the method</param>
		/// <param name="method">method name to find and use in type's method list</param>
		/// <returns>dynamically invokable delegate</returns>
		/// <exception cref="ArgumentNullException">when given target or method is null</exception>
		/// <exception cref="ArgumentException">when the method could not be found in target's method list</exception>
		[SecurityCritical]
		public static DynFunc Create(Type type, string method)
		{
			if (type is null || method is null)
				throw new ArgumentNullException($"Given target type or method is null.");

			MethodInfo func = type.GetMethod(method, BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public);
			if (func is null)
				throw new ArgumentException($"Could not find method in target's method list.");

			return Construct(null, func);
		}

		[SecurityCritical]
		public static DynFunc Create<T>(string method) => Create(typeof(T), method);

		/// <summary>
		/// Creates a new dynamic invokable function
		/// </summary>
		/// <param name="target">the instance object</param>
		/// <param name="method">method name to find and use in target's method list</param>
		/// <returns>dynamically invokable delegate</returns>
		/// <exception cref="ArgumentNullException">when given target or method is null</exception>
		/// <exception cref="ArgumentException">when the method could not be found in target's method list</exception>
		[SecurityCritical]
		public static DynFunc Create(object target, string method)
		{
			if (target is null || method is null)
				throw new ArgumentNullException($"Given target or method is null.");

			MethodInfo func = target.GetType().GetMethod(method, BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public);
			if (func is null)
				throw new ArgumentException($"Could not find method in target's method list.");

			return Construct(target, func);
		}

		/// <summary>
		/// Creates a new dynamic invokable function
		/// </summary>
		/// <param name="target">the instance object, may be null for static methods</param>
		/// <param name="method">method to make dynamically invokable</param>
		/// <returns>dynamically invokable delegate</returns>
		/// <exception cref="ArgumentNullException">when the given method is non-static and target is null or if given method is null</exception>
		[SecurityCritical]
		public static DynFunc Create(object target, MethodInfo method)
		{
			if (method is null)
			{
				throw new ArgumentNullException($"Given method is null.");
			}
			else if (!method.IsStatic && target is null)
			{
				string args = string.Join<string>(", ", method.GetParameters().Select(p => p.ParameterType.Name));
				throw new ArgumentNullException($"Can't create delegate for {method.DeclaringType.FullName}.{method.Name}({args}), it's a non-static method and it's missing a target instance.");
			}

			return Construct(target, method);
		}


		/// <summary>
		/// Creates a dynamic invokable function or simply returns it
		/// </summary>
		/// <param name="deleg">delegate to make dynamically invokable</param>
		/// <returns>dynamically invokable delegate or returns <paramref name="deleg"/> if it's aleady of type <see cref="DynFunc"/></returns>
		/// <exception cref="ArgumentNullException">when the given method is non-static and target is null or if given method is null</exception>
		[SecuritySafeCritical]
		public static DynFunc Create(Delegate deleg) => deleg is DynFunc dynFunc ? dynFunc : Create(deleg.Target, deleg.Method);

		// no need to recreate it
		public static DynFunc Create(DynFunc deleg) => deleg;

		[SecurityCritical]
		private static DynFunc Construct(object target, MethodInfo method)
		{
			if (s_wrappedMethods.TryGetValue(method, out var existingMethod))
			{
				return (DynFunc)existingMethod.CreateDelegate(typeof(DynFunc), target);
			}

			// TODO: implement optional parameter(s) support with parameter.IsOptional and parameter.DefaultValue

			ParameterInfo[] parameters = method.GetParameters();
#if DYN_FUNC_CALLI
			Type[] parameterTypes = new Type[parameters.Length];
#endif
			bool hasThis = (method.CallingConvention & CallingConventions.HasThis) != 0;

			var lambda = new DynamicMethod($"{method.DeclaringType.FullName}.{method.Name}", typeof(object),
				hasThis ? new[] { typeof(object), typeof(Remote), typeof(object[]) } : new[] { typeof(Remote), typeof(object[]) });

			ILGenerator g = lambda.GetILGenerator();

			OpCode ldarg_remote, ldarg_args;
			if (hasThis)
			{
				g.Emit(OpCodes.Ldarg_0);
				ldarg_remote = OpCodes.Ldarg_1;
				ldarg_args = OpCodes.Ldarg_2;
			}
			else
			{
				target = null;
				ldarg_remote = OpCodes.Ldarg_0;
				ldarg_args = OpCodes.Ldarg_1;
			}

			for (int i = 0, p = 0; i < parameters.Length; ++i)
			{
				var parameter = parameters[i];
				var t = parameter.ParameterType;

#if DYN_FUNC_CALLI
				parameterTypes[i] = t;
#endif
				if (Attribute.IsDefined(parameter, typeof(SourceAttribute), true))
				{
					if (t == typeof(Remote))
					{
						g.Emit(ldarg_remote);
						continue;
					}
					else if (t == typeof(bool))
					{
						g.Emit(ldarg_remote);
						g.Emit(OpCodes.Call, ((Func<Remote, bool>)Remote.IsRemoteInternal).Method);
						continue;
					}
					else
					{
						var constructor = t.GetConstructor(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic, null, new[] { typeof(Remote) }, null);
						if (constructor != null)
						{
							g.Emit(ldarg_remote);
							g.Emit(OpCodes.Newobj, constructor);
							continue;
						}
					}
					
					throw new ArgumentException($"{nameof(SourceAttribute)} used on type {t}, this type can't be constructed with parameter Remote.");
				}

				g.Emit(ldarg_args);
				g.Emit(OpCodes.Ldc_I4_S, (byte)p++);
				g.Emit(OpCodes.Ldelem_Ref);

				if (t.IsValueType)
				{
					Label diff = g.DefineLabel();
					Label done = g.DefineLabel();

					g.Emit(OpCodes.Dup);

					// check if given type is already of the target type
					g.Emit(OpCodes.Isinst, t);
					g.Emit(OpCodes.Brfalse_S, diff);

					// same type, unbox and pass it as an argument
					g.Emit(OpCodes.Unbox_Any, t);
					g.Emit(OpCodes.Br, done);

					// not the same type, try and convert it
					g.MarkLabel(diff);
					if (t.IsPrimitive)
					{
						g.Emit(OpCodes.Call, convertMethods[t]); // already handles null
					}
					else
					{
						g.Emit(OpCodes.Pop);
						g.Emit(OpCodes.Ldloc_S, g.DeclareLocal(t));
					}

					g.MarkLabel(done);
				}
				else if (t != typeof(object))
				{
					g.Emit(OpCodes.Castclass, t); // throwing cast, exception on fail
					//g.Emit(OpCodes.Isinst, t);    // non throwing cast, null on fail
				}
			}

#if DYN_FUNC_CALLI
			g.Emit(OpCodes.Ldc_I8, (long)method.MethodHandle.GetFunctionPointer());
			g.EmitCalli(OpCodes.Calli, method.CallingConvention, method.ReturnType, parameterTypes, null);
#else
			g.EmitCall(OpCodes.Call, method, null);
#endif

			if (method.ReturnType == typeof(void))
				g.Emit(OpCodes.Ldnull);
			else
				g.Emit(OpCodes.Box, method.ReturnType);

			g.Emit(OpCodes.Ret);

			Delegate dynFunc = lambda.CreateDelegate(typeof(DynFunc), target);

			s_wrappedMethods.Add(method, dynFunc.Method);
			s_dynfuncMethods.Add(dynFunc.Method, method);

			return (DynFunc)dynFunc;
		}

		#region Func<,> creators, C# why?!
		[SecuritySafeCritical] public static DynFunc Create<Ret>(Func<Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, Ret>(Func<A, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, Ret>(Func<A, B, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, Ret>(Func<A, B, C, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, Ret>(Func<A, B, C, D, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, Ret>(Func<A, B, C, D, E, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, Ret>(Func<A, B, C, D, E, F, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, Ret>(Func<A, B, C, D, E, F, G, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, Ret>(Func<A, B, C, D, E, F, G, H, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, Ret>(Func<A, B, C, D, E, F, G, H, I, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, Ret>(Func<A, B, C, D, E, F, G, H, I, J, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, K, Ret>(Func<A, B, C, D, E, F, G, H, I, J, K, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, K, L, Ret>(Func<A, B, C, D, E, F, G, H, I, J, K, L, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, K, L, M, Ret>(Func<A, B, C, D, E, F, G, H, I, J, K, L, M, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, K, L, M, N, Ret>(Func<A, B, C, D, E, F, G, H, I, J, K, L, M, N, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, Ret>(Func<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, Ret> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Ret>(Func<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Ret> method) => Create(method.Target, method.Method);
		#endregion

		#region Action<> creators, C# again, why?!
		[SecuritySafeCritical] public static DynFunc Create(Action method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A>(Action<A> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B>(Action<A, B> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C>(Action<A, B, C> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D>(Action<A, B, C, D> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E>(Action<A, B, C, D, E> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F>(Action<A, B, C, D, E, F> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G>(Action<A, B, C, D, E, F, G> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H>(Action<A, B, C, D, E, F, G, H> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I>(Action<A, B, C, D, E, F, G, H, I> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J>(Action<A, B, C, D, E, F, G, H, I, J> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, K>(Action<A, B, C, D, E, F, G, H, I, J, K> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, K, L>(Action<A, B, C, D, E, F, G, H, I, J, K, L> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, K, L, M>(Action<A, B, C, D, E, F, G, H, I, J, K, L, M> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, K, L, M, N>(Action<A, B, C, D, E, F, G, H, I, J, K, L, M, N> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>(Action<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O> method) => Create(method.Target, method.Method);
		[SecuritySafeCritical] public static DynFunc Create<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>(Action<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> method) => Create(method.Target, method.Method);
		#endregion

		#region Casting and Conversion

		private static MethodInfo GetMethodInfo<T, TResult>(Func<T, TResult> fun) => fun.Method;

		private static readonly Dictionary<Type, MethodInfo> convertMethods = new Dictionary<Type, MethodInfo>()
		{
			[typeof(bool)] = GetMethodInfo<object, bool>(Convert.ToBoolean),
			[typeof(char)] = GetMethodInfo<object, char>(Convert.ToChar),
			[typeof(sbyte)] = GetMethodInfo<object, sbyte>(Convert.ToSByte),
			[typeof(byte)] = GetMethodInfo<object, byte>(Convert.ToByte),
			[typeof(short)] = GetMethodInfo<object, short>(Convert.ToInt16),
			[typeof(ushort)] = GetMethodInfo<object, ushort>(Convert.ToUInt16),
			[typeof(int)] = GetMethodInfo<object, int>(Convert.ToInt32),
			[typeof(uint)] = GetMethodInfo<object, uint>(Convert.ToUInt32),
			[typeof(long)] = GetMethodInfo<object, long>(Convert.ToInt64),
			[typeof(ulong)] = GetMethodInfo<object, ulong>(Convert.ToUInt64),
			[typeof(float)] = GetMethodInfo<object, float>(Convert.ToSingle),
			[typeof(double)] = GetMethodInfo<object, double>(Convert.ToDouble),
			[typeof(decimal)] = GetMethodInfo<object, decimal>(Convert.ToDecimal)
		};

		#endregion

		/// <summary>
		/// Returns the original method that's been wrapped or its own <see cref="Delegate.Method"/>
		/// </summary>
		/// <param name="dynFunc">this reference</param>
		/// <returns>The wrapped method or <see cref="Delegate.Method"/> if it wasn't wrapped</returns>
		[EditorBrowsable(EditorBrowsableState.Never)]
		public static MethodInfo GetWrappedMethod(this DynFunc dynFunc)
		{
			return s_dynfuncMethods.TryGetValue(dynFunc.Method, out var existingMethod) ? existingMethod : dynFunc.Method;
		}
	}
}
