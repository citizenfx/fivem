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
