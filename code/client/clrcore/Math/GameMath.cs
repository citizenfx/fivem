using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public class GameMath
    {
        public static Vector3 DirectionToRotation(Vector3 direction, float roll)
        {
            direction = Vector3.Normalize(direction);
            Vector3 rotpos = Vector3.Normalize(new Vector3(direction.Z, (float)Math.Sqrt(direction.X * direction.X + direction.Y * direction.Y), 0.0f));
			return new Vector3(
				MathUtil.RadiansToDegrees((float)Math.Atan2(rotpos.X, rotpos.Y)),
				roll,
				-MathUtil.RadiansToDegrees((float)Math.Atan2(direction.X, direction.Y))
			);
        }

        public static Vector3 RotationToDirection(Vector3 rotation)
        {
            float rotZ = MathUtil.DegreesToRadians(rotation.Z);
            float rotX = MathUtil.DegreesToRadians(rotation.X);
            float multXY = Math.Abs((float)Math.Cos(rotX));
            return new Vector3((float)-Math.Sin(rotZ) * multXY, (float)Math.Cos(rotZ) * multXY, (float)Math.Sin(rotX));
        }

        public static float DirectionToHeading(Vector3 dir)
        {
			Vector2 dir2 = (Vector2)dir;
			dir2.Normalize();
            return MathUtil.RadiansToDegrees((float)Math.Atan2(dir2.X, dir2.Y));
        }

        public static Vector3 HeadingToDirection(float heading)
        {
            heading = MathUtil.DegreesToRadians(heading);
            return new Vector3((float)-Math.Sin(heading), (float)Math.Cos(heading), 0.0f);
        }
    }
}
