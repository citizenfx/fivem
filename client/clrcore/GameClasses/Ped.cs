using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class Ped : Entity
    {
        private bool m_hasExisted;

        internal override void SetHandle(int handle)
        {
            if (m_handle != -1)
            {
                ObjectCache<Ped>.Remove(this);
            }

            m_handle = handle;
            m_hasExisted = false;

            ObjectCache<Ped>.Add(this);
        }

        // get_Model

        public Vector3 Velocity
        {
            get
            {
                Pointer pX = typeof(float), pY = typeof(float), pZ = typeof(float);
                
                Function.Call(Natives.GET_CHAR_VELOCITY, m_handle, pX, pY, pZ);

                return new Vector3((float)pX, (float)pY, (float)pZ);
            }
            set
            {
                Function.Call(Natives.SET_CHAR_VELOCITY, m_handle, value.X, value.Y, value.Z);
            }
        }

        public Gender Gender
        {
            get
            {
                return (Function.Call<bool>(Natives.IS_CHAR_MALE, m_handle)) ? Gender.Male : Gender.Female;
            }
        }

        public string Voice
        {
            set
            {
                if (string.IsNullOrWhiteSpace(value) || value.Equals("default", StringComparison.InvariantCultureIgnoreCase))
                {
                    //SetDefaultVoice();
                    return;
                }

                Function.Call(Natives.SET_AMBIENT_VOICE_NAME, m_handle, value);
            }
        }

        public int Health
        {
            get
            {
                Pointer pHealth = typeof(int);

                Function.Call(Natives.GET_CHAR_HEALTH, m_handle, pHealth);

                return (int)pHealth - 100;
            }
            set
            {
                Function.Call(Natives.SET_CHAR_HEALTH, m_handle, value + 100);
            }
        }

        // MaxHealth?

        public int Armor
        {
            get
            {
                Pointer pArmor = typeof(int);

                Function.Call(Natives.GET_CHAR_ARMOUR, m_handle, pArmor);

                return (int)pArmor;
            }
            set
            {
                Function.Call(Natives.ADD_ARMOUR_TO_CHAR, m_handle, value - Armor);
            }
        }

        public float HeightAboveGround
        {
            get
            {
                Pointer pValue = typeof(float);

                Function.Call(Natives.GET_CHAR_HEIGHT_ABOVE_GROUND, m_handle, pValue);

                return (float)pValue;
            }
        }

        public bool IsOnFire
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CHAR_ON_FIRE, m_handle);
            }
            set
            {
                if (value)
                {
                    int fire = Function.Call<int>(Natives.START_CHAR_FIRE, m_handle);

                    // ?
                }
                else
                {
                    Function.Call(Natives.EXTINGUISH_CHAR_FIRE, m_handle);
                }
            }
        }

        public bool IsRequiredForMission
        {
            get
            {
                return Function.Call<bool>(Natives.IS_PED_A_MISSION_PED, m_handle);
            }
            set
            {
                if (value)
                {
                    Function.Call(Natives.SET_CHAR_AS_MISSION_CHAR, m_handle);
                }
                else
                {
                    NoLongerNeeded();
                }
            }
        }

        public bool IsAlive
        {
            get
            {
                return !IsDead;
            }
        }

        public bool IsAliveAndWell
        {
            get
            {
                return Exists && IsAlive && !IsInjured;
            }
        }

        public bool IsDead
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CHAR_DEAD, m_handle);
            }
        }

        public bool IsGettingIntoAVehicle
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CHAR_GETTING_IN_TO_A_CAR, m_handle);
            }
        }

        public bool IsGettingUp
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CHAR_GETTING_UP, m_handle);
            }
        }

        public bool IsInAir
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CHAR_IN_AIR, m_handle);
            }
        }

        public bool IsInCombat
        {
            get
            {
                return Function.Call<bool>(Natives.IS_PED_IN_COMBAT, m_handle);
            }
        }

        public bool IsInGroup
        {
            get
            {
                return Function.Call<bool>(Natives.IS_PED_IN_GROUP, m_handle);
            }
        }

        public bool IsInjured
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CHAR_INJURED, m_handle);
            }
        }

        public bool IsInMeleeCombat
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CHAR_IN_MELEE_COMBAT, m_handle);
            }
        }

        public bool IsInWater
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CHAR_IN_WATER, m_handle);
            }
        }

        public bool IsMissionCharacter
        {
            get
            {
                return Function.Call<bool>(Natives.IS_PED_A_MISSION_PED, m_handle);
            }
        }

        public bool IsRagdoll
        {
            get
            {
                return Function.Call<bool>(Natives.IS_PED_RAGDOLL, m_handle);
            }
            set
            {
                if (value)
                {
                    PreventRagdoll = false;
                    Function.Call(Natives.SWITCH_PED_TO_RAGDOLL, m_handle, 10000, -1, 0, 1, 1, 0);
                }
                else
                {
                    Function.Call(Natives.SWITCH_PED_TO_ANIMATED, m_handle, false);
                }
            }
        }

        public bool IsOnScreen
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CHAR_ON_SCREEN, m_handle);
            }
        }

        public bool IsShooting
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CHAR_SHOOTING, m_handle);
            }
        }

        public bool IsSwimming
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CHAR_SWIMMING, m_handle);
            }
        }

        public bool Invincible
        {
            set
            {
                Function.Call(Natives.SET_CHAR_INVINCIBLE, m_handle, value);
            }
        }

        public bool Visible
        {
            set
            {
                Function.Call(Natives.SET_CHAR_VISIBLE, m_handle, value);
            }
        }

        // CurrentRoom

        public bool FreezePosition
        {
            set
            {
                Function.Call(Natives.FREEZE_CHAR_POSITION, m_handle, value);
            }
        }

        public Vehicle CurrentVehicle
        {
            get
            {
                Pointer pCar = typeof(int);
                Function.Call(Natives.GET_CAR_CHAR_IS_USING, m_handle, pCar);

                int car = (int)pCar;

                if (car == 0)
                {
                    return null;
                }

                return ObjectCache<Vehicle>.Get(car);
            }
        }

        public Blip AttachBlip()
        {
            return Blip.AddBlip(this);
        }

        public void AttachTo(Vehicle vehicle, Vector3 offset)
        {
            Function.Call(Natives.ATTACH_PED_TO_CAR, m_handle, vehicle.Handle, 0, offset.X, offset.Y, offset.Z, 0.0f, 0.0f, 0, 0);
        }

        public void Detach()
        {
            Function.Call(Natives.DETACH_PED, m_handle);
        }

        public bool IsAttachedToVehicle
        {
            get
            {
                return Function.Call<bool>(Natives.IS_PED_ATTACHED_TO_ANY_CAR, m_handle);
            }
        }

        public void ApplyForce(Vector3 direction, Vector3 rotation)
        {
            Function.Call(Natives.APPLY_FORCE_TO_PED, m_handle, 3, direction.X, direction.Y, direction.Z, rotation.X, rotation.Y, rotation.Z, 0, 0, 1, 1);
        }

        public void ApplyForce(Vector3 direction)
        {
            ApplyForce(direction, Vector3.Zero);
        }

        public void ApplyForceRelative(Vector3 direction, Vector3 rotation)
        {
            Function.Call(Natives.APPLY_FORCE_TO_PED, m_handle, 3, direction.X, direction.Y, direction.Z, rotation.X, rotation.Y, rotation.Z, 0, true, 1, 1);
        }

        public void ApplyForceRelative(Vector3 direction)
        {
            ApplyForceRelative(direction, Vector3.Zero);
        }

        public int Accuracy
        {
            set
            {
                Function.Call(Natives.SET_CHAR_ACCURACY, m_handle, value);
            }
        }

        public bool DiesWhenInjured
        {
            set
            {
                Function.Call(Natives.SET_CHAR_WILL_MOVE_WHEN_INJURED, m_handle, !value);
                Function.Call(Natives.SET_PED_DIES_WHEN_INJURED, m_handle, value);
            }
        }

        public bool BlockPermanentEvents
        {
            set
            {
                Function.Call(Natives.SET_BLOCKING_OF_NON_TEMPORARY_EVENTS, m_handle, value);
            }
        }

        public bool BlockWeaponSwitching
        {
            set
            {
                Function.Call(Natives.BLOCK_PED_WEAPON_SWITCHING, m_handle, value);
            }
        }

        public bool BlockGestures
        {
            set
            {
                Function.Call(Natives.BLOCK_CHAR_GESTURE_ANIMS, m_handle, value);
            }
        }

        public bool CanBeDraggedOutOfVehicle
        {
            set
            {
                Function.Call(Natives.SET_CHAR_CANT_BE_DRAGGED_OUT, m_handle, !value);
            }
        }

        public bool CanBeKnockedOffBike
        {
            set
            {
                Function.Call(Natives.SET_CHAR_CAN_BE_KNOCKED_OFF_BIKE, m_handle, value);
            }
        }

        public bool CowerInsteadOfFleeing
        {
            set
            {
                Function.Call(Natives.SET_CHAR_WILL_COWER_INSTEAD_OF_FLEEING, m_handle, value);
            }
        }

        public bool PreventRagdoll
        {
            set
            {
                Function.Call(Natives.UNLOCK_RAGDOLL, m_handle, !value);
            }
        }

        public bool IsEnemy
        {
            set
            {
                Function.Call(Natives.SET_CHAR_AS_ENEMY, m_handle, value);
            }
        }

        public void NoLongerNeeded()
        {
            Pointer pedPtr = m_handle;
            Function.Call(Natives.MARK_CHAR_AS_NO_LONGER_NEEDED, pedPtr);
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
                    m_hasExisted = Function.Call<bool>(Natives.DOES_CHAR_EXIST, m_handle);

                    return m_hasExisted;
                }
                catch
                {
                    return false;
                }
            }
        }

        protected override uint GetCoordinatesFunction
        {
            get
            {
                return Natives.GET_CHAR_COORDINATES;
            }
        }

        protected override uint SetCoordinatesFunction
        {
            get
            {
                return Natives.SET_CHAR_COORDINATES;
            }
        }

        protected override uint GetHeadingFunction
        {
            get { return Natives.GET_CHAR_HEADING; }
        }

        protected override uint SetHeadingFunction
        {
            get { return Natives.SET_CHAR_HEADING; }
        }
    }

    public enum Gender
    {
        Male,
        Female
    }
}
