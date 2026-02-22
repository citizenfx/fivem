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
        private readonly Delegate m_method;

        private FunctionReference(Delegate method)
        {
            m_method = method;
        }

        public int Identifier { get; private set; }

		private int m_refCount;


        public static FunctionReference Create(Delegate method)
        {
            var reference = new FunctionReference(method);
            var referenceId = Register(reference);

            reference.Identifier = referenceId;

            return reference;            
        }

        private static Dictionary<int, FunctionReference> ms_references = new Dictionary<int, FunctionReference>();
        private static int ms_referenceId = 0;

        private static int Register(FunctionReference reference)
        {
            var thisRefId = ms_referenceId;
            ms_references[thisRefId] = reference;

            unchecked { ms_referenceId++; }

            return thisRefId;
        }

        public static byte[] Invoke(int reference, byte[] arguments)
        {
			if (!ms_references.TryGetValue(reference, out var funcRef))
            {
                Debug.WriteLine("No such reference for {0}.", reference);

                // return nil
                return MsgPackSerializer.Serialize(new object[] { false, null });
            }

            var method = funcRef.m_method;

            // deserialize the passed arguments
            var argList = (List<object>)MsgPackDeserializer.Deserialize(arguments);
            var argArray = CallUtilities.GetPassArguments(method.Method, argList.ToArray(), string.Empty);

			// the Lua runtime expects this to be an array, so it be an array.
			var rv = method.DynamicInvoke(argArray);

			// is this actually an asynchronous method?
			if (rv is Task)
			{
				dynamic rt = rv;

				rv = new
				{
					__cfx_async_retval = new Action<dynamic>(rvcb =>
					{
						rt.ContinueWith(new Action<Task>(t =>
						{
							rvcb(new object[] { rt.Result }, false);
						}));
					})
				};
			}

			return MsgPackSerializer.Serialize(new object[] { true, new[] { rv } });
        }

        public static int Duplicate(int reference)
        {
            FunctionReference funcRef;

            if (ms_references.TryGetValue(reference, out funcRef))
            {
				funcRef.m_refCount++; // TODO: interlocked?
				return reference;
            }

            return -1;
        }

        public static void Remove(int reference)
        {
            if (ms_references.TryGetValue(reference, out var funcRef))
            {
				if (--funcRef.m_refCount <= 0)
				{
					ms_references.Remove(reference);
				}
            }
        }
    }
}
