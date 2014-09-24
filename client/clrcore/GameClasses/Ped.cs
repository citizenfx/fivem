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
