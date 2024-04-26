using System;
using System.ComponentModel;

namespace CitizenFX.Core
{
	[Flags]
	public enum Binding
	{
		/// <summary>
		/// No one can call this
		/// </summary>
		None = 0x0,

		/// <summary>
		/// Server only accepts server calls, client only client calls
		/// </summary>
		Local = 0x1,

		/// <summary>
		/// Server only accepts client calls, client only server calls
		/// </summary>
		Remote = 0x2,

		/// <summary>
		/// Accept all incoming calls
		/// </summary>
		All = Local | Remote
	}

	/// <summary>
	/// Schedule this method to run on the first frame when this <see cref="BaseScript"/> is loaded
	/// </summary>
	/// <remarks>Only works on <see cref="BaseScript"/> inherited class methods</remarks>
	[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
	public class TickAttribute : Attribute
	{
		public bool StopOnException { get; set; } = false;
		public TickAttribute() { }
	}

	/// <summary>
	/// Register this method to listen for the given <see cref="Command"/> when this <see cref="BaseScript"/> is loaded
	/// </summary>
	/// <remarks>Only works on <see cref="BaseScript"/> inherited class methods</remarks>
	[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
	public class CommandAttribute : Attribute
	{
		public string Command { get; }
		public bool Restricted { get; set; }

		/// <returns></returns>
		/// <inheritdoc cref="Func.ConstructCommandRemapped(object, System.Reflection.MethodInfo)"/>
		public bool RemapParameters { get; set; } = false;
		public CommandAttribute(string command, bool restricted = false)
		{
			Command = command;
			Restricted = restricted;
		}
	}

// TODO: revert this commit (blame check) when RedM has KeyMapping support, also don't make changes to this comment.
#if GTA_FIVE
	/// <summary>
	/// Register this method to listen for the given key <see cref="Command"/> when this <see cref="BaseScript"/> is loaded
	/// </summary>
	/// <remarks>This will bind the given input details to the command, triggering all commands registered as such.<br />Only works on <see cref="BaseScript"/> inherited class methods</remarks>
#else
	/// <summary>Does nothing on server side or RedM</summary>
	[EditorBrowsable(EditorBrowsableState.Never)]
#endif
	[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
	public class KeyMapAttribute : Attribute
	{
		public string Command { get; }
		public string Description { get; }
		public string InputMapper { get; }
		public string InputParameter { get; }

		/// <returns></returns>
		/// <inheritdoc cref="Func.ConstructCommandRemapped(object, System.Reflection.MethodInfo)"/>
		public bool RemapParameters { get; set; } = false;

		/// <inheritdoc cref="KeyMapAttribute"/>
		/// <param name="command">The command to execute, and the identifier of the binding</param>
		/// <param name="description">A description for in the settings menu</param>
		/// <param name="inputMapper">The mapper ID to use for the default binding, e.g. keyboard</param>
		/// <param name="inputParameter">The IO parameter ID to use for the default binding, e.g. f3</param>
		public KeyMapAttribute(string command, string description, string inputMapper, string inputParameter)
		{
			Command = command;
			Description = description;
			InputMapper = inputMapper;
			InputParameter = inputParameter;
		}

		/// <inheritdoc cref="KeyMapAttribute"/>
		/// <remarks>Does not register the key mapping, so it works the same as <see cref="Command"/></remarks>
		/// <param name="commandOnly">The command to execute, and the identifier of the binding</param>
		public KeyMapAttribute(string commandOnly)
		{
			Command = commandOnly;
		}
	}

	/// <summary>
	/// Register this method to listen for the given <see cref="Event"/> when this <see cref="BaseScript"/> is loaded
	/// </summary>
	/// <remarks>Only works on <see cref="BaseScript"/> inherited class methods</remarks>
	[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
	public class EventHandlerAttribute : Attribute
	{
		public string Event { get; }
		public Binding Binding { get; }
		public EventHandlerAttribute(string name, Binding binding = Binding.All)
		{
			Event = name;
			Binding = binding;
		}
	}
	
#if !IS_FXSERVER
	/// <summary>
	/// Register this method to listen for the given <see cref="CallbackName"/> when this <see cref="BaseScript"/> is loaded
	/// if <see cref="IsRawCallback"/> is specified this will use a raw NUI callback instead
	/// </summary>
	/// <remarks>Only works on <see cref="BaseScript"/> inherited class methods</remarks>
#else
	/// <summary>Does nothing on server side</summary>
	[EditorBrowsable(EditorBrowsableState.Never)]
#endif
	[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
	public class NuiCallbackAttribute : Attribute
	{
		public string CallbackName { get; }
		public bool IsRawCallback { get; }
		public NuiCallbackAttribute(string callbackName, bool isRawCallback = false)
		{
			CallbackName = callbackName;
			if (isRawCallback)
			{
				throw new NotImplementedException("Raw Nui Callbacks are not currently implemented");
			}
			IsRawCallback = isRawCallback;
		}
	}

	/// <summary>
	/// Register this method to listen for the given <see cref="Export"/> when this <see cref="BaseScript"/> is loaded
	/// </summary>
	/// <remarks>Only works on <see cref="BaseScript"/> inherited class methods</remarks>
	[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
	public class ExportAttribute : Attribute
	{
		public string Export { get; }
		public Binding Binding { get; }
		public ExportAttribute(string name, Binding binding = Binding.Local)
		{ 
			Export = name;
			Binding = binding;
		}
	}


	/// <summary>
	/// When used in events it'll be filled with the caller (source) of this event
	/// </summary>
	/// <example>
	///		Shared libraries
	///		<code>[Source] Remote remote</code>
	///	</example>
	/// <example>
	///		Server libraries
	///		<code>[Source] Player player</code>
	/// </example>
	/// <example>
	///		Shared libraries
	///		<code>[Source] bool isRemote</code>
	/// </example>
	[AttributeUsage(AttributeTargets.Parameter)]
	public class SourceAttribute : Attribute
	{ }

	/// <summary>
	/// Explicitly enable or disable <see cref="BaseScript" /> instantiation when this resource is starting.
	/// </summary>
	[AttributeUsage(AttributeTargets.Class)]
	public class EnableOnLoadAttribute : Attribute
	{
		public bool Enable { get; }
		public EnableOnLoadAttribute(bool enable)
		{
			Enable = enable;
		}
	}
}
