using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public sealed class GameObject : HandleObject
    {
        private bool m_hasExisted;
        internal override void SetHandle(int handle)
        {
            if (m_handle != -1)
                ObjectCache<GameObject>.Remove(this);

            m_handle = handle;
            m_hasExisted = false;

            ObjectCache<GameObject>.Add(this);
        }

        internal bool RecordCollisions
        {
            set
            {
                Function.Call(Natives.SET_OBJECT_RECORDS_COLLISIONS, m_handle, value);
            }
        }
        internal bool Dynamic
        {
            set
            {
                Function.Call(Natives.SET_OBJECT_DYNAMIC, m_handle, value);
            }
        }

        // This requires some unsafe stuff.
        /*public int MemoryAddress {
            return null;
        }*/
        public bool Exists
        {
            get
            {
                return Function.Call<bool>(Natives.DOES_OBJECT_EXIST, m_handle);
            }
        }
        public Vector3 Position
        {
            get
            {
                Pointer x = typeof(float), y = typeof(float), z = typeof(float);
                Function.Call(Natives.GET_OBJECT_COORDINATES, m_handle, x, y, z);
                return new Vector3((float)x, (float)y, (float)z);
            }
            set
            {
                Function.Call(Natives.SET_OBJECT_COORDINATES, m_handle, value.X, value.Y, value.Z);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                return RotationQuaternion.ToRotation();
            }
            set
            {
                Function.Call(Natives.SET_OBJECT_ROTATION, m_handle, value.X, value.Y, value.Z);
            }
        }

        public Quaternion RotationQuaternion
        {
            get
            {
                Pointer x = typeof(float), y = typeof(float), z = typeof(float), w = typeof(float);
                Function.Call(Natives.GET_OBJECT_QUATERNION, m_handle, x, y, z, w);
                return new Quaternion((float)x, (float)y, (float)z, (float)w);
            }
            set
            {
                Function.Call(Natives.SET_OBJECT_QUATERNION, m_handle, value.X, value.Y, value.Z, value.W);
            }
        }

        public float Heading
        {
            get
            {
                Pointer val = typeof(float);
                Function.Call(Natives.GET_OBJECT_HEADING, val);
                return (float)val;
            }
            set
            {
                Function.Call(Natives.SET_OBJECT_HEADING, value);
            }
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

        public Room CurrentRoom
        {
            get
            {
                //The commented out parts were commented out in the scripthook, yet I think they might be necessary - I don't know.
                //Pointer ii = typeof(int);
                Pointer rk = typeof(int);
                //Function.Call(Natives.GET_INTERIOR_FROM_OBJECT, m_handle, ii);
                Function.Call(Natives.GET_ROOM_KEY_FROM_OBJECT, m_handle, rk);
                return new Room((int)rk, (int)0/*(int)ii*/);
            }
            set
            {
                Function.Call(Natives.ADD_OBJECT_TO_INTERIOR_ROOM_BY_KEY, m_handle, (uint)value.RoomKey);
            }
        }
        public Model Model
        {
            get
            {
                Pointer model = typeof(int);
                Function.Call(Natives.GET_OBJECT_MODEL, m_handle, model);
                return new Model((int) model);
            }
        }

        public bool Visible
        {
            set
            {
                Function.Call(Natives.SET_OBJECT_VISIBLE, value);
            }
        }

        public bool IsOnFire
        {
            get
            {
                return Function.Call<bool>(Natives.IS_OBJECT_ON_FIRE, m_handle);
            }
            // To set the fires, we'll need a Fire class.
        }

        public bool FreezePosition
        {
            set
            {
                Function.Call(Natives.FREEZE_OBJECT_POSITION, m_handle, value);
            }
        }

        public Vector3 Velocity
        {
            get
            {
                Pointer x = typeof(float), y = typeof(float), z = typeof(float);
                Function.Call(Natives.GET_OBJECT_VELOCITY, m_handle, x, y, z);
                return new Vector3((float)x, (float)y, (float)z);
            }
            set
            {
                ApplyForce(value - Velocity);
            }
        }

        public void ApplyForce(Vector3 Direction, Vector3 Rotation)
        {
            ApplyForce(Direction, new Vector3());
        }

        public void ApplyForce(Vector3 Direction)
        {
            ApplyForce(Direction, new Vector3());
        }

        public void ApplyForceRelative(Vector3 Direction, Vector3 Rotation)
        {
            Function.Call(Natives.APPLY_FORCE_TO_OBJECT, m_handle, 3, Direction.X, Direction.Y, Direction.Z, Rotation.X, Rotation.Y, Rotation.Z, 0, 0, 1, 1);
        }

        public void ApplyForceRelative(Vector3 Direction)
        {
            ApplyForceRelative(Direction, new Vector3());
        }

        public bool IsAttachedSomewhere
        {
            get
            {
                return Function.Call<bool>(Natives.IS_OBJECT_ATTACHED, m_handle);
            }
        }

        public bool Collision
        {
            set
            {
                Function.Call(Natives.SET_OBJECT_COLLISION, m_handle, value);
            }
        }

        public Vector3 GetOffsetPosition(Vector3 Offset)
        {
            Pointer x = typeof(float), y = typeof(float), z = typeof(float);
            Function.Call(Natives.GET_OFFSET_FROM_OBJECT_IN_WORLD_COORDS, m_handle, Offset.X, Offset.Y, Offset.Z, x, y, z);
            return new Vector3((float)x, (float)y, (float)z);
        }

        /*public Blip AttachBlip()
        {
            return Blip.AddBlip(this);
        }*/

        /*public void AttachToPed(Ped Ped, Bone Bone, Vector3 PositionOffset, Vector3 Rotation)
        {
            CurrentRoom = Ped.CurrentRoom;
            if (IsAttachedSomewhere) Detach();
            Function.Call(Natives.ATTACH_OBJECT_TO_PED, m_handle, Ped.Handle, (uint)Bone, PositionOffset.X, PositionOffset.Y, PositionOffset.Z, Rotation.X, Rotation.Y, Rotation.Z, 0);
        }*/

        public void AttachToVehicle(Vehicle Vehicle, Vector3 PositionOffset, Vector3 Rotation)
        {
            if (IsAttachedSomewhere) Detach();
            Function.Call(Natives.ATTACH_OBJECT_TO_CAR, m_handle, Vehicle.Handle, PositionOffset.X, PositionOffset.Y, PositionOffset.Z, Rotation.X, Rotation.Y, Rotation.Z);
        }

        public void Delete()
        {
            if (m_handle == 0) return;
            Pointer obj = m_handle;
            Function.Call(Natives.DELETE_OBJECT, obj);
        }

        public void Detach()
        {
            Function.Call(Natives.DETACH_OBJECT, m_handle, true);
        }

        public bool InternalCheckExists()
        {
            if (m_handle == 0) return false;
            try
            {
                return Function.Call<bool>(Natives.DOES_OBJECT_EXIST, m_handle);
            }
            catch (Exception)
            {
                return false;
            }
        }
        public void NoLongerNeeded()
        {
            Pointer h = m_handle;
            Function.Call(Natives.MARK_OBJECT_AS_NO_LONGER_NEEDED, h);
        }
    }
}
