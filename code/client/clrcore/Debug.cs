using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Security;

namespace CitizenFX.Core
{
    public static class Debug
    {
		public const string LIGHT_RED = "^1";
		public const string LIGHT_GREEN = "^2";
		public const string YELLOW = "^3";
		public const string DARK_BLUE = "^4";
		public const string LIGHT_BLUE = "^5";
		public const string PURPLE = "^6";
		public const string WHITE = "^7";
		public const string DARK_RED = "^8";
		public const string PINK = "^9";

		[SecuritySafeCritical]
        public static void Write(string data)
        {
			var channel = "script:" + ((InternalManager.GlobalManager?.ResourceName) ?? "mono");

            GameInterface.PrintLog(channel, data);
        }

        public static void Write(string format, params object[] args)
        {
            Write(string.Format(format, args));
        }

        public static void WriteLine()
        {
            Write("\n");
        }
				
        public static void WriteLine(string data)
        {
            Write(data + "\n");
        }

        public static void WriteLine(string format, params object[] args)
        {
            Write(string.Format(format, args) + "\n");
        }
    }
}
