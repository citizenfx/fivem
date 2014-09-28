using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class VehicleExtra
    {
        private Vehicle m_vehicle;
        private int m_extraId;

        public int Id
        {
            get
            {
                return m_extraId;
            }
        }

        internal VehicleExtra(Vehicle vehicle, int extraId)
        {
            m_vehicle = vehicle;
            m_extraId = extraId;
        }
    }
}
