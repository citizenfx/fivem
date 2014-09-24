using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public static class Function
    {
        public static void Call(uint nativeHash, params Parameter[] args)
        {
            Invoke(nativeHash, args);
        }

        public static T Call<T>(uint nativeHash, params Parameter[] args)
        {
            return Invoke<T>(nativeHash, args);
        }

        public static void Invoke(uint nativeHash, params Parameter[] args)
        {
            InvokeInternal(nativeHash, typeof(void), args);
        }

        public static T Invoke<T>(uint nativeHash, params Parameter[] args)
        {
            return (T)InvokeInternal(nativeHash, typeof(T), args);
        }

        private static List<IntPtr> ms_toFreeStringList = new List<IntPtr>(16);

        private static object InvokeInternal(uint nativeHash, Type returnType, Parameter[] args)
        {
            GameInterface.NativeCallArguments context = new GameInterface.NativeCallArguments();
            context.Initialize();

            context.nativeHash = nativeHash;
            context.numArguments = 0;

            // push arguments into the context
            foreach (var arg in args)
            {
                int i = context.numArguments;
                context.argumentFlags[i] = 0;

                switch (arg.Type)
                {
                    case Parameter.ParameterType.String:
                        context.intArguments[i] = GetStringPointer((string)arg.Value);
                        context.numArguments++;
                        break;
                    case Parameter.ParameterType.Float:
                        context.floatArguments[i] = (float)arg.Value;
                        context.argumentFlags[i] |= 0x80;
                        context.numArguments++;
                        break;
                    case Parameter.ParameterType.FloatPointer:
                        {
                            var ptr = (Pointer)arg.Value;
                            context.floatArguments[i] = (float)(ptr.Value ?? 0f);
                            context.argumentFlags[i] |= 0x80 | 0x40;
                            context.numArguments++;
                            break;
                        }
                    case Parameter.ParameterType.Integer:
                        context.intArguments[i] = (int)arg.Value;
                        context.numArguments++;
                        break;
                    case Parameter.ParameterType.IntPointer:
                        {
                            var ptr = (Pointer)arg.Value;
                            context.intArguments[i] = (int)(ptr.Value ?? 0);
                            context.argumentFlags[i] |= 0x40;
                            context.numArguments++;
                            break;
                        }
                    case Parameter.ParameterType.None:
                        throw new ArgumentException("None is not a valid parameter type.");
                }
            }

            Debug.WriteLine();

            // invoke the native
            if (!InvokeContext(ref context))
            {
                FreeStringPointers();

                throw new SystemException(string.Format("Execution of native hash {0} failed.", nativeHash));
            }

            FreeStringPointers();

            // get out any pointer values we may have passed
            {
                var i = 0;

                foreach (var arg in args)
                {
                    switch (arg.Type)
                    {
                        case Parameter.ParameterType.IntPointer:
                            {
                                var ptr = (Pointer)arg.Value;
                                ptr.SetValue(context.intArguments[i]);

                                break;
                            }

                        case Parameter.ParameterType.FloatPointer:
                            {
                                var ptr = (Pointer)arg.Value;
                                ptr.SetValue(context.floatArguments[i]);

                                break;
                            }
                    }

                    i++;
                }
            }

            // and get out the result value
            if (returnType == typeof(int))
            {
                return (int)context.resultValue;
            }
            else if (returnType == typeof(bool))
            {
                return (context.resultValue != 1);
            }
            else if (returnType == typeof(float))
            {
                byte[] bytes = BitConverter.GetBytes(context.resultValue);
                return BitConverter.ToSingle(bytes, 0);
            }
            else if (returnType == typeof(string))
            {
                return MarshalOutString(ref context);
            }

            return null;
        }

        [SecuritySafeCritical]
        private static string MarshalOutString(ref GameInterface.NativeCallArguments context)
        {
            return Marshal.PtrToStringAnsi(new IntPtr(context.resultValue));
        }

        [SecuritySafeCritical]
        private static bool InvokeContext(ref GameInterface.NativeCallArguments arguments)
        {
            return GameInterface.InvokeGameNative(ref arguments);
        }

        [SecuritySafeCritical]
        private static int GetStringPointer(string value)
        {
            if (value == null)
            {
                value = string.Empty;
            }

            IntPtr str = Marshal.StringToHGlobalAnsi(value);

            ms_toFreeStringList.Add(str); // as we'll need to free this later

            return str.ToInt32();
        }

        [SecuritySafeCritical]
        private static void FreeStringPointers()
        {
            foreach (var str in ms_toFreeStringList)
            {
                Marshal.FreeHGlobal(str);
            }

            ms_toFreeStringList.Clear();
        }
    }
}
