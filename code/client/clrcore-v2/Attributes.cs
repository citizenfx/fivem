using System;

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
		public CommandAttribute(string command, bool restricted = false)
		{
			Command = command;
			Restricted = restricted;
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

	[AttributeUsage(AttributeTargets.Parameter)]
	public class SourceAttribute : Attribute
	{ }
}
