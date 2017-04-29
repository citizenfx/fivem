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
        public dynamic this[string resourceName] => new ExportSet(resourceName);
    }

    public class ExportSet : DynamicObject
    {
        private readonly string m_resourceName;

        public ExportSet(string resourceName)
        {
			m_resourceName = resourceName ?? throw new ArgumentNullException(nameof(resourceName));
        }

        private class DelegateFn
        {
            public dynamic Delegate { get; set; }
        }

        public override bool TryInvokeMember(InvokeMemberBinder binder, object[] args, out object result)
        {
            // get the event name
            var eventName = $"__cfx_export_{m_resourceName}_{binder.Name}";

            // get the member
            var exportDelegate = new DelegateFn();
            BaseScript.TriggerEvent(eventName, new Action<dynamic>(a => exportDelegate.Delegate = a));

            if (exportDelegate.Delegate == null)
            {
                result = null;
                return false;
            }

            // try invoking it
            result = exportDelegate.Delegate(args);

            return true;
        }
    }
}
