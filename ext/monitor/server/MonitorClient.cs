using System;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Threading.Tasks;
using CitizenFX.Core;

using ProtoBuf;

using static CitizenFX.Core.Native.API;

namespace FxMonitor
{
    public class MonitorClient : BaseScript
    {
        private NamedPipeServerStream m_inPipe;
        private NamedPipeServerStream m_outPipe;

        private bool m_ran = false;

        private string m_nucleusUrl;

        internal MonitorClient()
        {
            m_inPipe = new NamedPipeServerStream($"FXServer_Instance_MOut_{Process.GetCurrentProcess().Id}", PipeDirection.InOut, 1, PipeTransmissionMode.Byte);
            m_outPipe = new NamedPipeServerStream($"FXServer_Instance_MIn_{Process.GetCurrentProcess().Id}", PipeDirection.InOut, 1, PipeTransmissionMode.Byte);

            Task.Run(async () =>
            {
                await m_inPipe.WaitForConnectionAsync();
            });

            Task.Run(async () =>
            {
                while (true)
                {
                    await m_outPipe.WaitForConnectionAsync();

                    while (m_outPipe.IsConnected)
                    {
                        await Task.Delay(500);
                    }

                    m_outPipe = new NamedPipeServerStream($"FXServer_Instance_MIn_{Process.GetCurrentProcess().Id}", PipeDirection.InOut, 1, PipeTransmissionMode.Byte);
                }
            });
        }

        [Tick]
        public Task MonitorClient_Tick()
        {
            if (m_inPipe.IsConnected && !m_ran)
            {
                m_ran = true;

                Task.Run(async () =>
                {
                    try
                    {
                        while (m_inPipe.IsConnected)
                        {
                            var cmd = Serializer.DeserializeWithLengthPrefix<BaseCommand>(m_inPipe, PrefixStyle.Base128);
                            
                            await HandleCommand(cmd);
                        }
                    }
                    catch (Exception e) { CitizenFX.Core.Debug.WriteLine($"{e}"); }

                    if (GetConvar("monitor_killServerOnBrokenPipe", "0") != "0")
                    {
                        ExecuteCommand("quit");
                    }

                    m_inPipe.Close();
                    m_inPipe = new NamedPipeServerStream($"FXServer_Instance_{Process.GetCurrentProcess().Id}", PipeDirection.InOut, 1);
                    await m_inPipe.WaitForConnectionAsync();

                    m_ran = false;
                });
            }

            return BaseScript.Delay(1000);
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
            if (m_outPipe.IsConnected)
            {
                var ms = new MemoryStream();
                Serializer.Serialize<T>(ms, msg);

                try
                {
                    Serializer.SerializeWithLengthPrefix(m_outPipe, new BaseCommand()
                    {
                        Type = type,
                        Data = ms.ToArray()
                    }, PrefixStyle.Base128);
                } catch {}
            }

            return Task.CompletedTask;
        }
    }
}