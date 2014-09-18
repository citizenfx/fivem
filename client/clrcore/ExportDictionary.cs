using System;
using System.Collections.Generic;
using System.Dynamic;
using System.Linq;
using System.Security;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public class ExportDictionary
    {
        public new dynamic this[string resourceName]
        {
            get
            {
                return new ExportSet(resourceName);
            }
        }
    }

    public class ExportSet : DynamicObject
    {
        private string m_resourceName;

        public ExportSet(string resourceName)
        {
            if (resourceName == null)
            {
                throw new ArgumentNullException("resourceName");
            }

            m_resourceName = resourceName;
        }

        public override bool TryInvokeMember(InvokeMemberBinder binder, object[] args, out object result)
        {
            // serialize arguments
            var argsSerialized = MsgPackSerializer.Serialize(args);

            // try invoking the actual member
            var resultSerialized = InvokeResourceExport(binder.Name, argsSerialized);

            if (resultSerialized == null)
            {
                result = null;
                return false;
            }

            var returnData = MsgPackDeserializer.Deserialize(resultSerialized) as List<object>;

            if (returnData == null || returnData.Count == 0)
            {
                result = null;
            }
            else
            {
                result = returnData[0];
            }

            return true;
        }
        
        [SecuritySafeCritical]
        private byte[] InvokeResourceExport(string exportName, byte[] argsSerialized)
        {
            return GameInterface.InvokeResourceExport(m_resourceName, exportName, argsSerialized);
        }
    }
}
