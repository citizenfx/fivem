using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public class GameMath
    {
        public static Vector3 DirectionToRotation(Vector3 dir, float roll)
        {
            dir = Vector3.Normalize(dir);
            Vector3 rotval;
            rotval.Z = -MathUtil.RadiansToDegrees((float)Math.Atan2(dir.X, dir.Y));
            Vector3 rotpos = Vector3.Normalize(new Vector3(dir.Z, new Vector3(dir.X, dir.Y, 0.0f).Length(), 0.0f));
            rotval.X = MathUtil.RadiansToDegrees((float)Math.Atan2(rotpos.X, rotpos.Y));
            rotval.Y = roll;
            return rotval;
        }
        public static Vector3 RotationToDirection(Vector3 Rotation)
        {
            float rotZ = MathUtil.DegreesToRadians(Rotation.Z);
            float rotX = MathUtil.DegreesToRadians(Rotation.X);
            float multXY = Math.Abs((float)Math.Cos(rotX));
            return new Vector3((float)-Math.Sin(rotZ) * multXY, (float)Math.Cos(rotZ) * multXY, (float)Math.Sin(rotX));
        }

        public static float DirectionToHeading(Vector3 dir)
        {
            dir.Z = 0.0f;
            dir.Normalize();
            return MathUtil.RadiansToDegrees((float)Math.Atan2(dir.X, dir.Y));
        }

        public static Vector3 HeadingToDirection(float Heading)
        {
            Heading = MathUtil.DegreesToRadians(Heading);
            return new Vector3((float)-Math.Sin(Heading), (float)Math.Cos(Heading), 0.0f);
        }
    }
}
