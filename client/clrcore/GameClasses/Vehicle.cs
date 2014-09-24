using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class Vehicle : Entity
    {
        private bool m_hasExisted;

        internal override void SetHandle(int handle)
        {
            if (m_handle != -1)
            {
                ObjectCache<Vehicle>.Remove(this);
            }

            m_handle = handle;
            m_hasExisted = false;

            ObjectCache<Vehicle>.Add(this);
        }

        protected override uint GetCoordinatesFunction
        {
            get { return Natives.GET_CAR_COORDINATES; }
        }

        protected override uint SetCoordinatesFunction
        {
            get { return Natives.SET_CAR_COORDINATES; }
        }

        protected override uint GetHeadingFunction
        {
            get { return Natives.GET_CAR_HEADING; }
        }

        protected override uint SetHeadingFunction
        {
            get { return Natives.SET_CAR_HEADING; }
        }
    }
}
