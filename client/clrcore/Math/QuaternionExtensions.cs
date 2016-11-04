using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    internal static class QuaternionExtensions
    {
        /*internal static Vector3 ToRotation(this Quaternion q)
        {
            float pitch = (float)Math.Atan2(2.0f * (q.Y * q.Z + q.W * q.X), q.W * q.W - q.X * q.X - q.Y * q.Y + q.Z * q.Z);
            float yaw = (float)Math.Atan2(2.0f * (q.X * q.Y + q.W * q.Z), q.W * q.W + q.X * q.X - q.Y * q.Y - q.Z * q.Z);
            float roll = (float)Math.Asin(-2.0f * (q.X * q.Z - q.W * q.Y));

            return new Vector3(MathUtil.RadiansToDegrees(pitch), MathUtil.RadiansToDegrees(roll), MathUtil.RadiansToDegrees(yaw));
        }

        internal static Quaternion FromRotation(Vector3 value)
        {
            float rotX = MathUtil.DegreesToRadians(value.X);
            float rotY = MathUtil.DegreesToRadians(value.Y);
            float rotZ = MathUtil.DegreesToRadians(value.Z);

		    Quaternion qyaw  = Quaternion.RotationAxis(World.WorldVectors.WorldUp, rotZ);
		    qyaw.Normalize();

            Quaternion qpitch = Quaternion.RotationAxis(World.WorldVectors.WorldEast, rotX);
		    qpitch.Normalize();

            Quaternion qroll = Quaternion.RotationAxis(World.WorldVectors.WorldNorth, rotY);
		    qroll.Normalize();

		    Quaternion yawpitch = qyaw * qpitch * qroll;
		    yawpitch.Normalize();

		    return yawpitch;
        }*/
    }
}
