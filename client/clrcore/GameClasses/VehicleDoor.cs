using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class VehicleDoor
    {
        private Vehicle m_vehicle;
        private VehicleDoors m_index;

        public VehicleDoors Index
        {
            get
            {
                return m_index;
            }
        }

        internal VehicleDoor(Vehicle vehicle, VehicleDoors index)
        {
            m_vehicle = vehicle;
            m_index = index;
        }
    }
}
