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

        private bool m_running = false;

        private string m_nucleusUrl;

        internal MonitorClient()
        {
            // TODO: cleaner way?
            var nngFactory = new nng.Tests.TestFactory();

            var pairResult = nngFactory
                .PairOpen()
                .ThenDial($"ipc:///tmp/fxs_instance_{Process.GetCurrentProcess().Id}");

            if (pairResult.IsErr())
            {
                if (GetConvar("monitor_killServerOnBrokenPipe", "0") != "0")
                {
                    ExecuteCommand("quit");
                }

                return;
            }

            m_pair = pairResult.Ok();

            Task.Run(async () =>
            {
                // also send on initial connect
                await WriteCommand(2, new GetPortResponseCommand(GetConvarInt("netPort", 30120), m_nucleusUrl));

                while (m_running)
                {
                    try
                    {
                        var (success, err, message) = m_pair.RecvMsg(nng.Native.Defines.NngFlag.NNG_FLAG_NONBLOCK);

                        if (success)
                        {
                            var cmd = Serializer.Deserialize<BaseCommand>(new MemoryStream(message.AsSpan().ToArray()));

                            if (cmd != null)
                            {
                                await HandleCommand(cmd);
                            }
                        }
                    }
                    catch (Exception e) { CitizenFX.Core.Debug.WriteLine($"{e}"); }

                    await Task.Delay(500);
                }
            });
        }

        [EventHandler("_cfx_internal:nucleusConnected")]
        public void OnNucleusConnected(string url)
        {
            m_nucleusUrl = url;

            Task.Run(async () =>
            {
                await WriteCommand(3, new NucleusConnectedCommand(url));
            });
        }

        private async Task HandleCommand(BaseCommand command)
        {
            switch (command.Type)
            {
                case 1:
                    await WriteCommand(2, new GetPortResponseCommand(GetConvarInt("netPort", 30120), m_nucleusUrl));
                    break;
            }
        }

        private Task WriteCommand<T>(int type, T msg)
        {
            if (m_pair != null)
            {
                var ms = new MemoryStream();
                Serializer.Serialize<T>(ms, msg);

                try
                {
                    var outStream = new MemoryStream();
                    Serializer.Serialize(outStream, new BaseCommand()
                    {
                        Type = type,
                        Data = ms.ToArray()
                    });

                    m_pair.Send(outStream.ToArray());
                } catch {}
            }

            return Task.CompletedTask;
        }
    }
}