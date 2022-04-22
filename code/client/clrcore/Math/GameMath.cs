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

		public static bool PointIsWithinCircle(float circleRadius, float circleCenterPointX, float circleCenterPointY, float pointToCheckX, float pointToCheckY)
		{
			return (Math.Pow(pointToCheckX - circleCenterPointX, 2) + Math.Pow(pointToCheckY - circleCenterPointY, 2)) < (Math.Pow(circleRadius, 2));
		}

		public static bool PointIsWithinSphere(float sphereRadius, Vector3 SphereCenter, Vector3 PointToCheck)
		{
			return (Math.Pow(SphereCenter.X - PointToCheck.X, 2) + Math.Pow(SphereCenter.Y - PointToCheck.Y, 2) + Math.Pow(SphereCenter.Z - PointToCheck.Z, 2)) < Math.Pow(sphereRadius, 2);
		}

		private static Vector3 PolarSphereToWorld3D(Vector3 center, float radius, float polarAngleDeg, float azimuthAngleDeg)
		{
			var polarAngleRad = MathUtil.DegreesToRadians(polarAngleDeg);
			var azimuthAngleRad = MathUtil.DegreesToRadians(azimuthAngleDeg);
			return new Vector3(
				center.X + radius * ((float)Math.Sin(azimuthAngleRad) * (float)Math.Cos(polarAngleRad)),
				center.Y - radius * ((float)Math.Sin(azimuthAngleRad) * (float)Math.Sin(polarAngleRad)),
				center.Z - radius * (float)Math.Cos(azimuthAngleRad)
			);
		}

		public static Vector2 ToVector2(float[] xyArray)
		{
			if (xyArray.Length >= 2)
				return new Vector2(xyArray[0], xyArray[1]);
			return Vector2.Zero;
		}

		public static Vector2 ToVector2(Vector3 vector)
		{
			return new Vector2(vector.X, vector.Y);
		}

		public static Vector2 ToVector2(Vector4 vector)
		{
			return new Vector2(vector.X, vector.Y);
		}

		public static Vector3 ToVector3(float[] xyzArray)
		{
			if (xyzArray.Length >= 3)
				return new Vector3(xyzArray[0], xyzArray[1], xyzArray[2]);
			return Vector3.Zero;
		}

		public static Vector3 ToVector3(Vector4 vector)
		{
			return new Vector3(vector.X, vector.Y, vector.Z);
		}

		public static Vector4 ToVector4(float[] xyzwArray)
		{
			if (xyzwArray.Length >= 4)
				return new Vector4(xyzwArray[0], xyzwArray[1], xyzwArray[2], xyzwArray[3]);
			return Vector4.Zero;
		}
	}
}
