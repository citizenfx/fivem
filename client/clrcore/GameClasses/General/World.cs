using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public static class World
    {
        private static int m_baseYear = 2003;

        public static Weather Weather
        {
            get
            {
                Pointer weatherPtr = typeof(int);
                Function.Call(Natives.GET_CURRENT_WEATHER, weatherPtr);

                return (Weather)(int)weatherPtr;
            }
            set
            {
                Function.Call(Natives.FORCE_WEATHER, (int)value);
            }
        }

        private static int GetDayDiff(TimeSpan time)
        {
            if (time.Ticks >= 0) 
                return time.Days;

            int days = time.Days;
            time += new TimeSpan(-days, 0, 0, 0);

            if (time.Ticks < 0)
                return (days - 1);

            return days;
        }

        private static TimeSpan GetTimeOfDay(TimeSpan time)
        {
            if (time.Ticks >= 0) 
                return new TimeSpan(time.Hours, time.Minutes, time.Seconds);

            TimeSpan tod = new TimeSpan(-time.Days + 1, 0, 0, 0);
            tod += time;

            return tod;
        }

        public static TimeSpan CurrentDayTime
        {
            get
            {
                Pointer hoursPtr = typeof(uint), minutesPtr = typeof(uint);
                Function.Call(Natives.GET_TIME_OF_DAY, hoursPtr, minutesPtr);

                return new TimeSpan((int)hoursPtr, (int)minutesPtr, 0);
            }
            set
            {
                int d = GetDayDiff(value);

                if (d > 0)
                {
                    for (int i = 0; i < d; i++)
                        Function.Call(Natives.SET_TIME_ONE_DAY_FORWARD);
                }
                else if (d < 0)
                {
                    for (int i = 0; i > d; i--)
                        Function.Call(Natives.SET_TIME_ONE_DAY_BACK);
                }

                value = GetTimeOfDay(value);
                Function.Call(Natives.SET_TIME_OF_DAY, value.Hours, value.Minutes);
            }
        }

        public static DateTime CurrentDate
        {
            get
            {
                Pointer monthPtr = typeof(uint), dayPtr = typeof(uint), hourPtr = typeof(uint), minutePtr = typeof(uint);

                Function.Call(Natives.GET_TIME_OF_DAY, hourPtr, minutePtr);
                Function.Call(Natives.GET_CURRENT_DATE, dayPtr, monthPtr);

                return new DateTime(m_baseYear, (int)monthPtr, (int)dayPtr, (int)hourPtr, (int)minutePtr, 0);
            }
        }

        public static float CarDensity
        {
            set
            {
                Function.Call(Natives.SET_CAR_DENSITY_MULTIPLIER, value);
            }
        }

        public static float PedDensity
        {
            set
            {
                Function.Call(Natives.SET_PED_DENSITY_MULTIPLIER, value);
            }
        }
        
        public static bool GravityEnabled
        {
            set
            {
                Function.Call(Natives.SET_GRAVITY_OFF, !value);
            }
        }

        public static void OneDayForward()
        {
            Function.Call(Natives.SET_TIME_ONE_DAY_FORWARD);
        }
        
        public static void LockDayTime(TimeSpan lockedTime)
        {
            if (lockedTime.Ticks < 0)
                return;

            Function.Call(Natives.FORCE_TIME_OF_DAY, lockedTime.Hours, lockedTime.Minutes);
        }

        public static void LockDayTime(int hour, int minute)
        {
            if (hour < 0 || hour > 23)
                return;

            if (minute < 0 || minute > 59)
                return;

            Function.Call(Natives.FORCE_TIME_OF_DAY, hour, minute);
        }

        public static void LockDayTime()
        {
            Pointer weatherPtr = typeof(int);
            Function.Call(Natives.GET_CURRENT_WEATHER, weatherPtr);

            LockDayTime(CurrentDayTime);
            Function.Call(Natives.FORCE_WEATHER_NOW, (int)weatherPtr);
        }

        public static void UnlockDayTime()
        {
            Function.Call(Natives.RELEASE_TIME_OF_DAY);
        }

        public static void LoadEnvironmentNow(Vector3 position)
        {
            Function.Call(Natives.REQUEST_COLLISION_AT_POSN, position.X, position.Y, position.Z);
            Function.Call(Natives.LOAD_ALL_OBJECTS_NOW);
            Function.Call(Natives.LOAD_SCENE, position.X, position.Y, position.Z);// WARNING: causes a call to OnRender and thus a Deadlock!
            Function.Call(Natives.POPULATE_NOW);
        }

        public static void AddExplosion(Vector3 position, ExplosionType type, float power, bool playSound, bool noVisuals, float cameraShake)
        {
            Function.Call(Natives.ADD_EXPLOSION, position.X, position.Y, position.Z, (uint)type, playSound, noVisuals, cameraShake);
        }

        public static void AddExplosion(Vector3 position, ExplosionType type, float power)
        {
            AddExplosion(position, type, power, true, false, 1.0f);
        }

        public static void AddExplosion(Vector3 position)
        {
            AddExplosion(position, ExplosionType.Default, 1.0f, true, false, 1.0f);
        }

        //internal static void DrawCheckpoint(Vector3 position, float diameter, Color color){ }
        //internal static void DrawLight(Vector3 position, Color color, float range, float intensity) { }

        //some leftover (internal) functions
        //GetValidPedHandles
        //GetValidVehicleHandles
        //GetValidObjectHandles

        public static Ped GetClosestPed(Vector3 position, float radius)
        {
            Pointer pedPtr = typeof(int);

            Function.Call(Natives.BEGIN_CHAR_SEARCH_CRITERIA);
            Function.Call(Natives.END_CHAR_SEARCH_CRITERIA);
            Function.Call(Natives.GET_CLOSEST_CHAR, position.X, position.Y, position.Z, radius, true, true, pedPtr);

            if ((int)pedPtr == 0)
                return null;

            return ObjectCache<Ped>.Get((int)pedPtr);
        }

        public static Ped GetRandomPed(Vector3 position, float radius)
        {
            Pointer pedPtr = typeof(int);

            Function.Call(Natives.BEGIN_CHAR_SEARCH_CRITERIA);
            Function.Call(Natives.END_CHAR_SEARCH_CRITERIA);
            Function.Call(Natives.GET_RANDOM_CHAR_IN_AREA_OFFSET_NO_SAVE, position.X - radius, position.Y - radius, position.Z - radius, radius * 2.0f, radius * 2.0f, radius * 2.0f, pedPtr);

            if ((int)pedPtr == 0)
                return null;

            return ObjectCache<Ped>.Get((int)pedPtr);
        }

        //some more leftover functions
        //GetPeds
        //GetAllPeds
        //GetVehicles
        //GetAllVehicles
        //GetClosestVehicle
        //GetAllObjects

        internal static float GetGroundZBelow(float x, float y, float z)
        {
            Pointer zPtr = typeof(float);
            Function.Call(Natives.GET_GROUND_Z_FOR_3D_COORD, x, y, z, zPtr);

            return (float)zPtr;
        }

        internal static float GetGroundZAbove(float x, float y, float z)
        {
            if (z < 0.0f)
                z = 0.0f;

            float lastZ, resZ;

            for (int i = 0; i <= 10; i++)
            {
                lastZ = z + (float)Math.Pow(2.0, (double)i);
                resZ = GetGroundZBelow(x, y, lastZ);

                if ((resZ < lastZ) && (resZ > z))
                    return resZ;
            }
            return z;
        }

        internal static float GetGroundZNext(float x, float y, float z)
        {
            if (z < 0.0f)
                z = 0.0f;

            float lastZ, resZ;

            for (int i = 0; i <= 10; i++)
            {
                lastZ = z + (float)Math.Pow(2.0, (double)i);
                resZ = GetGroundZBelow(x, y, lastZ);

                if ((resZ < lastZ) && (resZ > 0.0f))
                    return resZ;
            }

            return z;
        }

        public static float GetGroundZ(Vector3 position, GroundType type)
        {
            switch (type)
            {
                case GroundType.Highest:
                    return GetGroundZBelow(position.X, position.Y, 1024.0f);
                case GroundType.Lowest:
                    return GetGroundZAbove(position.X, position.Y, 0.0f);
                case GroundType.NextBelowCurrent:
                    return GetGroundZBelow(position.X, position.Y, position.Z);
                case GroundType.NextAboveCurrent:
                    return GetGroundZAbove(position.X, position.Y, position.Z);
                default:
                    return GetGroundZNext(position.X, position.Y, position.Z);
            }
        }

        public static float GetGroundZ(Vector3 position)
        {
            return GetGroundZ(position, GroundType.Closest);
        }

        public static Vector3 GetGroundPosition(Vector3 position, GroundType type)
        {
            return new Vector3(position.X, position.Y, GetGroundZ(position, type));
        }

        public static Vector3 GetGroundPosition(Vector3 position)
        {
            return GetGroundPosition(position, GroundType.Closest);
        }

        public static Vector3 GetNextPositionOnPavement(Vector3 position)
        {
            Pointer xPtr = typeof(float), yPtr = typeof(float), zPtr = typeof(float);

            Function.Call(Natives.GET_SAFE_POSITION_FOR_CHAR, position.X, position.Y, position.Z, true, xPtr, yPtr, zPtr);

            return new Vector3((float)xPtr, (float)yPtr, (float)zPtr);
        }

        public static Vector3 GetNextPositionOnStreet(Vector3 position)
        {
            uint inarea = Function.Call<uint>(Natives.GET_MAP_AREA_FROM_COORDS, position.X, position.Y, position.Z);
            Pointer xPtr = typeof(float), yPtr = typeof(float), zPtr = typeof(float), hPtr = typeof(float);
            Pointer outareaPtr = typeof(float);

            Function.Call(Natives.GET_NTH_CLOSEST_CAR_NODE_WITH_HEADING_ON_ISLAND, position.X, position.Y, position.Z, 1, inarea, xPtr, yPtr, zPtr, hPtr, outareaPtr);

            return new Vector3((float)xPtr, (float)yPtr, (float)zPtr);
        }

        public static Vector3 GetPositionAround(Vector3 position, float distance)
        {
            return position + RandomXY() * distance;
        }

        private static Vector3 RandomXY()
        {
            Random rnd = new Random();
            Vector3 vec = new Vector3((float)(rnd.NextDouble() - 0.5), (float)(rnd.NextDouble() - 0.5), 0.0f);

            vec.Normalize();

            return vec;
        }

        /*public static string GetStreetName(Vector3 position)
        {
            Pointer strHashPtr0 = typeof(uint), strHashPtr1 = typeof(uint);

            Function.Call(Natives.FIND_STREET_NAME_AT_POSITION, position.X, position.Y, position.Z, strHashPtr0, strHashPtr1);

            string str1 = Function.Call<string>(Natives.GET_STRING_FROM_HASH_KEY, (string)strHashPtr0).Trim();
            string str2 = Function.Call<string>(Natives.GET_STRING_FROM_HASH_KEY, (string)strHashPtr1).Trim();

            if (str1.Length == 0)
                return str2;

            if (str2.Length == 0)
                return str1;

            return string.Format("{0}, {1}", str1, str2);
        }*/

        public static string GetZoneName(Vector3 position)
        {
            return Function.Call<string>(Natives.GET_NAME_OF_INFO_ZONE, position.X, position.Y, position.Z);
        }

        public static void SetGroupRelationShip(RelationshipGroup group, Relationship level, RelationshipGroup targetGroup)
        {
            Function.Call(Natives.ALLOW_GANG_RELATIONSHIPS_TO_BE_CHANGED_BY_NEXT_COMMAND, true);
            Function.Call(Natives.SET_RELATIONSHIP, (int)level, (int)group, (int)targetGroup);
        }

        public static ScriptedFire StartFire(Vector3 position, int unknown1, int unknown2)
        {
            int fire = Function.Call<int>(Natives.START_SCRIPT_FIRE, position.X, position.Y, position.Z, unknown1, unknown2);
            if (fire == 0)
                return null;

            return ObjectCache<ScriptedFire>.Get(fire);
        }

        public static ScriptedFire StartFire(Vector3 position)
        {
            return StartFire(position, 1, 1);
        }

        public static void ExtinguishFire(Vector3 position, float radius)
        {
            Function.Call(Natives.EXTINGUISH_FIRE_AT_POINT, position.X, position.Y, position.Z, radius);
        }

        public static void ExtinguishAllScriptFires()
        {
            Function.Call(Natives.REMOVE_ALL_SCRIPT_FIRES);
        }

        public static Ped CreatePed(Vector3 position, Gender gender)
        {
            Pointer pedPtr = typeof(int);
            if (gender == Gender.Male)
                Function.Call(Natives.CREATE_RANDOM_MALE_CHAR, position.X, position.Y, position.Z, pedPtr);
            else
                Function.Call(Natives.CREATE_RANDOM_FEMALE_CHAR, position.X, position.Y, position.Z, pedPtr);

            if ((int)pedPtr == 0)
                return null;

            return ObjectCache<Ped>.Get((int)pedPtr);
        }

        public static Ped CreatePed(Vector3 position)
        {
            Pointer pedPtr = typeof(int);
            Function.Call(Natives.CREATE_RANDOM_CHAR, position.X, position.Y, position.Z, pedPtr);

            if ((int)pedPtr == 0)
                return null;

            return ObjectCache<Ped>.Get((int)pedPtr);
        }

        public static Ped CreatePed(Model model, Vector3 position, RelationshipGroup type)
        {
            if (!model.IsPed)
                return null;

            if (!model.LoadToMemoryNow())
                return null;

            Pointer pedPtr = typeof(int);
            Function.Call(Natives.CREATE_CHAR, (int)type, model.Hash, position.X, position.Y, position.Z, pedPtr, true);
            model.AllowDisposeFromMemory();

            if ((int)pedPtr == 0)
                return null;

            return ObjectCache<Ped>.Get((int)pedPtr);
        }

        public static Ped CreatePed(Model model, Vector3 position)
        {
            Ped ped = CreatePed(model, position, RelationshipGroup.Civillian_Male);

            if (ped == null)
                return null;

            if (ped.Gender == Gender.Female)
                ped.RelationshipGroup = RelationshipGroup.Civillian_Female;

            return ped;
        }

        public static Vehicle CreateVehicle(Vector3 position)
        {
            Pointer modelPtr = typeof(int), unknownPtr = typeof(int), carPtr = typeof(int);

            Function.Call(Natives.GET_RANDOM_CAR_MODEL_IN_MEMORY, true, modelPtr, unknownPtr);
            Function.Call(Natives.CREATE_CAR, (int)modelPtr, position.X, position.Y, position.Z, carPtr, true);
            
            if ((int)carPtr == 0)
                return null;

            return ObjectCache<Vehicle>.Get((int)carPtr);
        }

        public static Vehicle CreateVehicle(Model model, Vector3 position)
        {
            if (!model.IsVehicle)
                return null;

            if (!model.LoadToMemoryNow())
                return null;

            Pointer carPtr = typeof(int);
            Function.Call(Natives.CREATE_CAR, model.Handle, position.X, position.Y, position.Z, carPtr, true);
            model.AllowDisposeFromMemory();

            if ((int)carPtr == 0)
                return null;

            return ObjectCache<Vehicle>.Get((int)carPtr);
        }
    }
}
