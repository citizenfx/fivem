using CitizenFX.Core;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;

using static CitizenFX.Core.Native.API;

namespace FxMonitor
{
    public class MonitorMain : BaseScript
    {
        private string m_rootPath;
        private List<Instance> m_instances = new List<Instance>();

        internal MonitorMain()
        {
            m_rootPath = GetConvar("serverRoot", "");
        }

        [Tick]
        public async Task MonitorMain_Tick()
        {
            foreach (var instance in m_instances.ToArray())
            {
                await instance.Update();
            }
        }

        [Command("add_instance")]
        public void AddInstance(string[] args)
        {
            if (args.Length < 1)
            {
                Debug.WriteLine("? add_instance [config name]");
                return;
            }

            try
            {
                var cfg = File.ReadAllText(Path.Combine(m_rootPath, args[0] + ".json"));
                var instanceConfig = JsonConvert.DeserializeObject<InstanceConfig>(cfg);
                var instance = new Instance(instanceConfig);

                m_instances.Add(instance);

                Debug.WriteLine($"^1+^7 {instance.Name}");
            }
            catch (Exception)
            {

            }
        }

        [Command("remove_instance")]
        public async void RemoveInstance(string[] args)
        {
            if (args.Length < 1)
            {
                Debug.WriteLine("? remove_instance [config name]");
                return;
            }

            try
            {
                var instance = m_instances.Find(a => a.Name == args[0]);
                await instance.Stop();
                m_instances.Remove(instance);

                Debug.WriteLine($"^3-^7 {instance.Name}");
            }
            catch (Exception ex)
            {

            }
        }
    }
}