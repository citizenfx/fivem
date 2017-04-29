using System;
using System.Diagnostics;
using System.Linq;
using System.Security;

using System.Reflection;
using System.Runtime.InteropServices;

using CitizenFX.Core;

namespace CitizenFX.Core
{
    internal static class RuntimeManager
    {
        public static void Initialize()
        {
            Debug.WriteLine("Hello!");
        }

        public static Guid[] GetImplementedClasses(Guid ifImpl)
        {
            return Assembly.GetExecutingAssembly().GetTypes()
                .Where(a => a.IsClass)
                .Where(a => a.GetInterfaces()
                    .Select(b => b.GetCustomAttribute<GuidAttribute>()?.Value)
                    .Any(b => (b != null && ifImpl == Guid.Parse(b)))
                )
                .Select(a => Guid.Parse(a.GetCustomAttribute<GuidAttribute>().Value))
                .ToArray();
        }

        [SecuritySafeCritical]
        public static IntPtr CreateObjectInstance(Guid guid, Guid iid)
        {
            try
            {
                var type = Assembly.GetExecutingAssembly().GetTypes()
                                    .Where(a => a.GetCustomAttribute<GuidAttribute>() != null)
                                    .First(
                                           a => Guid.Parse(a.GetCustomAttribute<GuidAttribute>().Value) == guid);

                return Marshal.GetComInterfaceForObject(
                    Activator.CreateInstance(type),
                    Assembly.GetExecutingAssembly().GetTypes()
                        .Where(a => a.GetCustomAttribute<GuidAttribute>() != null)
                        .First(a => a.IsInterface && Guid.Parse(a.GetCustomAttribute<GuidAttribute>().Value) == iid));
            }
            catch (Exception e)
            {
                Debug.WriteLine($"Failed to get instance for guid {guid} and iid {iid}: {e}");

                throw;
            }
        }
    }
}