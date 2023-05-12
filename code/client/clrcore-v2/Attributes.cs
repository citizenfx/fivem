using System;

namespace CitizenFX.Core
{
	[Flags]
	public enum Binding { None = 0x0, Local = 0x1, Remote = 0x2, All = Local | Remote }

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
		public EventHandlerAttribute(string name, Binding binding = Binding.All)
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
		public ExportAttribute(string name, Binding binding = Binding.Local)
		{ 
			Export = name;
			Binding = binding;
		}
	}
}
