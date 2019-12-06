using System;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Threading.Tasks;
using CitizenFX.Core;

using nng;
using ProtoBuf;

using static CitizenFX.Core.Native.API;

namespace FxMonitor
{
    public class MonitorClient : BaseScript
    {
        private nng.IPairSocket m_pair;
        private PipePeer m_peer;

        private bool m_firstTick = false;

        private string m_nucleusUrl;

        internal MonitorClient()
        {
            // TODO: cleaner way?
            var nngFactory = new nng.Tests.TestFactory();

            var pairResult = nngFactory
                .PairOpen()
                .ThenDial($"ipc:///tmp/fxs_instance_{Process.GetCurrentProcess().Id}");

            void TryQuit()
            {
                if (GetConvar("monitor_killServerOnBrokenPipe", "0") != "0")
                {
                    Tick += async () => ExecuteCommand("quit \"Monitor pipe broken.\"");
                }
            }

            if (pairResult.IsErr())
            {
                TryQuit();

                return;
            }

            m_pair = pairResult.Ok();
            m_peer = new PipePeer(m_pair);

            m_peer.CommandReceived += async cmd =>
            {
                await HandleCommand(cmd);
            };

            m_peer.TargetUnreachable += () =>
            {
                TryQuit();
            };

            m_peer.Start();

            Tick += MonitorClient_Tick;
        }

        private Task MonitorClient_Tick()
        {
            if (!m_firstTick)
            {
                if (m_peer != null)
                {
                    m_peer.WriteCommand(2, new GetPortResponseCommand(GetConvarInt("netPort", 30120), m_nucleusUrl));

                    m_firstTick = true;
                }
            }

            return Task.CompletedTask;
        }

        [EventHandler("_cfx_internal:nucleusConnected")]
        public void OnNucleusConnected(string url)
        {
            m_nucleusUrl = url;

            Task.Run(async () =>
            {
                if (m_peer != null)
                {
                    await m_peer.WriteCommand(3, new NucleusConnectedCommand(url));
                }
            });
        }

        private async Task HandleCommand(BaseCommand command)
        {
            switch (command.Type)
            {
                case 1:
                    await m_peer.WriteCommand(2, new GetPortResponseCommand(GetConvarInt("netPort", 30120), m_nucleusUrl));
                    break;
            }
        }
    }
}