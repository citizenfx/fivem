using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class Pickup : HandleObject
    {
        private bool m_hasExisted;

        internal override void SetHandle(int handle)
        {
            if (m_handle != -1)
                ObjectCache<Pickup>.Remove(this);

            m_handle = handle;
            m_hasExisted = false;

            ObjectCache<Pickup>.Add(this);
        }

        public Vector3 Position
        {
            get
            {
                Pointer pX = typeof(float), pY = typeof(float), pZ = typeof(float);

                Function.Call(Natives.GET_PICKUP_COORDINATES, m_handle, pX, pY, pZ);

                return new Vector3((float)pX, (float)pY, (float)pZ);
            }
        }

        public bool CollectableByCar
        {
            set
            {
                Function.Call(Natives.SET_PICKUP_COLLECTABLE_BY_CAR, m_handle, value);
            }
        }

        public bool HasBeenCollected
        {
            get
            {
                return Function.Call<bool>(Natives.HAS_PICKUP_BEEN_COLLECTED, m_handle);
            }
        }

        /*public Room CurrentRoom
        {
            get
            {
                return null
            }
            set
            {

            }
        }*/

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
                    m_hasExisted = Function.Call<bool>(Natives.DOES_PICKUP_EXIST, m_handle);

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

            Function.Call(Natives.REMOVE_PICKUP, m_handle);
        }

        public Blip AttachBlip()
        {
            return Blip.AddBlip(this);
        }

        public bool GiveToPed(Ped ped)
        {
            if (!Function.Call<bool>(Natives.DOES_CHAR_EXIST, ped.Handle))
                return false;

            Function.Call(Natives.GIVE_PED_PICKUP_OBJECT, ped.Handle, m_handle, true);

            return true;
        }

        public static Pickup CreatePickup(Vector3 position, Model model, PickupType type, Vector3 rotation)
        {
            if (!model.LoadToMemoryNow())
                return null;

            Pointer res = typeof(int);

            Function.Call(Natives.CREATE_PICKUP_ROTATE, model.Hash, (int)type, 0, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, res);
            model.AllowDisposeFromMemory();

            if ((int)res == 0)
                return null;

            return ObjectCache<Pickup>.Get((int)res);
        }

        public static Pickup CreatePickup(Vector3 position, Model model, PickupType type)
        {
            if (!model.LoadToMemoryNow())
                return null;

            Pointer res = typeof(int);

            Function.Call(Natives.CREATE_PICKUP, model.Hash, (int)type, position.X, position.Y, position.Z, res, false);
            model.AllowDisposeFromMemory();

            if ((int)res == 0)
                return null;

            return ObjectCache<Pickup>.Get((int)res);
        }

        public static Pickup CreateWeaponPickup(Vector3 position, Weapon weapon, int ammo, Vector3 rotation)
        {
            Model model = Model.GetWeaponModel(weapon);

            if (!model.LoadToMemoryNow())
                return null;

            Pointer res = typeof(int);
            Function.Call(Natives.CREATE_PICKUP_ROTATE, model.Hash, (int)PickupType.Weapon, ammo, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, res);
            model.AllowDisposeFromMemory();

            if ((int)res == 0)
                return null;

            return ObjectCache<Pickup>.Get((int)res);
        }

        public static Pickup CreateWeaponPickup(Vector3 position, Weapon weapon, int ammo)
        {
            Model model = Model.GetWeaponModel(weapon);

            if (!model.LoadToMemoryNow())
                return null;

            Pointer res = typeof(int);
            Function.Call(Natives.CREATE_PICKUP_WITH_AMMO, model.Hash, (int)PickupType.Weapon, ammo, position.X, position.Y, position.Z, res);

            if ((int)res == 0)
                return null;

            return ObjectCache<Pickup>.Get((int)res);
        }

        public static Pickup CreateMoneyPickup(Vector3 position, int moneyAmount)
        {
            Pointer res = typeof(int);

            Function.Call(Natives.CREATE_MONEY_PICKUP, position.X, position.Y, position.Z, moneyAmount, true, res);

            if ((int)res == 0)
                return null;

            return ObjectCache<Pickup>.Get((int)res);
        }
    }

    public enum PickupType
    {
        None = 0,
        Weapon = 22,
    }
}
