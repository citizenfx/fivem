using System;

namespace CitizenFX.Core
{
	[Flags]
	public enum Binding { NONE = 0, LOCAL = 1, REMOTE = 2, ALL = LOCAL | REMOTE }

	[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
	public class TickAttribute : Attribute
	{
		public TickAttribute() { }
	}

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

	[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
	public class EventHandlerAttribute : Attribute
	{
		public string Event { get; }
		public Binding Binding { get; }
		public EventHandlerAttribute(string name, Binding binding = Binding.ALL)
		{
			Event = name;
			Binding = binding;
		}
	}

	[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
	public class ExportAttribute : Attribute
	{
		public string Export { get; }
		public Binding Binding { get; }
		public ExportAttribute(string name, Binding binding = Binding.LOCAL)
		{ 
			Export = name;
			Binding = binding;
		}
	}

#if IS_FXSERVER
	[AttributeUsage(AttributeTargets.Parameter, Inherited = false, AllowMultiple = false)]
	public sealed class FromSourceAttribute : Attribute
	{
	}
#endif
}
