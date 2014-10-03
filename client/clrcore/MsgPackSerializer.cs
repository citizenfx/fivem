using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

using MsgPack;

namespace CitizenFX.Core
{
    static class MsgPackSerializer
    {
        //private static SerializationContext ms_context = new SerializationContext();

        static MsgPackSerializer()
        {
            //SerializationContext.Default.CompatibilityOptions.PackerCompatibilityOptions = MsgPack.PackerCompatibilityOptions.None; // we want to use ext types, as our unpackers aren't retarded
            //SerializationContext.Default.Serializers.RegisterOverride(new DelegateSerializer());
        }

        private static readonly Type[] WriteTypes = new[] {
            typeof(string), typeof(DateTime), typeof(Enum), 
            typeof(decimal), typeof(Guid),
        };

        public static bool IsSimpleType(this Type type)
        {
            return type.IsPrimitive || WriteTypes.Contains(type);
        }

        public static byte[] Serialize(object obj)
        {
            /*if (obj == null)
            {
                return new byte[] { 0xC0 };
            }

            var serializer = MessagePackSerializer.Get(obj.GetType());

            return serializer.PackSingleObject(obj);*/

            if (obj == null)
            {
                return new byte[] { 0xC0 };
            }

            var stream = new MemoryStream();
            using (var packer = Packer.Create(stream, PackerCompatibilityOptions.None))
            {
                Serialize(obj, packer);

                return stream.ToArray();
            }
        }

        private static void Serialize(object obj, Packer packer)
        {
            if (obj == null)
            {
                packer.PackNull();

                return;
            }

            var type = obj.GetType();

            if (type.IsSimpleType())
            {
                packer.Pack(obj);
            }
            else if (obj is IDictionary)
            {
                var dict = (IDictionary)obj;

                packer.PackMapHeader(dict.Count);

                foreach (var key in dict.Keys)
                {
                    Serialize(key, packer);
                    Serialize(dict[key], packer);
                }
            }
            else if (obj is IList)
            {
                var list = (IList)obj;

                packer.PackArrayHeader(list.Count);

                foreach (var item in list)
                {
                    Serialize(item, packer);
                }
            }
            else if (obj is IEnumerable)
            {
                var enu = (IEnumerable)obj;

                var list = new List<object>();

                foreach (var item in enu)
                {
                    list.Add(item);
                }

                packer.PackArrayHeader(list.Count);

                list.ForEach(a => Serialize(a, packer));
            }
            else if (obj is IPackable)
            {
                var packable = (IPackable)obj;

                packable.PackToMessage(packer, null);
            }
            else
            {
                var properties = type.GetProperties();
                var dict = new Dictionary<string, object>();

                foreach (var property in properties)
                {
                    dict[property.Name] = property.GetValue(obj, null);
                }

                Serialize(dict, packer);
            }
        }
    }

    /*class DelegateSerializer : MessagePackSerializer<Delegate>
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
    }*/
}
