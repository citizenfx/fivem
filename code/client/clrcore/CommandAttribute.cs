using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
	[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
	public class CommandAttribute : Attribute
	{
		public string Command;
		public bool Restricted = false;

		public CommandAttribute(string command, bool restricted = false)
		{
			Command = command;
			Restricted = restricted;
		}
	}
}
