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
        private VehicleDoors m_door;

        public VehicleDoors Door
        {
            get
            {
                return m_door;
            }
        }

        internal VehicleDoor(Vehicle vehicle, VehicleDoors door)
        {
            m_vehicle = vehicle;
            m_door = door;
        }

        public float Angle
        {
            get
            {
                if (!m_vehicle.Exists)
                    return 0.0f;

                Pointer anglePtr = typeof(float);
                Function.Call(Natives.GET_DOOR_ANGLE_RATIO, m_vehicle.Handle, (int)m_door, anglePtr);

                return (float)anglePtr;
            }
            set
            {
                if (!m_vehicle.Exists)
                    return;

                if (value > 1.0f)
                    value = 1.0f;

                if (value > 0.001f)
                    Function.Call(Natives.CONTROL_CAR_DOOR, m_vehicle.Handle, (uint)m_door, value);
                else
                    Close();
            }
        }

        public bool IsFullyOpen
        {
            get
            {
                if (!m_vehicle.Exists)
                    return false;

                return Function.Call<bool>(Natives.IS_CAR_DOOR_FULLY_OPEN, m_vehicle.Handle, (int)m_door);
            }
        }

        public bool IsOpen
        {
            get
            {
                if (!m_vehicle.Exists)
                    return false;

                return Angle > 0.001f;
            }
            set
            {
                if (!m_vehicle.Exists)
                    return;

                if (value)
                    Open();
                else
                    Close();
            }
        }

        public bool IsDamaged
        {
            get
            {
                if (!m_vehicle.Exists)
                    return false;

                return Function.Call<bool>(Natives.IS_CAR_DOOR_DAMAGED, m_vehicle.Handle, (int)m_door);
            }
        }

        public void Open()
        {
            if (!m_vehicle.Exists)
                return;

            Function.Call(Natives.OPEN_CAR_DOOR, m_vehicle.Handle, (int)m_door);
        }

        public void Close()
        {
            if (!m_vehicle.Exists)
                return;

            Function.Call(Natives.SHUT_CAR_DOOR, m_vehicle.Handle, (int)m_door);
        }

        public void Break()
        {
            if (!m_vehicle.Exists)
                return;

            Function.Call(Natives.BREAK_CAR_DOOR, m_vehicle.Handle, (int)m_door, false);
        }
    }
}