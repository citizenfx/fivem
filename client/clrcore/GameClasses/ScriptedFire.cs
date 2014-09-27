using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class ScriptedFire : HandleObject
    {
        private bool m_hasExisted;

        public Vector3 Position
        {
            get
            {
                Pointer x = typeof(float), y = typeof(float), z = typeof(float);
                Function.Call(Natives.GET_SCRIPT_FIRE_COORDS, m_handle, x, y, z);

                return new Vector3((float)x, (float)y, (float)z);
            }
        }

        public bool Exists
        {
            get
            {
                if (m_hasExisted)
                {
                    return true;
                }

                if (m_handle == 0)
                {
                    return false;
                }

                try
                {
                    m_hasExisted = Function.Call<bool>(Natives.DOES_SCRIPT_FIRE_EXIST, m_handle);

                    return m_hasExisted;
                }
                catch
                {
                    return false;
                }
            }
        }

        public void Delete()
        {
            if (m_handle == 0)
                return;

            Function.Call(Natives.REMOVE_SCRIPT_FIRE, m_handle);
        }

        internal override void SetHandle(int handle)
        {
            if (m_handle != -1)
                ObjectCache<ScriptedFire>.Remove(this);

            m_handle = handle;
            m_hasExisted = false;

            ObjectCache<ScriptedFire>.Add(this);
        }
    }
}
