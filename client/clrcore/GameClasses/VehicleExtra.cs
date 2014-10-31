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

        public bool Enabled
        {
            get
            {
                if (!m_vehicle.Exists)
                    return false;

                return Function.Call<bool>(Natives.IS_VEHICLE_EXTRA_TURNED_ON, m_vehicle.Handle, m_extraId);
            }
            set
            {
                if (!m_vehicle.Exists)
                    return;

                Function.Call(Natives.TURN_OFF_VEHICLE_EXTRA, m_vehicle.Handle, m_extraId, !value);
            }
        }

        internal VehicleExtra(Vehicle vehicle, int extraId)
        {
            m_vehicle = vehicle;
            m_extraId = extraId;
        }
    }
}