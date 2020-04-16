using System;
using Microsoft.SymbolStore;

namespace CitizenFX.BuildTools.SymbolUpload
{
	internal sealed class Tracer : ITracer
	{
		public void WriteLine(string message)
		{
			Console.WriteLine(message);
		}

		public void WriteLine(string format, params object[] arguments)
		{
			Console.WriteLine(format, arguments);
		}

		public void Information(string message)
		{
			if (this.Enabled)
			{
				Console.WriteLine(message);
			}
		}

		public void Information(string format, params object[] arguments)
		{
			if (this.Enabled)
			{
				Console.WriteLine(format, arguments);
			}
		}

		public void Warning(string message)
		{
			if (this.Enabled)
			{
				Console.WriteLine("WARNING: " + message);
			}
		}

		public void Warning(string format, params object[] arguments)
		{
			if (this.Enabled)
			{
				Console.WriteLine("WARNING: " + format, arguments);
			}
		}

		public void Error(string message)
		{
			Console.WriteLine("ERROR: " + message);
		}

		public void Error(string format, params object[] arguments)
		{
			Console.WriteLine("ERROR: " + format, arguments);
		}

		public void Verbose(string message)
		{
			if (this.EnabledVerbose)
			{
				Console.WriteLine(message);
			}
		}

		public void Verbose(string format, params object[] arguments)
		{
			if (this.EnabledVerbose)
			{
				Console.WriteLine(format, arguments);
			}
		}

		public bool Enabled;

		public bool EnabledVerbose;
	}
}