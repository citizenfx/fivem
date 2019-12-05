using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Threading.Tasks;
using CitizenFX.Core;
using Debug = CitizenFX.Core.Debug;

using static CitizenFX.Core.Native.API;
using System.Linq;
using ProtoBuf;
using nng;

namespace FxMonitor
{
    public class Instance
    {
        private InstanceConfig m_instanceConfig;

        private Process m_process;
        private int? m_port;
        private string m_nucleusUrl;

        public string Name => m_instanceConfig.Name;

        private static string ms_rootPath;

        private nng.IPairSocket m_pair;
        private bool m_hadPipe;

        static Instance()
        {
            ms_rootPath = GetConvar("serverRoot", "");
        }

        public Instance(InstanceConfig instanceConfig)
        {
            this.m_instanceConfig = instanceConfig;
        }

        public async Task Update()
        {
            if (m_process == null)
            {
                await Start();
            }
            else
            {
                await Monitor();
            }
        }

        public async Task Start()
        {
            // does the pid already exist?
            var exists = false;

            try
            {
                var pidString = File.ReadAllText(Path.Combine(ms_rootPath, "cache", $"{Name}.pid"));
                var pid = int.Parse(pidString);

                var proc = Process.GetProcessById(pid);

                if (proc.ProcessName.Contains("FXServer") || proc.ProcessName.Contains("ld-musl-x86_64.so.1"))
                {
                    exists = true;
                    m_process = proc;
                }
            }
            catch {}

            if (!exists)
            {
                await StartNew();
            }

            StartIPC();
        }

        private void StartIPC()
        {
            Task.Run(async () =>
            {
                // TODO: cleaner way?
                var nngFactory = new nng.Tests.TestFactory();

                while (!m_hadPipe)
                {
                    try
                    {
                        var pid = m_process.Id;

                        // c# isnt f# or rust
                        // why is this api trying to be both at once
                        var pairResult = nngFactory
                            .PairOpen()
                            .ThenListen($"ipc:///tmp/fxs_instance_{pid}");

                        if (pairResult.TryError(out var error))
                        {
                            await Task.Delay(500);
                            continue;
                        }

                        m_pair = pairResult.Unwrap();

                        m_hadPipe = true;

                        while (m_pair != null && m_process != null && pid == m_process.Id)
                        {
                            // he tried, ok
                            if (m_pair.RecvMsg().TryOk(out var message))
                            {
                                var cmd = Serializer.Deserialize<BaseCommand>(new MemoryStream(message.AsSpan().ToArray()));

                                if (cmd != null)
                                {
                                    await HandleCommand(cmd);
                                }
                            }
                        }
                    }
                    catch (Exception e) { Debug.WriteLine(e.ToString()); }
                }
            });
        }

        private Task HandleCommand(BaseCommand command)
        {
            switch (command.Type)
            {
                case 2:
                {
                    var cd = command.DeserializeAs<GetPortResponseCommand>();
                    m_port = cd.Port;

                    string urlBit = "";

                    if (!string.IsNullOrWhiteSpace(cd.NucleusUrl))
                    {
                        m_nucleusUrl = cd.NucleusUrl;
                        urlBit = $", ^4{cd.NucleusUrl}^7";
                    }

                    m_nucleusUrl = cd.NucleusUrl;
                    Debug.WriteLine($"^2>^7 {Name} (:{m_port}{urlBit})");
                    break;
                }
                case 3:
                    m_nucleusUrl = command.DeserializeAs<NucleusConnectedCommand>().Url;
                    Debug.WriteLine($"^2:^7 {Name} -> ^4{m_nucleusUrl}^7");
                    break;
            }

            return Task.CompletedTask;
        }

        private async Task StartNew()
        {
            var psi = new ProcessStartInfo();

            var arguments = new List<string>();

            if (Environment.OSVersion.Platform == PlatformID.Win32NT)
            {
                psi.FileName = Path.Combine(GetConvar("citizen_root", ""), "FXServer.exe");
            }
            else
            {
                var r = GetConvar("citizen_root", "");
                var lr = Path.GetDirectoryName(Path.GetDirectoryName(Path.GetDirectoryName(Path.GetDirectoryName(r))));

                psi.FileName = Path.Combine(r, "ld-musl-x86_64.so.1");
                arguments.Add("--library-path");
                arguments.Add($"{lr}/alpine/usr/lib/v8/:{lr}/alpine/lib/:{lr}/alpine/usr/lib/");
                arguments.Add("--");
                arguments.Add(Path.Combine(r, "FXServer"));

                arguments.Add("+start");
                arguments.Add("monitor");
            }

            foreach (var config in m_instanceConfig.ConfigFiles)
            {
                arguments.Add("+exec");
                arguments.Add(config);
            }

            var fixedConvars = new Dictionary<string, string>()
            {
                { "monitor_killServerOnBrokenPipe", "1" },
                { "citizen_dir", GetConvar("citizen_dir", "") },
                { "con_disableNonTTYReads", "true" }
            };

            foreach (var convar in m_instanceConfig.ConVars.Concat(fixedConvars))
            {
                arguments.Add("+set");
                arguments.Add(convar.Key);
                arguments.Add(convar.Value);
            }

            foreach (var command in m_instanceConfig.Commands)
            {
                // TODO: handle spaces
                var parts = command.Split(new char[] { ' ' }, 3);
                arguments.Add("+" + parts[0]);
                arguments.AddRange(parts.Skip(1));
            }

            psi.Arguments = string.Join(" ",
                arguments
                    .Select(a => a.Replace("\"", "\\\""))
                    .Select(a =>
                        a.Contains(" ") || a.Contains("\"")
                            ? $"\"{a}\""
                            : a));

            psi.UseShellExecute = false;
            psi.CreateNoWindow = false;
            psi.WindowStyle = ProcessWindowStyle.Normal;

            psi.RedirectStandardError = true;
            psi.RedirectStandardOutput = true;

            // TODO: redirect stdout/stderr and handle closure(?)

            m_process = new Process();
            m_process.StartInfo = psi;
            if (!m_process.Start())
            {
                await BaseScript.Delay(5000);
            }

            void PrintOutput(object sender, DataReceivedEventArgs args) =>
                Debug.WriteLine("^5!^7 {0} => {1}", Name, args.Data);

            if (GetConvar("monitor_showServerOutput", "0") != "0")
            {
                m_process.OutputDataReceived += PrintOutput;
                m_process.ErrorDataReceived += PrintOutput;
            }

            m_process.BeginOutputReadLine();
            m_process.BeginErrorReadLine();

            // save a pidfile
            Directory.CreateDirectory(Path.Combine(ms_rootPath, "cache"));
            File.WriteAllText(Path.Combine(ms_rootPath, "cache", $"{Name}.pid"), m_process.Id.ToString());
        }

        public async Task Monitor()
        {
            bool healthy = true;

            if (m_process.HasExited)
            {
                healthy = false;
            }

            /*if (m_hadPipe && !m_inPipe.IsConnected)
            {
                healthy = false;
            }*/

            if (!healthy)
            {
                // TODO: various kinds of backoff
                Debug.WriteLine($"^3?^7 {Name}");

                await Stop();
                await Start();
            }
        }

        public async Task Stop()
        {
            try
            {
                m_pair.Dispose();
            } catch {}

            m_hadPipe = false;
            m_pair = null;

            try
            {
                m_process?.Kill();
            }
            catch {}

            m_process = null;
        }
    }
}