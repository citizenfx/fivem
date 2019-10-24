using CitizenFX.Core;

using static CitizenFX.Core.Native.API;

namespace FxMonitor
{
    public class MonitorInit : BaseScript
    {
        public MonitorInit()
        {
            if (GetConvar("monitorMode", "false") == "true")
            {
                BaseScript.RegisterScript(new MonitorMain());
            }
            else
            {
                BaseScript.RegisterScript(new MonitorClient());
            }
        }
    }
}