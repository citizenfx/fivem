using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Security;
using System.Text;
using System.Threading;

namespace CitizenFX.Core
{
	[SecuritySafeCritical]
	internal static class ScriptManager
	{
		private static readonly List<BaseScript> s_activeScripts = new List<BaseScript>();

		private static readonly Dictionary<string, Assembly> s_loadedAssemblies = new Dictionary<string, Assembly>();

		private static readonly HashSet<string> s_assemblySearchPaths = new HashSet<string>();

		#region Initialization

		[SecuritySafeCritical]
		static ScriptManager()
		{
			//CultureInfo.DefaultThreadCurrentCulture = CultureInfo.InvariantCulture;
			//CultureInfo.DefaultThreadCurrentUICulture = CultureInfo.InvariantCulture;

			Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
			Thread.CurrentThread.CurrentUICulture = CultureInfo.InvariantCulture;

			AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;
			AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;
		}

		static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
		{
			Debug.WriteLine($"Unhandled exception: {e.ExceptionObject}");
		}
		#endregion

		#region Scripts
		internal static void AddScript(BaseScript script)
		{
			if (!s_activeScripts.Contains(script))
			{
				s_activeScripts.Add(script);

				script.Enable();
			}
		}

		internal static void RemoveScript(BaseScript script)
		{
			script.Disable();

			s_activeScripts.Remove(script);
		}

		private static void LoadScripts(Assembly assembly)
		{
			IEnumerable<Type> definedTypes;

			{
				// We have ClientScript and ServerScript defined only in the respective environments.
				// Handle type load exceptions and keep going.
				// See https://stackoverflow.com/a/11915414

				Func<Type, bool> typesPredicate = t =>
					t != null
					&& !t.IsAbstract
					&& t.IsSubclassOf(typeof(BaseScript))
					&& t.GetConstructor(Type.EmptyTypes) != null
					&& !(t.GetCustomAttribute<EnableOnLoadAttribute>()?.Enable == false);

				/*foreach (var type in assembly.GetTypes())
				{
					DevDebug.WriteLine(type.BaseType.BaseType.Assembly.Location.ToString());
				}*/

				try
				{
					definedTypes = assembly.GetTypes().Where(typesPredicate);
				}
				catch (ReflectionTypeLoadException e)
				{
					definedTypes = e.Types.Where(typesPredicate);
				}
			}

			foreach (var type in definedTypes)
			{
				try
				{
					var derivedScript = Activator.CreateInstance(type) as BaseScript;

					Debug.WriteLine("Instantiated instance of script {0}.", type.FullName);

					AddScript(derivedScript);
				}
				catch (Exception e)
				{
					Debug.WriteLine("Failed to instantiate instance of script {0}: {1}", type.FullName, e.ToString());
				}
			}
		}
		#endregion

		#region Assembly loading
		static Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
		{
			return LoadAssembly(args.Name.Split(',')[0], true);
		}

		public static Assembly LoadAssembly(string name, bool useSearchPaths = false)
		{
			if (name.EndsWith(".dll"))
				name = name.Substring(0, name.Length - 4); // remove last ".dll"

			Assembly assembly;
			var exceptions = new StringBuilder();

			if ((assembly = LoadAssemblyInternal(name, exceptions)) != null)
			{
				return assembly;
			}

			if (useSearchPaths)
			{
				foreach (var path in s_assemblySearchPaths)
				{
					if ((assembly = LoadAssemblyInternal($"{path}/{name}", exceptions)) != null)
					{
						return assembly;
					}
				}
			}

			if (!name.Contains(".resources"))
			{
				Debug.WriteLine($"Could not load assembly {name} - loading exceptions: {exceptions}");
			}

			return null;
		}

		[SecuritySafeCritical]
		private static Assembly LoadAssemblyInternal(string name, StringBuilder exceptions)
		{
			Assembly assembly;

			if (s_loadedAssemblies.TryGetValue(name, out assembly))
			{
#if !IS_FXSERVER
				Debug.WriteLine($"Returning previously loaded assembly {s_loadedAssemblies[name].FullName}");
#endif
				return assembly;
			}

			try
			{
				if (ScriptInterface.ReadAssembly(name, out byte[] assemblyData, out byte[] symbolData))
				{
					var dirName = Path.GetDirectoryName(name);
					if (!string.IsNullOrWhiteSpace(dirName))
					{
						s_assemblySearchPaths.Add(dirName.Replace('\\', '/'));
					}

					assembly = Assembly.Load(assemblyData, symbolData);

#if !IS_FXSERVER
					Debug.WriteLine($"Loaded {assembly.FullName} into {AppDomain.CurrentDomain.FriendlyName}");
#endif

					s_loadedAssemblies[name] = assembly;
					LoadScripts(assembly);

					return assembly;
				}
			}
			catch (FileNotFoundException e)
			{
				if (name.StartsWith("CitizenFX.") && name.EndsWith(".Natives"))
				{
					// Some notes:
					//  * We wouldn't have been here if the UGC assembly was asking for the latest version, see MonoComponentHost::AssemblyResolve in citizen-scripting-mono-v2
					//  * Tried to load the UGC supplied CitizenFX.*.Natives.dll, which is 404, so load the local version instead
					//  * The latest CitizenFX.*.Natives.dll can be requested by asking for version: 2, 65535, 65535, 65535.

					AssemblyName coreVersion = new AssemblyName(name) { Version = new Version(2, 65535, 65535, 65535) };
					Debug.WriteLine($"Couldn't load UGC supplied {name} version, this can result into a MissingMethodException. Loading local version instead.");

					return AppDomain.CurrentDomain.Load(coreVersion);
				}

				//Switching the FileNotFound to a NotImplemented tells mono to disable I18N support.
				//See: https://github.com/mono/mono/blob/8fee89e530eb3636325306c66603ba826319e8c5/mcs/class/corlib/System.Text/EncodingHelper.cs#L131
				if (string.Equals(name, "I18N", StringComparison.OrdinalIgnoreCase))
					throw new NotImplementedException("I18N not found", e);
			}
			catch (Exception e)
			{
				exceptions.AppendLine($"Exception loading assembly {name}.dll: {e}");
			}

			return null;
		}
		#endregion
	}
}
