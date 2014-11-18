using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class Vehicle : Entity
    {
        private bool m_hasExisted;
        private Dictionary<VehicleDoors, VehicleDoor> m_doorCache;
        private Dictionary<int, VehicleExtra> m_extraCache;

        internal override void SetHandle(int handle)
        {
            if (m_handle != -1)
            {
                ObjectCache<Vehicle>.Remove(this);
            }

            m_handle = handle;
            m_hasExisted = false;

            ObjectCache<Vehicle>.Add(this);
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
                    m_hasExisted = Function.Call<bool>(Natives.DOES_VEHICLE_EXIST, m_handle);

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
            get { return Natives.GET_CAR_COORDINATES; }
        }

        protected override uint SetCoordinatesFunction
        {
            get { return Natives.SET_CAR_COORDINATES; }
        }

        protected override uint GetHeadingFunction
        {
            get { return Natives.GET_CAR_HEADING; }
        }

        protected override uint SetHeadingFunction
        {
            get { return Natives.SET_CAR_HEADING; }
        }

        public Vector3 Direction
        {
            get
            {
                return HeadingToDirection(Heading);
            }
        }

        private Vector3 HeadingToDirection(float heading)
        {
            heading = MathUtil.DegreesToRadians(heading);
		    return new Vector3((float)-Math.Sin(heading), (float)Math.Cos(heading), 0.0f);
        }

        public Vector3 Rotation
        {
            get
            {
                return RotationQuaternion.ToRotation();
            }
            set
            {
                RotationQuaternion = QuaternionExtensions.FromRotation(value);
            }
        }

        public Quaternion RotationQuaternion
        {
            get
            {
                Pointer xPtr = typeof(float), yPtr = typeof(float), zPtr = typeof(float), wPtr = typeof(float);
                Function.Call(Natives.GET_VEHICLE_QUATERNION, m_handle, xPtr, yPtr, zPtr, wPtr);

                return new Quaternion((float)xPtr, (float)yPtr, (float)zPtr, (float)wPtr);
            }
            set
            {
                Function.Call(Natives.SET_VEHICLE_QUATERNION, m_handle, value.X, value.Y, value.Z, value.W);
            }
        }

        public Room CurrentRoom
        {
            get
            {
                Pointer ii = typeof(int), rk = typeof(uint);
                Function.Call(Natives.GET_INTERIOR_FROM_CAR, m_handle, ii);
                Function.Call(Natives.GET_KEY_FOR_CAR_IN_ROOM, m_handle, rk);
                return new Room((int)rk, (int)ii);
            }
            set
            {
                Function.Call(Natives.SET_ROOM_FOR_CAR_BY_KEY, m_handle, (int)value.InteriorID);
            }
        }

        //needs some unsafe code apparently
        /*public float CurrentRPM
        {
            get
            {

            }
        }*/

        public Model Model
        {
            get
            {
                Pointer modelPtr = typeof(int);
                Function.Call(Natives.GET_CAR_MODEL, m_handle, modelPtr);

                return new Model((int)modelPtr);
            }
        }

        public bool Visible
        {
            set
            {
                Function.Call(Natives.SET_CAR_VISIBLE, m_handle, value);
            }
        }

        public bool IsOnFire
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CAR_ON_FIRE, m_handle);
            }
            set
            {
                if (value)
                {
                    int fire = Function.Call<int>(Natives.START_CAR_FIRE, m_handle);
                    if (fire == 0)
                        return;

                    //not sure about this one
                    ObjectCache<ScriptedFire>.Get(fire);
                }
                else
                {
                    Function.Call(Natives.EXTINGUISH_CAR_FIRE, m_handle);
                }
            }
        }

        public bool IsRequiredForMission
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CAR_A_MISSION_CAR, m_handle);
            }
            set
            {
                if (value)
                {
                    Function.Call(Natives.SET_CAR_AS_MISSION_CAR, m_handle);
                }
                else
                {
                    NoLongerNeeded();
                }
            }
        }

        public bool FreezePosition
        {
            set
            {
                Function.Call(Natives.FREEZE_CAR_POSITION, m_handle, value);
            }
        }

        public Vector3 Velocity
        {
            get
            {
                return Direction * Speed;
            }
            set
            {
                //ApplyForce(value - Velocity);
            }
        }

        //these probably belong in Entity -> protected abstract uint ApplyForceFunction { get; }
        //ApplyForce
        //ApplyForceRelative

        public bool AllowSirenWithoutDriver
        {
            set
            {
                Function.Call(Natives.SET_SIREN_WITH_NO_DRIVER, m_handle, true);
            }
        }

        public bool CanBeDamaged
        {
            set
            {
                Function.Call(Natives.SET_CAR_CAN_BE_DAMAGED, m_handle, value);
            }
        }

        public bool CanBeVisiblyDamaged
        {
            set
            {
                Function.Call(Natives.SET_CAR_CAN_BE_VISIBLY_DAMAGED, m_handle, value);
            }
        }

        public bool CanTiresBust
        {
            set
            {
                Function.Call(Natives.SET_CAN_BURST_CAR_TYRES, m_handle, value);
            }
        }

        public float Dirtyness
        {
            get
            {
                Pointer dirtPtr = typeof(float);
                Function.Call(Natives.GET_VEHICLE_DIRT_LEVEL, m_handle, dirtPtr);

                return (float)dirtPtr;
            }
            set
            {
                Function.Call(Natives.SET_VEHICLE_DIRT_LEVEL, m_handle, value);
            }
        }

        public VehicleDoorLock DoorLock
        {
            get
            {
                Pointer statusPtr = typeof(int);
                Function.Call(Natives.GET_CAR_DOOR_LOCK_STATUS, m_handle, statusPtr);

                return (VehicleDoorLock)(int)statusPtr;
            }
            set
            {
                Function.Call(Natives.LOCK_CAR_DOORS, m_handle, (uint)value);
            }
        }

        public float EngineHealth
        {
            get
            {
                return Function.Call<float>(Natives.GET_ENGINE_HEALTH, m_handle); //yes, this function actually returns float!
            }
            set
            {
                Function.Call(Natives.SET_ENGINE_HEALTH, m_handle, value);

                if (value <= 0.0f)
                    Function.Call(Natives.SET_CAR_ENGINE_ON, m_handle, false, true);
            }
        }

        public bool EngineRunning
        {
            /*get
            {
                return CurrentRPM > 0.0f;
            }*/
            set
            {
                Function.Call(Natives.SET_CAR_ENGINE_ON, m_handle, value, false);
            }
        }

        public bool HazardLightsOn
        {
            set
            {
                Function.Call(Natives.SET_VEH_HAZARDLIGHTS, m_handle, value);
            }
        }

        public bool InteriorLightOn
        {
            set
            {
                Function.Call(Natives.SET_VEH_INTERIORLIGHT, m_handle, value);
            }
        }

        public int Health
        {
            get
            {
                Pointer healthPtr = typeof(uint);
                Function.Call(Natives.GET_CAR_HEALTH, m_handle, healthPtr);

                return (int)healthPtr;
            }
            set
            {
                Function.Call(Natives.SET_CAR_HEALTH, m_handle, value);
            }
        }

        public bool IsDriveable
        {
            get
            {
                return Function.Call<bool>(Natives.IS_VEH_DRIVEABLE, m_handle);
            }
        }

        public bool IsOnAllWheels
        {
            get
            {
                return Function.Call<bool>(Natives.IS_VEHICLE_ON_ALL_WHEELS, m_handle);
            }
        }

        public bool IsOnScreen
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CAR_ON_SCREEN, m_handle);
            }
        }

        public bool IsAlive
        {
            get
            {
                return !Function.Call<bool>(Natives.IS_CAR_DEAD, m_handle);
            }
        }

        public bool IsUpright
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CAR_UPRIGHT, m_handle);
            }
        }

        public bool IsUpsideDown
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CAR_UPSIDEDOWN, m_handle);
            }
        }

        public string Name
        {
            get
            {
                Pointer modelPtr = typeof(int);
                Function.Call(Natives.GET_CAR_MODEL, m_handle, modelPtr);

                return Function.Call<string>(Natives.GET_DISPLAY_NAME_FROM_VEHICLE_MODEL, (int)modelPtr);
            }
        }

        //requires some unsafe code again
        /*public bool LightsOn
        {
            get
            {

            }
        }*/

        public bool NeedsToBeHotwired
        {
            set
            {
                Function.Call(Natives.SET_NEEDS_TO_BE_HOTWIRED, m_handle, value);
            }
        }

        public int PassengerSeats
        {
            get
            {
                Pointer countPtr = typeof(uint);
                Function.Call(Natives.GET_MAXIMUM_NUMBER_OF_PASSENGERS, m_handle, countPtr);

                return (int)countPtr;
            }
        }

        public float PetrolTankHealth
        {
            get
            {
                return Function.Call<float>(Natives.GET_PETROL_TANK_HEALTH, m_handle);
            }
            set
            {
                Function.Call(Natives.SET_PETROL_TANK_HEALTH, m_handle, value);
            }
        }

        public bool PreviouslyOwnedByPlayer
        {
            set
            {
                Function.Call(Natives.SET_HAS_BEEN_OWNED_BY_PLAYER, m_handle, value);
            }
        }

        public bool SirenActive
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CAR_SIREN_ON, m_handle);
            }
            set
            {
                Function.Call(Natives.SWITCH_CAR_SIREN, m_handle, value);
            }
        }

        public float Speed
        {
            get
            {
                Pointer speedPtr = typeof(float);
                Function.Call(Natives.GET_CAR_SPEED, m_handle, speedPtr);

                return (float)speedPtr;
            }
            set
            {
                if (Model.IsTrain)
                {
                    Function.Call(Natives.SET_TRAIN_SPEED, m_handle, value);
                    Function.Call(Natives.SET_TRAIN_CRUISE_SPEED, m_handle, value);
                }
                else
                {
                    Function.Call(Natives.SET_CAR_FORWARD_SPEED, m_handle, value);
                }
            }
        }

        public ColorIndex Color
        {
            get
            {
                Pointer c1Ptr = typeof(int), c2Ptr = typeof(int);
                Function.Call(Natives.GET_CAR_COLOURS, m_handle, c1Ptr, c2Ptr);

                return new ColorIndex((int)c1Ptr);
            }
            set
            {
                Pointer c1Ptr = typeof(int), c2Ptr = typeof(int);

                Function.Call(Natives.GET_CAR_COLOURS, m_handle, c1Ptr, c2Ptr);
                Function.Call(Natives.CHANGE_CAR_COLOUR, m_handle, value.Index, (int)c2Ptr);
            }
        }

        public ColorIndex SpecularColor
        {
            get
            {
                Pointer c1Ptr = typeof(int), c2Ptr = typeof(int);
                Function.Call(Natives.GET_EXTRA_CAR_COLOURS, m_handle, c1Ptr, c2Ptr);

                return new ColorIndex((int)c1Ptr);
            }
            set
            {
                Pointer c1Ptr = typeof(int), c2Ptr = typeof(int);

                Function.Call(Natives.GET_EXTRA_CAR_COLOURS, m_handle, c1Ptr, c2Ptr);
                Function.Call(Natives.SET_EXTRA_CAR_COLOURS, m_handle, value.Index, (int)c2Ptr);
            }
        }

        public ColorIndex FeatureColor1
        {
            get
            {
                Pointer c1Ptr = typeof(int), c2Ptr = typeof(int);
                Function.Call(Natives.GET_CAR_COLOURS, m_handle, c1Ptr, c2Ptr);

                return new ColorIndex((int)c2Ptr);
            }
            set
            {
                Pointer c1Ptr = typeof(int), c2Ptr = typeof(int);

                Function.Call(Natives.GET_CAR_COLOURS, m_handle, c1Ptr, c2Ptr);
                Function.Call(Natives.CHANGE_CAR_COLOUR, m_handle, (int)c1Ptr, value.Index);
            }
        }

        public ColorIndex FeatureColor2
        {
            get
            {
                Pointer c1Ptr = typeof(int), c2Ptr = typeof(int);
                Function.Call(Natives.GET_EXTRA_CAR_COLOURS, m_handle, c1Ptr, c2Ptr);

                return new ColorIndex((int)c2Ptr);
            }
            set
            {
                Pointer c1Ptr = typeof(int), c2Ptr = typeof(int);

                Function.Call(Natives.GET_EXTRA_CAR_COLOURS, m_handle, c1Ptr, c2Ptr);
                Function.Call(Natives.SET_EXTRA_CAR_COLOURS, m_handle, (int)c1Ptr, value.Index);
            }
        }

        public Blip AttachBlip()
        {
            return Blip.AddBlip(this);
        }

        public VehicleDoor GetDoor(VehicleDoors door)
        {
            VehicleDoor res;

		    if (m_doorCache == null) 
            {
			    m_doorCache = new Dictionary<VehicleDoors, VehicleDoor>();
		    } 
            else 
            {
			    if (m_doorCache.TryGetValue(door, out res)) 
                    return res;
		    }

		    res = new VehicleDoor(this, door);
		    m_doorCache.Add(door, res);

		    return res;
        }

        public VehicleExtra GetExtra(int extraId)
        {
            VehicleExtra res;

            if (m_extraCache == null)
            {
                m_extraCache = new Dictionary<int, VehicleExtra>();
            }
            else
            {
                if (m_extraCache.TryGetValue(extraId, out res))
                    return res;
            }

		    res = new VehicleExtra(this, extraId);
		    m_extraCache.Add(extraId, res);

		    return res;
        }

        public Ped CreatePedOnSeat(VehicleSeat seat)
        {
            if (seat <= VehicleSeat.None)
                return null;

            if (!IsSeatFree(seat))
                return null;

            Pointer pedPtr = typeof(int);

            if (seat == VehicleSeat.Driver)
                Function.Call(Natives.CREATE_RANDOM_CHAR_AS_DRIVER, m_handle, pedPtr);
            else
                Function.Call(Natives.CREATE_RANDOM_CHAR_AS_PASSENGER, m_handle, (int)seat, pedPtr);

            if ((int)pedPtr == 0)
                return null;

            return ObjectCache<Ped>.Get((int)pedPtr);
        }

        public async Task<Ped> CreatePedOnSeat(VehicleSeat seat, Model model, RelationshipGroup type)
        {
            if (seat <= VehicleSeat.None)
                return null;

            if (!IsSeatFree(seat))
                return null;

            await model.LoadToMemoryNow();
            Pointer pedPtr = typeof(int);

            if (seat == VehicleSeat.Driver)
                Function.Call(Natives.CREATE_CHAR_INSIDE_CAR, m_handle, (int)type, model.Hash, pedPtr);
            else
                Function.Call(Natives.CREATE_CHAR_AS_PASSENGER, m_handle, (int)type, model.Hash, (int)seat, pedPtr);

            model.AllowDisposeFromMemory();

            if ((int)pedPtr == 0)
                return null;

            return ObjectCache<Ped>.Get((int)pedPtr);
        }

        public async Task<Ped> CreatePedOnSeat(VehicleSeat seat, Model model)
        {
            Ped ped = await CreatePedOnSeat(seat, model, RelationshipGroup.Civillian_Male);

            if (ped == null)
                return null;

            if (ped.Gender == Gender.Female)
                ped.RelationshipGroup = RelationshipGroup.Civillian_Female;

            return ped;
        }

        public Ped GetPedOnSeat(VehicleSeat seat)
        {
            Pointer pedPtr = typeof(int);

            if (seat <= VehicleSeat.None)
                return null;

            if (seat == VehicleSeat.Driver)
                Function.Call(Natives.GET_DRIVER_OF_CAR, m_handle, pedPtr);
            else if (Function.Call<bool>(Natives.IS_CAR_PASSENGER_SEAT_FREE, m_handle, (uint)seat))
                return null;
            else
                Function.Call(Natives.GET_CHAR_IN_CAR_PASSENGER_SEAT, m_handle, (uint)seat, pedPtr);

            if ((int)pedPtr == 0)
                return null;

            return ObjectCache<Ped>.Get((int)pedPtr);
        }

        public bool IsSeatFree(VehicleSeat seat)
        {
            if (seat <= VehicleSeat.None)
                return false;

            if (seat == VehicleSeat.AnyPassengerSeat)
            {
                for (int i = 0; i < PassengerSeats; i++)
                {
                    if (IsSeatFree((VehicleSeat)i)) 
                        return true;
                }

                return false;
            }
            else if (seat == VehicleSeat.Driver)
            {
                Pointer pedPtr = typeof(int);
                Function.Call(Natives.GET_DRIVER_OF_CAR, m_handle, pedPtr);

                return (int)pedPtr == 0;
            }
            else
            {
                return Function.Call<bool>(Natives.IS_CAR_PASSENGER_SEAT_FREE, m_handle, (uint)seat);
            }
        }

        public bool IsTouching(Vehicle vehicle)
        {
            if (!vehicle.Exists)
                return false;

            return Function.Call<bool>(Natives.IS_CAR_TOUCHING_CAR, vehicle.Handle);
        }

        public VehicleSeat GetFreeSeat()
        {
            if (IsSeatFree(VehicleSeat.Driver))
                return VehicleSeat.Driver;

            return GetFreePassengerSeat();
        }

        public VehicleSeat GetFreePassengerSeat()
        {
            for (int i = 0; i < PassengerSeats; i++)
            {
                if (IsSeatFree((VehicleSeat)i))
                    return (VehicleSeat)i;
            }

            return VehicleSeat.None;
        }

        public Vector3 GetOffsetPosition(Vector3 offset)
        {
            Pointer xPtr = typeof(float), yPtr = typeof(float), zPtr = typeof(float);
            Function.Call(Natives.GET_OFFSET_FROM_CAR_IN_WORLD_COORDS, m_handle, offset.X, offset.Y, offset.Z, xPtr, yPtr, zPtr);

            return new Vector3((float)xPtr, (float)yPtr, (float)zPtr);
        }

        public Vector3 GetOffset(Vector3 worldPosition)
        {
            Pointer xPtr = typeof(float), yPtr = typeof(float), zPtr = typeof(float);
            Function.Call(Natives.GET_OFFSET_FROM_CAR_GIVEN_WORLD_COORDS, m_handle, worldPosition.X, worldPosition.Y, worldPosition.Z, xPtr, yPtr, zPtr);

            return new Vector3((float)xPtr, (float)yPtr, (float)zPtr);
        }

        public bool IsTireBurst(VehicleWheel wheel)
        {
            return Function.Call<bool>(Natives.IS_CAR_TYRE_BURST, m_handle, (int)wheel);
        }

        public void BurstTire(VehicleWheel wheel)
        {
            Function.Call(Natives.BURST_CAR_TYRE, m_handle, (int)wheel);
        }

        //more unsafe stuff
        /*public void FixTire(VehicleWheel wheel)
        {

        }*/

        public void CloseAllDoors()
        {
            Function.Call(Natives.CLOSE_ALL_CAR_DOORS, m_handle);
        }

        public void Delete() //shouldn't this be in the Entity class?
        {
            //SetExistsFalse();

            if (m_handle == 0)
                return;

            Function.Call(Natives.DELETE_CAR, m_handle);
        }

        public void EveryoneLeaveVehicle()
        {
            Function.Call(Natives.TASK_EVERYONE_LEAVE_CAR, m_handle);
        }

        public void PassengersLeaveVehicle(bool immediatly)
        {
            Ped ped;
		    int seats = PassengerSeats;
		    for (int seat = 0; seat < seats; seat++) 
            {
                ped = GetPedOnSeat((VehicleSeat)seat);
                if (ped != null) 
                {
                    if (immediatly)
                        Function.Call(Natives.TASK_LEAVE_CAR_IMMEDIATELY, ped.Handle, m_handle);
				    else
                        Function.Call(Natives.TASK_LEAVE_CAR, ped.Handle, m_handle);
			    }
		    }
        }
        
        public void PassengersLeaveVehicle()
        {
            PassengersLeaveVehicle(false);
        }

        public void Explode()
        {
            Function.Call(Natives.EXPLODE_CAR, m_handle, true, false);
        }

        public void NoLongerNeeded() //shouldn't this be in the Entity class as well?
        {
            Pointer p = m_handle;
            Function.Call(Natives.MARK_CAR_AS_NO_LONGER_NEEDED, p, m_handle);
        }

        public void PlaceOnGroundProperly()
        {
            Function.Call(Natives.SET_CAR_ON_GROUND_PROPERLY, m_handle);
        }

        public void PlaceOnNextStreetProperly()
        {
            Pointer tX = typeof(float), tY = typeof(float), tZ = typeof(float), tH = typeof(float);
            Vector3 p = Position;

            for (int i = 1; i < 40; i++)
            {
                Function.Call(Natives.GET_NTH_CLOSEST_CAR_NODE_WITH_HEADING, p.X, p.Y, p.Z, i, tX, tY, tZ, tH);
                if (!Function.Call<bool>(Natives.IS_POINT_OBSCURED_BY_A_MISSION_ENTITY, (float)tX, (float)tY, (float)tZ, 5.0f, 5.0f, 5.0f))
                {
                    Heading = (float)tH;
                    Position = new Vector3((float)tX, (float)tY, (float)tZ);
                    PlaceOnGroundProperly();

                    return;
                }
            }
        }

        public void Repair()
        {
            Function.Call(Natives.FIX_CAR, m_handle);
        }

        public void MakeProofTo(bool bullets, bool fire, bool explosions, bool collisions, bool meleeAttacks)
        {
            Function.Call(Natives.SET_CAR_PROOFS, m_handle, bullets, fire, explosions, collisions, meleeAttacks);
        }

        public void SoundHorn(int duration)
        {
            Function.Call(Natives.SOUND_CAR_HORN, m_handle, duration);
        }

        public void Wash()
        {
            Dirtyness = 0.0f;
            Function.Call(Natives.WASH_VEHICLE_TEXTURES, m_handle, 255);
        }
    }
}
