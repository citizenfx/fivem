using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using MsgPack.Serialization;

namespace CitizenFX.Core
{
    static class MsgPackSerializer
    {
        //private static SerializationContext ms_context = new SerializationContext();

        static MsgPackSerializer()
        {
            SerializationContext.Default.CompatibilityOptions.PackerCompatibilityOptions = MsgPack.PackerCompatibilityOptions.None; // we want to use ext types, as our unpackers aren't retarded
            SerializationContext.Default.Serializers.RegisterOverride(new DelegateSerializer());
        }

        public static byte[] Serialize(object obj)
        {
            if (obj == null)
            {
                return new byte[] { 0xC0 };
            }

            var serializer = MessagePackSerializer.Get(obj.GetType());

            return serializer.PackSingleObject(obj);
        }
    }

    class DelegateSerializer : MessagePackSerializer<Delegate>
    {
        public DelegateSerializer()
            : base(SerializationContext.Default)
        {

        }

        protected override void PackToCore(MsgPack.Packer packer, Delegate objectTree)
        {
            if (objectTree is CallbackDelegate)
            {
                throw new InvalidOperationException("Resending callback delegates is currently not supported.");
            }

            var funcRefDetails = FunctionReference.Create(objectTree);

            var resourceNameBytes = Encoding.UTF8.GetBytes(funcRefDetails.Resource);
            var delegateData = new byte[8 + resourceNameBytes.Length];

            Array.Copy(BitConverter.GetBytes(funcRefDetails.Identifier).Reverse().ToArray(), 0, delegateData, 0, 4);
            Array.Copy(BitConverter.GetBytes(funcRefDetails.Instance).Reverse().ToArray(), 0, delegateData, 4, 4);
            Array.Copy(resourceNameBytes, 0, delegateData, 8, resourceNameBytes.Length);

            packer.PackExtendedTypeValue(1, delegateData);
        }

        protected override Delegate UnpackFromCore(MsgPack.Unpacker unpacker)
        {
            throw new NotImplementedException();
        }
    }
}
