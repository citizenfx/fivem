using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Security;

namespace CitizenFX.Core
{
    class FunctionReference
    {
        private Delegate m_method;

        private FunctionReference(Delegate method)
        {
            m_method = method;
        }

        public uint Identifier { get; private set; }

        public uint Instance { get; private set; }

        public string Resource { get; private set; }

        public static FunctionReference Create(Delegate method)
        {
            var reference = new FunctionReference(method);
            var referenceId = Register(reference);

            reference.Identifier = referenceId;

            reference.FillOutOurDetails();

            return reference;            
        }

        [SecuritySafeCritical]
        private void FillOutOurDetails()
        {
            string resourceName;
            string resourcePath;
            string resourceAssembly;
            uint instanceId;

            if (!GameInterface.GetEnvironmentInfo(out resourceName, out resourcePath, out resourceAssembly, out instanceId))
            {
                throw new Exception("Could not get environment information while creating a callback junction.");
            }

            Resource = resourceName;
            Instance = instanceId;
        }

        private static Dictionary<uint, FunctionReference> ms_references = new Dictionary<uint, FunctionReference>();
        private static uint ms_referenceId = 0;

        private static uint Register(FunctionReference reference)
        {
            uint thisRefId = ms_referenceId;
            ms_references[thisRefId] = reference;

            unchecked { ms_referenceId++; }

            return thisRefId;
        }

        public static byte[] Invoke(uint reference, byte[] arguments)
        {
            FunctionReference funcRef;
            
            if (!ms_references.TryGetValue(reference, out funcRef))
            {
                Debug.WriteLine("No such reference for {0}.", reference);

                // return nil
                return new byte[] { 0xC0 };
            }

            var method = funcRef.m_method;

            // deserialize the passed arguments
            var argList = (List<object>)MsgPackDeserializer.Deserialize(arguments);
            var argArray = argList.ToArray();

            // the Lua runtime expects this to be an array, so it be an array.
            return MsgPackSerializer.Serialize(new object[] { method.DynamicInvoke(argArray) });
        }

        public static void Remove(uint reference)
        {
            if (ms_references.ContainsKey(reference))
            {
                ms_references.Remove(reference);
            }
        }
    }
}
