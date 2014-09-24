using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public abstract class Entity : HandleObject
    {
        public Vector3 Position
        {
            get
            {
                Pointer pX = typeof(float), pY = typeof(float), pZ = typeof(float);

                Function.Call(GetCoordinatesFunction, m_handle, pX, pY, pZ);

                return new Vector3((float)pX, (float)pY, (float)pZ);
            }
            set
            {
                Function.Call(SetCoordinatesFunction, m_handle, value.X, value.Y, value.Z);
            }
        }

        public float Heading
        {
            get
            {
                Pointer pH = typeof(float);

                Function.Call(GetHeadingFunction, m_handle, pH);

                return (float)pH;
            }
            set
            {
                Function.Call(SetHeadingFunction, m_handle, value);
            }
        }

        protected abstract uint GetCoordinatesFunction
        {
            get;
        }

        protected abstract uint SetCoordinatesFunction
        {
            get;
        }

        protected abstract uint GetHeadingFunction { get; }
        protected abstract uint SetHeadingFunction { get; }
    }
}
