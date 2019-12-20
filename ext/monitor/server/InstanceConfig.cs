using System.Collections.Generic;

namespace FxMonitor
{
    public class InstanceConfig
    {
        public string Name { get; set; }
        public List<string> ConfigFiles { get; set; }
        public Dictionary<string, string> ConVars { get; set; }
        public List<string> Commands { get; set; }
    }
}