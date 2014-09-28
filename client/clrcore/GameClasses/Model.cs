using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class Model
    {
        private int m_hash;
        private string m_name;

        public Model(int hash)
        {
            m_hash = hash;
            m_name = String.Empty;
        }

        public Model(uint hash)
        {
            m_hash = Convert.ToInt32(hash);
            m_name = string.Empty;
        }

        public Model(string modelName)
        {
            m_name = modelName;
            m_hash = Function.Call<int>(Natives.GET_HASH_KEY, m_name);
        }

        public uint Handle
        {
            get
            {
                return Convert.ToUInt32(m_hash);
            }
        }

        public int Hash
        {
            get
            {
                return m_hash;
            }
        }

        public bool IsInMemory
        {
            get
            {
                try
                {
                    return Function.Call<bool>(Natives.HAS_MODEL_LOADED, m_hash);
                }
                catch (Exception)
                {
                    return false;
                }
            }
        }

        public bool IsCollisionDataInMemory
        {
            get
            {
                return Function.Call<bool>(Natives.HAS_COLLISION_FOR_MODEL_LOADED, m_hash);
            }
        }

        public bool IsValid
        {
            get
            {
                if (m_hash == 0) return false;
                try
                {
                    return Function.Call<bool>(Natives.HAS_MODEL_LOADED, m_hash);
                }
                catch (Exception)
                {
                    return false;
                }
            }
        }

        public void LoadToMemory()
        {
            if (m_hash == 0) return;
            try
            {
                Function.Call(Natives.REQUEST_MODEL, m_hash);
            }
            catch { }
        }

        public bool LoadToMemoryNow(int timeout)
        {
            // Dummy, don't think this'll work with Citizen straightaway
            return false;
        }

        public bool LoadToMemoryNow()
        {
            return LoadToMemoryNow(1000);
        }

        public void LoadCollisionDataToMemory()
        {
            if (m_hash == 0) return;
            Function.Call(Natives.REQUEST_COLLISION_FOR_MODEL, m_hash);
        }

        public bool LoadCollisionDataToMemoryNow(int timeout)
        {
            // Dummy, don't think this'll work with Citizen straightaway
            return false;
        }

        public void AllowDisposeFromMemory()
        {
            if (m_hash == 0) return;
            Function.Call(Natives.MARK_MODEL_AS_NO_LONGER_NEEDED, m_hash);
        }

        /*public void GetDimensions(ref Vector3 MinVector, ref Vector3 MaxVector)
        {
            Pointer pV1 = typeof(Vector3);
            Pointer pV2 = typeof(Vector3);
            Vector3 v1, v2;

            Function.Call(Natives.GET_MODEL_DIMENSIONS, pHash, v1, v1);

            v1 = (Vector3)pV1;
            v2 = (Vector3)pV2;

            MinVector.X = v1.X; MinVector.Y = v1.Y; MinVector.Z = v1.Z;
            MaxVector.X = v2.X; MaxVector.Y = v2.Y; MaxVector.Z = v2.Z;
        }

        public Vector3 GetDimensions()
        {
            Pointer pV1 = typeof(Vector3);
            Pointer pV2 = typeof(Vector3);
            Vector3 v1, v2;

            Function.Call(Natives.GET_MODEL_DIMENSIONS, pHash, pV1, pV2);

            return new Vector3(v2.X - v1.X, v2.Y - v1.Y, v2.Z - v1.Z);
        }*/

        public bool IsBike
        {
            get
            {
                if (m_hash == 0) return false;
                return Function.Call<bool>(Natives.IS_THIS_MODEL_A_BIKE, m_hash);
            }
        }

        public bool IsBoat
        {
            get
            {
                if (m_hash == 0) return false;
                return Function.Call<bool>(Natives.IS_THIS_MODEL_A_BOAT, m_hash);
            }
        }

        public bool IsCar
        {
            get
            {
                if (m_hash == 0) return false;
                return Function.Call<bool>(Natives.IS_THIS_MODEL_A_CAR, m_hash);
            }
        }

        public bool IsHelicopter
        {
            get
            {
                if (m_hash == 0) return false;
                return Function.Call<bool>(Natives.IS_THIS_MODEL_A_HELI, m_hash);
            }
        }

        public bool IsPed
        {
            get
            {
                if (m_hash == 0) return false;
                return Function.Call<bool>(Natives.IS_THIS_MODEL_A_PED, m_hash);
            }
        }

        public bool IsPlane
        {
            get
            {
                if (m_hash == 0) return false;
                return Function.Call<bool>(Natives.IS_THIS_MODEL_A_PLANE, m_hash);
            }
        }

        public bool IsTrain
        {
            get
            {
                if (m_hash == 0) return false;
                return Function.Call<bool>(Natives.IS_THIS_MODEL_A_TRAIN, m_hash);
            }
        }

        public bool IsVehicle
        {
            get
            {
                if (m_hash == 0) return false;
                return Function.Call<bool>(Natives.IS_THIS_MODEL_A_VEHICLE, m_hash);
            }
        }

        public static Model BasicCopModel
        {
            get
            {
                Pointer model = typeof(int);
                Function.Call(Natives.GET_CURRENT_BASIC_COP_MODEL, model);
                return new Model((int)model);
            }
        }

        public static Model CurrentCopModel
        {
            get
            {
                Pointer model = typeof(int);
                Function.Call(Natives.GET_CURRENT_COP_MODEL, model);
                return new Model((int)model);
            }
        }

        public static Model BasicPoliceCarModel
        {
            get
            {
                Pointer model = typeof(int);
                Function.Call(Natives.GET_CURRENT_BASIC_POLICE_CAR_MODEL, model);
                return new Model((int)model);
            }
        }

        public static Model CurrentPoliceCarModel
        {
            get
            {
                Pointer model = typeof(int);
                Function.Call(Natives.GET_CURRENT_POLICE_CAR_MODEL, model);
                return new Model((int)model);
            }
        }

        public static Model TaxiCarModel
        {
            get
            {
                Pointer model = typeof(int);
                Function.Call(Natives.GET_CURRENT_TAXI_CAR_MODEL, model);
                return new Model((int)model);
            }
        }

        public static Model GetWeaponModel(Weapon wep)
        {
            Pointer model = typeof(int);
            Function.Call(Natives.GET_WEAPONTYPE_MODEL, (int)wep, model);
            return new Model((int)model);
        }

        public static bool operator ==(Model left, Model right)
        {
            return left.Hash == right.Hash;
        }

        public static bool operator !=(Model left, Model right)
        {
            return !(left == right);
        }

        public static implicit operator Model(string source)
        {
            return new Model(source);
        }

        public static implicit operator Model(int source)
        {
            return new Model(source);
        }

        public static implicit operator Model(uint source)
        {
            return new Model(source);
        }

        public static Model FromString(string input)
        {
            if (string.IsNullOrEmpty(input))
                return new Model(0);

            if (input.StartsWith("&H", StringComparison.InvariantCultureIgnoreCase) || input.StartsWith("0x", StringComparison.InvariantCultureIgnoreCase))
            {
                if (input.Length < 3)
                    return new Model(0);

                try { return new Model(Convert.ToInt32(input.Substring(2), 16)); }
                catch { return new Model(0); }
            }

            int num = 0;
            if (int.TryParse(input, System.Globalization.NumberStyles.Integer, System.Globalization.CultureInfo.InvariantCulture, out num))
                return new Model(num);

            return new Model(input);
        }

        public override string ToString()
        {
            if (string.IsNullOrEmpty(m_name))
                return "0x" + m_hash.ToString("X8");
            else
                return m_name;
        }
    }
}
