using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core.client.clrcore.GameClasses
{
    public sealed class Camera : HandleObject
    {
        private bool m_hasExisted;

        internal override void SetHandle(int handle)
        {
            if (m_handle != -1)
            {
                ObjectCache<Camera>.Remove(this);
            }

            m_handle = handle;
            m_hasExisted = false;

            ObjectCache<Camera>.Add(this);
        }

        public Camera()
        {
            Pointer c = typeof(uint);
            Function.Call(Natives.CREATE_CAM, 14, c);
            this.m_handle = (int)c;
        }

        public Vector3 Position
        {
            get
            {
                Pointer x = typeof(float), y = typeof(float), z = typeof(float);
                Function.Call(Natives.GET_CAM_POS, m_handle, x, y, z);
                return new Vector3((float)x, (float)y, (float)z);
            }
            set
            {
                Function.Call(Natives.SET_CAM_POS, m_handle, value.X, value.Y, value.Z);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                Pointer x = typeof(float), y = typeof(float), z = typeof(float);
                Function.Call(Natives.GET_CAM_ROT, m_handle, x, y, z);
                return new Vector3((float)x, (float)y, (float)z);
            }
            set
            {
                Function.Call(Natives.SET_CAM_ROT, m_handle, value.X, value.Y, value.Z);
            }
        }
        public Vector3 Direction
        {
            get
            {
                return GameMath.RotationToDirection(Rotation);
            }
            set
            {
                Rotation = GameMath.DirectionToRotation(value, Rotation.Y);
            }
        }

        public float Heading
        {
            get
            {
                return Rotation.Z;
            }
            set
            {
                Vector3 rot = Rotation;
                rot.Z = value;
                Rotation = rot;
            }
        }

        public float Roll
        {
            get
            {
                return Rotation.Y;
            }
            set
            {
                Vector3 rot = Rotation;
                rot.Y = value;
                Rotation = rot;
            }
        }

        public float FOV
        {
            get
            {
                Pointer fov = typeof(float);
                Function.Call(Natives.GET_CAM_FOV, m_handle, fov);
                return (float)fov;
            }
            set
            {
                Function.Call(Natives.SET_CAM_FOV, m_handle, value);
            }
        }

        public bool isActive
        {
            get
            {
                return Function.Call<bool>(Natives.IS_CAM_ACTIVE, m_handle);
            }
            set
            {
                if (value) Activate();
                else Deactivate();
            }
        }

        public float DrunkEffectIntensity
        {
            set
            {
                if (value <= 0.0001f) Function.Call(Natives.SET_DRUNK_CAM, 0.0f, 0);
                else Function.Call(Natives.SET_DRUNK_CAM, value, int.MaxValue);
            }
        }

        public void TargetPed(Ped ped)
        {
            Function.Call(Natives.SET_CAM_TARGET_PED, m_handle, ped.Handle);
        }

        public void LookAt(GameObject obj)
        {
            Function.Call(Natives.POINT_CAM_AT_OBJECT, m_handle, obj.Handle);
        }

        public void LookAt(Vehicle veh)
        {
            Function.Call(Natives.POINT_CAM_AT_VEHICLE, m_handle, veh.Handle);
        }

        public void LookAt(Ped ped)
        {
            Function.Call(Natives.POINT_CAM_AT_PED, m_handle, ped.Handle);
        }

        public void LookAt(Vector3 Position)
        {
            Function.Call(Natives.POINT_CAM_AT_COORD, m_handle, Position.X, Position.Y, Position.Z);
        }

        public void Activate()
        {
            Pointer handle = typeof(int);
            handle = m_handle;
            Function.Call(Natives.ACTIVATE_SCRIPTED_CAMS, 1, 1);
            Function.Call(Natives.BEGIN_CAM_COMMANDS, handle);
            Function.Call(Natives.SET_CAM_ACTIVE, m_handle, true);
            Function.Call(Natives.SET_CAM_PROPAGATE, m_handle, true);
        }

        public bool isSphereVisible(Vector3 Position, float Radius)
        {
            return Function.Call<bool>(Natives.CAM_IS_SPHERE_VISIBLE, m_handle, Position.X, Position.Y, Position.Z, Radius);
        }

        public void Deactivate()
        {
            Pointer handle = typeof(int);
            handle = m_handle;
            Function.Call(Natives.SET_CAM_ACTIVE, m_handle, false);
            Function.Call(Natives.SET_CAM_PROPAGATE, m_handle, false);
            Function.Call(Natives.END_CAM_COMMANDS, handle);
            Function.Call(Natives.ACTIVATE_SCRIPTED_CAMS, 0, 0);
        }

        public void Delete()
        {
            if (m_handle == 0) return;
            if (isActive) Deactivate();
            Function.Call(Natives.DESTROY_CAM, m_handle);
        }

        public bool InternalCheckExists()
        {
            if (m_handle == 0) return false;
            try
            {
                return Function.Call<bool>(Natives.DOES_CAM_EXIST, m_handle);
            }
            catch (Exception)
            {
                return false;
            }
        }
    }
}
