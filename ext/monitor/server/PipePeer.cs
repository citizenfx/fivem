using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using nng;
using ProtoBuf;

namespace FxMonitor
{
    internal class PipePeer
    {
        private readonly IPairSocket m_socket;
        private bool m_running;

        public event Func<BaseCommand, Task> CommandReceived;
        public event Action TargetUnreachable;

        private bool m_errored;
        private DateTime m_lastPong;

        public PipePeer(IPairSocket socket)
        {
            m_socket = socket;
        }

        public void Start()
        {
            m_running = true;

            Task.Run(async () =>
            {
                while (m_running)
                {
                    await Task.Delay(2500);
                    await WriteCommand(4, "ping");

                    if (m_lastPong.Ticks == 0)
                    {
                        continue;
                    }

                    if ((DateTime.UtcNow - m_lastPong) > TimeSpan.FromSeconds(15))
                    {
                        if (!m_errored)
                        {
                            TargetUnreachable?.Invoke();
                        }

                        m_errored = true;
                    }
                    else
                    {
                        m_errored = false;
                    }
                }
            });

            Task.Run(async () =>
            {
                while (m_running)
                {
                    try
                    {
                        var (success, err, message) = m_socket.RecvMsg();

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
                }
            });
        }

        private async Task HandleCommand(BaseCommand command)
        {
            // ping/pong
            if (command.Type == 4)
            {
                var cmd = command.DeserializeAs<string>();

                if (cmd == "ping")
                {
                    await WriteCommand(4, "pong");
                }
                else if (cmd == "pong")
                {
                    m_lastPong = DateTime.UtcNow;
                }

                return;
            }

            await CommandReceived?.Invoke(command);
        }

        public void Stop()
        {
            m_running = false;
        }

        public Task WriteCommand<T>(int type, T msg)
        {
            if (m_socket != null)
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

                    m_socket.Send(outStream.ToArray(), nng.Native.Defines.NngFlag.NNG_FLAG_NONBLOCK);
                }
                catch { }
            }

            return Task.CompletedTask;
        }
    }
}
