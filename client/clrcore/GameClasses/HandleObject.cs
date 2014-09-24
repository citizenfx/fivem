using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public abstract class HandleObject
    {
        protected int m_handle;

        public HandleObject()
        {
            m_handle = -1;
        }

        internal int Handle
        {
            get
            {
                return m_handle;
            }
        }

        internal abstract void SetHandle(int handle);
    }
}
