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
        [SecuritySafeCritical]
		
		public static bool Enabled { get; set; }
		
        public static void Write(string data)
        {
			if(Enabled != true) { return; }
			var channel = "script:" + ((InternalManager.GlobalManager?.ResourceName) ?? "mono");

            GameInterface.PrintLog(channel, data);
        }

        public static void Write(string format, params object[] args)
        {
			if(Enabled != true) { return; }
            Write(string.Format(format, args));
        }

        public static void WriteLine()
        {
			if(Enabled != true) { return; }
            Write("\n");
        }
				
        public static void WriteLine(string data)
        {
			if(Enabled != true) { return; }
            Write(data + "\n");
        }

        public static void WriteLine(string format, params object[] args)
        {
			if(Enabled != true) { return; }
            Write(string.Format(format, args) + "\n");
        }
    }
}
