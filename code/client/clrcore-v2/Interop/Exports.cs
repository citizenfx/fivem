using System;
using System.ComponentModel;
using System.Collections.Generic;

namespace CitizenFX.Core
{
	public class Exports
	{
		private Dictionary<string, DynFunc> m_exports => new Dictionary<string, DynFunc>();

		public static LocalExports Local { get; } = new LocalExports();

#if REMOTE_FUNCTION_ENABLED
#if !IS_FXSERVER
		public static RemoteExports Remote { get; } = new RemoteExports();
		public static RemoteExports Server { get; } = new RemoteExports();
#else
		public static RemoteExports Remote { get; } = new RemoteExports();
		public static RemoteExports Client { get; } = new RemoteExports();

		public ExportFunc this[Player player, string resource, string export]
		{
			get
			{
				CString fullExportName = ExportsManager.CreateFullExportName(resource, export);
				return (args) => ExportManager.RemoteInvoke(fullExportName, player, args);
			}
		}
#endif
#endif

		~Exports()
		{
			foreach(var export in m_exports.Keys)
				ExportsManager.RemoveExportHandler(export);
		}

		public ExportFunc this[string resource, string export]
		{
			get
			{
				CString fullExportName = ExportsManager.CreateFullExportName(resource, export);
				return (args) => ExportsManager.LocalInvoke(fullExportName, args);
			}
		}

		public DynFunc this[string export]
		{
			set => Add(export, value);
			get => m_exports[export];
		}

#if REMOTE_FUNCTION_ENABLED
		public DynFunc this[string export, Binding binding = Binding.LOCAL]
		{
			set => Add(export, value, binding);
		}
#endif

		public void Add(string name, DynFunc method, Binding binding = Binding.Local)
		{
			if (ExportsManager.AddExportHandler(name, method, binding))
				m_exports.Add(name, method);
		}

		public void Remove(string name)
		{
			if (m_exports.Remove(name))
				ExportsManager.RemoveExportHandler(name);
		}
	}

	public class ResourceExports
	{
		public string resourcePrefix;

		internal ResourceExports(string name)
		{
			resourcePrefix = ExportsManager.CreateExportPrefix(name);
		}

		public ExportFunc this[string exportName]
		{
			get
			{
				CString fullExportName = resourcePrefix + exportName;
				return (args) => ExportsManager.LocalInvoke(fullExportName, args);
			}
		}
	}

	[Browsable(false)]
	[EditorBrowsable(EditorBrowsableState.Never)]
	public struct LocalExports
	{
		public ExportFunc this[string resource, string export]
		{
			get
			{
				CString fullExportName = ExportsManager.CreateFullExportName(resource, export);
				return (args) => ExportsManager.LocalInvoke(fullExportName, args);
			}
		}
	}

#if REMOTE_FUNCTION_ENABLED
	[Browsable(false)]
	[EditorBrowsable(EditorBrowsableState.Never)]
	public struct RemoteExports
	{
#if IS_FXSERVER
		public Callback this[Player player, string resource, string export]
		{
			get
			{
				CString fullExportName = ExportsManager.CreateExportName(resource, export);
				return (args) => ExportManager.RemoteInvoke(fullExportName, export, args);
			}
		}
#else
		public Callback this[string resource, string export]
		{
			get
			{
				CString fullExportName = ExportsManager.CreateExportName(resource, export);
				return (args) => ExportManager.RemoteInvoke(fullExportName, args);
			}
		}
#endif
	}
#endif
}
