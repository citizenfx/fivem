using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class Blip : HandleObject
    {
        private bool m_hasExisted;

        public float Alpha
        {
            set
            {
                Function.Call(Natives.CHANGE_BLIP_ALPHA, m_handle, value);
            }
        }

        public BlipColor Color
        {
            get
            {
                Pointer pColor = typeof(int);

                Function.Call(Natives.GET_BLIP_COLOUR, m_handle, pColor);

                return (BlipColor)(int)pColor;
            }
            set
            {
                Function.Call(Natives.CHANGE_BLIP_COLOUR, m_handle, (int)value);
            }
        }

        public BlipIcon Icon
        {
            get
            {
                return (BlipIcon)Function.Call<int>(Natives.GET_BLIP_SPRITE, m_handle);
            }
            set
            {
                Function.Call(Natives.CHANGE_BLIP_SPRITE, m_handle, (int)value);
            }
        }

        public string Name
        {
            set
            {
                Function.Call(Natives.CHANGE_BLIP_NAME_FROM_ASCII, this.Handle, value);
            }
        }

        public BlipType Type
        {
            get
            {
                return (BlipType)Function.Call<uint>(Natives.GET_BLIP_INFO_ID_TYPE, m_handle);
            }
        }

        public bool ShowOnlyWhenNear
        {
            get
            {
                return Function.Call<bool>(Natives.IS_BLIP_SHORT_RANGE, m_handle);
            }
            set
            {
                Function.Call(Natives.SET_BLIP_AS_SHORT_RANGE, m_handle, value);
            }
        }

        public BlipDisplay Display
        {
            get
            {
                return (BlipDisplay)Function.Call<int>(Natives.GET_BLIP_INFO_ID_DISPLAY, m_handle);
            }
            set
            {
                Function.Call(Natives.CHANGE_BLIP_DISPLAY, m_handle, (int)value);
            }
        }

        public bool Friendly
        {
            set
            {
                Function.Call(Natives.SET_BLIP_AS_FRIENDLY, m_handle, value);
            }
        }

        public float Scale
        {
            set
            {
                Function.Call(Natives.CHANGE_BLIP_SCALE, m_handle, value);
            }
        }

        public int Priority
        {
            set
            {
                Function.Call(Natives.CHANGE_BLIP_PRIORITY, m_handle, value);
            }
        }

        public bool RouteActive
        {
            set
            {
                Function.Call(Natives.SET_ROUTE, m_handle, value);
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
                    m_hasExisted = Function.Call<bool>(Natives.DOES_BLIP_EXIST, m_handle);

                    return m_hasExisted;
                }
                catch
                {
                    return false;
                }
            }
        }

        internal override void SetHandle(int handle)
        {
            if (m_handle != -1)
                ObjectCache<Blip>.Remove(this);

            m_handle = handle;
            m_hasExisted = false;

            ObjectCache<Blip>.Add(this);
        }

        public void Remove()
        {
            Display = BlipDisplay.Hidden;
            Function.Call(Natives.REMOVE_BLIP, m_handle);
        }

        public Entity GetAttachedItem()
        {
            int id;
            switch (Type)
            {
                case BlipType.Vehicle:
                    id = Function.Call<int>(Natives.GET_BLIP_INFO_ID_CAR_INDEX, m_handle);
                    if (!Function.Call<bool>(Natives.DOES_VEHICLE_EXIST, id)) 
                        return null;

                    return ObjectCache<Vehicle>.Get(id);

                case BlipType.Ped:
                    id = Function.Call<int>(Natives.GET_BLIP_INFO_ID_PED_INDEX, m_handle);
                    if (!Function.Call<bool>(Natives.DOES_CHAR_EXIST, id))
                        return null;

                    return ObjectCache<Ped>.Get(id);

                /*case BlipType.Pickup:
                    id = Function.Call<uint>(Natives.GET_BLIP_INFO_ID_PICKUP_INDEX, m_handle);
                    if (!Function.Call<bool>(Natives.DOES_PICKUP_EXIST, id))
                        return ObjectCache<Pickup>.Get(id);*/
            }

            return null;
        }

        /*public void SetColorRGB(Color color)
        {

        }*/

        public static Blip AddBlip(Ped ped)
        {
            Pointer blipPtr = typeof(int);

            Function.Call(Natives.ADD_BLIP_FOR_CHAR, ped.Handle, blipPtr);

            return ObjectCache<Blip>.Get((int)blipPtr);
        }

        public static Blip AddBlip(Vector3 origin)
        {
            Pointer blipPtr = typeof(int);

            Function.Call(Natives.ADD_BLIP_FOR_COORD, origin.X, origin.Y, origin.Z, blipPtr);

            return ObjectCache<Blip>.Get((int)blipPtr);
        }

        public static Blip AddBlip(Vehicle vehicle)
        {
            Pointer blipPtr = typeof(int);

            Function.Call(Natives.ADD_BLIP_FOR_CAR, vehicle.Handle, blipPtr);

            return ObjectCache<Blip>.Get((int)blipPtr);
        }

        public static Blip AddBlip(Pickup pickup)
        {
            Pointer blipPtr = typeof(int);

            Function.Call(Natives.ADD_BLIP_FOR_PICKUP, pickup.Handle, blipPtr);

            return ObjectCache<Blip>.Get((int)blipPtr);
        }
    }
}
