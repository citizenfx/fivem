using System.IO;
using ProtoBuf;

namespace FxMonitor
{
    [ProtoContract]
    internal class BaseCommand
    {
        [ProtoMember(1)]
        public int Type { get; set; }

        [ProtoMember(2)]
        public byte[] Data { get; set; }

        public T DeserializeAs<T>()
        {
            var ms = new MemoryStream(Data);
            return Serializer.Deserialize<T>(ms);
        }
    }

    [ProtoContract]
    internal class GetPortResponseCommand
    {
        [ProtoMember(1)]
        public int Port { get; set; }

        [ProtoMember(2)]
        public string NucleusUrl { get; set; }

        public GetPortResponseCommand()
        {

        }

        public GetPortResponseCommand(int port, string nucleusUrl)
        {
            Port = port;
            NucleusUrl = nucleusUrl;
        }
    }

    [ProtoContract]
    internal class NucleusConnectedCommand
    {
        [ProtoMember(1)]
        public string Url { get; set; }

        public NucleusConnectedCommand()
        {

        }

        public NucleusConnectedCommand(string url)
        {
            Url = url;
        }
    }
}