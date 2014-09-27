using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class World
    {
        private World() { }

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
