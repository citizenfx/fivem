using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core.client.clrcore.GameClasses
{
    public sealed class Weapon
    {
        private Ped pOwner;
        private Weapons pID;
        public Weapon(Ped Owner, Weapons ID)
        {
            pOwner = Owner;
            pID = ID;
        }

        public WeaponSlot Slot
        {
            get
            {
                if ((int)pID <= (int)Weapons.Unarmed) return WeaponSlot.Unarmed;
                Pointer slot = typeof(int);
                Function.Call(Natives.GET_WEAPONTYPE_SLOT, (int)pID, slot);
                return (WeaponSlot)(int)slot;
            }
        }

        public bool isPresent
        {
            get
            {
                if (pID == Weapons.None || pID == Weapons.Unarmed) return false;
                return Function.Call<bool>(Natives.HAS_CHAR_GOT_WEAPON, pOwner.Handle, (int)pID);
            }
        }

        public int Ammo
        {
            get
            {
                if (pID == Weapons.None) return 0;
                if (pID == Weapons.Unarmed) return 1;
                if (isPresent)
                {
                    Pointer ammo = typeof(uint);
                    ammo = 0;
                    Function.Call(Natives.GET_AMMO_IN_CHAR_WEAPON, pOwner.Handle, (int)pID, ammo);
                    return (int)ammo;
                }
                else return 0;
            }
            set
            {
                if (pID <= Weapons.Unarmed) return;
                if (isPresent) Function.Call(Natives.SET_CHAR_AMMO, (int)pID, value);
                else Function.Call(Natives.GIVE_WEAPON_TO_CHAR, pOwner.Handle, (int)pID, value, 0);
            }
        }

        public int AmmoInClip
        {
            get
            {
                if (pID == Weapons.None) return 0;
                if (pID == Weapons.Unarmed) return 1;
                if (Function.Call<bool>(Natives.HAS_CHAR_GOT_WEAPON, pOwner.Handle, (int)pID))
                {
                    Pointer ammo = typeof(uint);
                    ammo = 0;
                    Function.Call(Natives.GET_AMMO_IN_CLIP, pOwner.Handle, (int)pID, ammo);
                    return (int)ammo;
                }
                else return 0;
            }
            set
            {
                if (pID <= Weapons.Unarmed) return;
                if(!Function.Call<bool>(Natives.HAS_CHAR_GOT_WEAPON, pOwner.Handle, (int)pID)) Function.Call(Natives.GIVE_WEAPON_TO_CHAR, pOwner.Handle, (int)pID, 1, 0);
                Function.Call(Natives.SET_AMMO_IN_CLIP, pOwner.Handle, (int)pID, value);
            }
        }

        public int MaxAmmo
        {
            get
            {
                if (pID == Weapons.None) return 0;
                if (pID == Weapons.Unarmed) return 1;
                Pointer ammo = typeof(uint);
                ammo = 0;
                Function.Call(Natives.GET_MAX_AMMO, pOwner.Handle, (int)pID, ammo);
                return (int)ammo;
            }
        }

        public void Select()
        {
            if (pID == Weapons.None) return;
            if (!Function.Call<bool>(Natives.HAS_CHAR_GOT_WEAPON, pOwner.Handle, (int)pID)) Function.Call(Natives.GIVE_WEAPON_TO_CHAR, pOwner.Handle, (int)pID, 1, 0);
            Function.Call(Natives.SET_CURRENT_CHAR_WEAPON, pOwner.Handle, (int)pID, true);
        }

        public void Remove()
        {
            if (pID <= Weapons.Unarmed) return;
            Function.Call(Natives.REMOVE_WEAPON_FROM_CHAR, pOwner.Handle, (int)pID);
        }

        /*public static Weapons operator Weapon(Weapon source)
        {
            return source.pID;
        }*/

        public static implicit operator Weapons(Weapon source)
        {
            return source.pID;
        }

        public static bool operator ==(Weapon left, Weapon right)
        {
            return (left.pID == right.pID);
        }
        public static bool operator !=(Weapon left, Weapon right)
        {
            return !(left == right);
        }

        public static bool operator ==(Weapons left, Weapon right)
        {
            if (right == null) return (left == Weapons.Unarmed);
            return (left == right.pID);
        }

        public static bool operator !=(Weapons left, Weapon right)
        {
            return !(left == right);
        }

        public static bool operator ==(Weapon left, Weapons right)
        {
            if (left == null) return (right == Weapons.Unarmed);
            return (right == left.pID);
        }

        public static bool operator !=(Weapon left, Weapons right)
        {
            return !(left == right);
        }
    }
}
