using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading;
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

        internal async Task Invoke(string eventName, string sourceString, object[] arguments)
        {
            var lookupKey = eventName.ToLower();
            EventHandlerEntry entry;
            
            if (TryGetValue(lookupKey, out entry))
            {
                await entry.Invoke(sourceString, arguments);
            }
        }
    }

    public class EventHandlerEntry
    {
        private readonly string m_eventName;
        private readonly List<Delegate> m_callbacks = new List<Delegate>();

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

        internal async Task Invoke(string sourceString, params object[] args)
        {
            var callbacks = m_callbacks.ToArray();

            foreach (var callback in callbacks)
            {
                try
                {
					var passArgs = CallUtilities.GetPassArguments(callback.Method, args, sourceString);
					var rv = callback.DynamicInvoke(passArgs);

					if (rv != null && rv is Task task)
					{
						await task;
					}
                }
                catch (Exception e)
                {
                    Debug.WriteLine("Error invoking callback for event {0}: {1}", m_eventName, e.ToString());

                    m_callbacks.Remove(callback);
                }
            }
        }
    }

	static class CallUtilities
	{
		public static object[] GetPassArguments(MethodInfo method, object[] args, string sourceString)
		{
			List<object> passArgs = new List<object>();

			var argIdx = 0;

			object ChangeType(object value, Type type)
			{
				// FIX: Null values should be checked against. This allows greater freedom for developers in more "complex" cases.
				// (Eg; passing "null" as a string value to an event invocation, rather than forcing string.Empty)
				if (ReferenceEquals(value, null))
					return null;

				if (type.IsAssignableFrom(value.GetType()))
				{
					return value;
				}
				else if (value is IConvertible)
				{
					return Convert.ChangeType(value, type);
				}

				throw new InvalidCastException($"Could not cast event argument from {value.GetType().Name} to {type.Name}");
			}

			object Default(Type type) => type.IsValueType ? Activator.CreateInstance(type) : null;

			foreach (var info in method.GetParameters())
			{
				var type = info.ParameterType;

				if (info.GetCustomAttribute<FromSourceAttribute>() != null)
				{
					// empty source -> default
					// FIX: Allow null source strings. (Shouldn't really be a thing, but a simple check to fix)
					if (string.IsNullOrEmpty(sourceString) || string.IsNullOrWhiteSpace(sourceString))
					{
						passArgs.Add(Default(type));

						continue;
					}

#if IS_FXSERVER
					if (type.IsAssignableFrom(typeof(Player)))
					{
						passArgs.Add(new Player(sourceString));

						continue;
					}
#endif

					passArgs.Add(ChangeType(sourceString, type));
				}
				else
				{
					if (argIdx >= args.Length)
					{
						passArgs.Add(Default(type));
					}
					else
					{
						passArgs.Add(ChangeType(args[argIdx], type));
					}

					argIdx++;
				}
			}

			return passArgs.ToArray();
		}
	}
}
