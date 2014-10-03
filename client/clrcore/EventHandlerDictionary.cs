using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public class EventHandlerDictionary : Dictionary<string, EventHandlerEntry>
    {
        public new EventHandlerEntry this[string key]
        {
            get
            {
                var lookupKey = key.ToLower();

                if (this.ContainsKey(lookupKey))
                {
                    return base[lookupKey];
                }

                var entry = new EventHandlerEntry(key);
                base.Add(lookupKey, entry);

                return entry;
            }
            set
            {
                // no operation
            }
        }

        public void Add(string key, Delegate value)
        {
            this[key] += value;
        }

        internal void Invoke(string eventName, object[] arguments)
        {
            var lookupKey = eventName.ToLower();
            EventHandlerEntry entry;
            
            if (TryGetValue(lookupKey, out entry))
            {
                entry.Invoke(arguments);
            }
        }
    }

    public class EventHandlerEntry
    {
        private string m_eventName;
        private List<Delegate> m_callbacks = new List<Delegate>();

        public EventHandlerEntry(string eventName)
        {
            m_eventName = eventName;
        }

        public static EventHandlerEntry operator +(EventHandlerEntry entry, Delegate deleg)
        {
            entry.m_callbacks.Add(deleg);

            return entry;
        }

        public static EventHandlerEntry operator -(EventHandlerEntry entry, Delegate deleg)
        {
            entry.m_callbacks.Remove(deleg);

            return entry;
        }

        internal void Invoke(params object[] args)
        {
            var callbacks = m_callbacks.ToArray();

            foreach (var callback in callbacks)
            {
                try
                {
                    callback.DynamicInvoke(args);
                }
                catch (Exception e)
                {
                    Debug.WriteLine("Error invoking callback for event {0}: {1}", m_eventName, e.ToString());

                    m_callbacks.Remove(callback);
                }
            }
        }
    }
}
