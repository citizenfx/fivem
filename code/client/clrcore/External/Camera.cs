using System;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	public enum CameraShake
	{
		Hand,
		SmallExplosion,
		MediumExplosion,
		LargeExplosion,
		Jolt,
		Vibrate,
		RoadVibration,
		Drunk,
		SkyDiving,
		FamilyDrugTrip,
		DeathFail
	}

	public sealed class Camera : PoolObject, IEquatable<Camera>, ISpatial
	{
		#region Fields
		internal static readonly string[] _shakeNames = {
			"HAND_SHAKE",
			"SMALL_EXPLOSION_SHAKE",
			"MEDIUM_EXPLOSION_SHAKE",
			"LARGE_EXPLOSION_SHAKE",
			"JOLT_SHAKE",
			"VIBRATE_SHAKE",
			"ROAD_VIBRATION_SHAKE",
			"DRUNK_SHAKE",
			"SKY_DIVING_SHAKE",
			"FAMILY5_DRUG_TRIP_SHAKE",
			"DEATH_FAIL_IN_EFFECT_SHAKE"
		};
		#endregion

		public Camera(int handle) : base(handle)
		{
		}

		/// <summary>
		/// Gets the memory address of this <see cref="Camera"/>.
		/// </summary>
		public IntPtr MemoryAddress
		{
			//get { return MemoryAccess.GetCameraAddress(Handle); }
			get
			{
				// CFX-TODO
				return IntPtr.Zero;
			}
		}

		private IntPtr MatrixAddress
		{
			get
			{
				IntPtr address = MemoryAddress;
				if (address == IntPtr.Zero)
				{
					return IntPtr.Zero;
				}
				return (MemoryAccess.ReadByte(address + 0x220) & 1) == 0 ? address + 0x30 : address + 0x110;
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Camera"/> is currently being rendered.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Camera"/> is active; otherwise, <c>false</c>.
		/// </value>
		public bool IsActive
		{
			get
			{
				return API.IsCamActive(Handle);
			}
			set
			{
				API.SetCamActive(Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets the position of this <see cref="Camera"/>.
		/// </summary>
		public Vector3 Position
		{
			get
			{
				return API.GetCamCoord(Handle);
			}
			set
			{
				API.SetCamCoord(Handle, value.X, value.Y, value.Z);
			}
		}
		/// <summary>
		/// Gets or sets the rotation of this <see cref="Camera"/>.
		/// </summary>
		/// <value>
		/// The yaw, pitch and roll rotations measured in degrees.
		/// </value>
		public Vector3 Rotation
		{
			get
			{
				return API.GetCamRot(Handle, 2);
			}
			set
			{
				API.SetCamRot(Handle, value.X, value.Y, value.Z, 2);
			}
		}
		/// <summary>
		/// Gets or sets the direction this <see cref="Camera"/> is pointing in.
		/// </summary>
		public Vector3 Direction
		{
			get { return ForwardVector; }
			set
			{
				value.Normalize();
				Vector3 vector1 = new Vector3(value.X, value.Y, 0f);
				Vector3 vector2 = new Vector3(value.Z, vector1.Length(), 0f);
				Vector3 vector3 = Vector3.Normalize(vector2);
				Rotation = new Vector3((float)(System.Math.Atan2(vector3.X, vector3.Y) * 57.295779513082323), Rotation.Y, (float)(System.Math.Atan2(value.X, value.Y) * -57.295779513082323));
			}
		}

		/// <summary>
		/// Gets the up vector of this <see cref="Camera"/>.
		/// </summary>
		public Vector3 UpVector
		{
			get
			{
				return Matrix.Up;
			}
		}

		/// <summary>
		/// Gets the forward vector of this <see cref="Camera"/>, see also <seealso cref="Direction"/>.
		/// </summary>
		public Vector3 ForwardVector
		{
			get
			{
				return Matrix.Forward;
			}
		}

		/// <summary>
		/// Gets the right vector of this <see cref="Camera"/>.
		/// </summary>
		public Vector3 RightVector
		{
			get
			{
				return Matrix.Right;
			}
		}
		/// <summary>
		/// Gets the matrix of this <see cref="Camera"/>.
		/// </summary>
		public Matrix Matrix
		{
			get
			{
				Vector3 rightVector = new Vector3();
				Vector3 forwardVector = new Vector3();
				Vector3 upVector = new Vector3();
				Vector3 position = new Vector3();

				API.GetCamMatrix(Handle, ref rightVector, ref forwardVector, ref upVector, ref position);

				return new Matrix(
					rightVector.X, rightVector.Y, rightVector.Z, 0.0f,
					forwardVector.X, forwardVector.Y, forwardVector.Z, 0.0f,
					upVector.X, upVector.Y, upVector.Z, 0.0f,
					position.X, position.Y, position.Z, 1.0f
				);
			}
		}

		/// <summary>
		/// Gets the position in world coords of an offset relative to this <see cref="Camera"/>
		/// </summary>
		/// <param name="offset">The offset from this <see cref="Camera"/>.</param>
		public Vector3 GetOffsetPosition(Vector3 offset)
		{
			return Vector3.TransformCoordinate(offset, Matrix);
		}
		/// <summary>
		/// Gets the relative offset of this <see cref="Camera"/> from a world coords position
		/// </summary>
		/// <param name="worldCoords">The world coords.</param>
		public Vector3 GetPositionOffset(Vector3 worldCoords)
		{
			return Vector3.TransformCoordinate(worldCoords, Matrix.Invert(Matrix));
		}

		/// <summary>
		/// Gets or sets the field of view of this <see cref="Camera"/>.
		/// </summary>
		public float FieldOfView
		{
			get
			{
				return API.GetCamFov(Handle);
			}
			set
			{
				API.SetCamFov(Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets the near clip of this <see cref="Camera"/>.
		/// </summary>
		public float NearClip
		{
			get
			{
				return API.GetCamNearClip(Handle);
			}
			set
			{
				API.SetCamNearClip(Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets the far clip of this <see cref="Camera"/>.
		/// </summary>
		public float FarClip
		{
			get
			{
				return API.GetCamFarClip(Handle);
			}
			set
			{
				API.SetCamFarClip(Handle, value);
			}
		}

		/// <summary>
		/// Sets the near depth of field for this <see cref="Camera"/>.
		/// </summary>
		public float NearDepthOfField
		{
			set
			{
				API.SetCamNearDof(Handle, value);
			}
		}
		/// <summary>
		/// Gets or sets the far depth of field of this <see cref="Camera"/>.
		/// </summary>
		public float FarDepthOfField
		{
			get
			{
				return API.GetCamFarDof(Handle);
			}
			set
			{
				API.SetCamFarDof(Handle, value);
			}
		}
		/// <summary>
		/// Sets the depth of field strength for this <see cref="Camera"/>.
		/// </summary>
		public float DepthOfFieldStrength
		{
			set
			{
				API.SetCamDofStrength(Handle, value);
			}
		}
		/// <summary>
		/// Sets the strenght of the motion blur for this <see cref="Camera"/>
		/// </summary>
		public float MotionBlurStrength
		{
			set
			{
				API.SetCamMotionBlurStrength(Handle, value);
			}
		}

		/// <summary>
		/// Shakes this <see cref="Camera"/>.
		/// </summary>
		/// <param name="shakeType">Type of the shake to apply.</param>
		/// <param name="amplitude">The amplitude of the shaking.</param>
		public void Shake(CameraShake shakeType, float amplitude)
		{
			API.ShakeCam(Handle, _shakeNames[(int)shakeType], amplitude);
		}
		/// <summary>
		/// Stops shaking this <see cref="Camera"/>.
		/// </summary>
		public void StopShaking()
		{
			API.StopCamShaking(Handle, true);
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Camera"/> is shaking.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Camera"/> is shaking; otherwise, <c>false</c>.
		/// </value>
		public bool IsShaking
		{
			get
			{
				return API.IsCamShaking(Handle);
			}
		}
		/// <summary>
		/// Sets the shake amplitude for this <see cref="Camera"/>.
		/// </summary>
		public float ShakeAmplitude
		{
			set
			{
				API.SetCamShakeAmplitude(Handle, value);
			}
		}

		/// <summary>
		/// Points this <see cref="Camera"/> at a specified <see cref="Entity"/>.
		/// </summary>
		/// <param name="target">The <see cref="Entity"/> to point at.</param>
		/// <param name="offset">The offset from the <paramref name="target"/> to point at.</param>
		public void PointAt(Entity target, Vector3 offset = default(Vector3))
		{
			API.PointCamAtEntity(Handle, target.Handle, offset.X, offset.Y, offset.Z, true);
		}
		/// <summary>
		/// Points this <see cref="Camera"/> at a specified <see cref="PedBone"/>.
		/// </summary>
		/// <param name="target">The <see cref="PedBone"/> to point at.</param>
		/// <param name="offset">The offset from the <paramref name="target"/> to point at</param>
		public void PointAt(PedBone target, Vector3 offset = default(Vector3))
		{
			API.PointCamAtPedBone(Handle, target.Owner.Handle, target, offset.X, offset.Y, offset.Z, true);
		}
		/// <summary>
		/// Points this <see cref="Camera"/> at a specified position.
		/// </summary>
		/// <param name="target">The position to point at.</param>
		public void PointAt(Vector3 target)
		{
			API.PointCamAtCoord(Handle, target.X, target.Y, target.Z);
		}
		/// <summary>
		/// Stops this <see cref="Camera"/> pointing at a specific target.
		/// </summary>
		public void StopPointing()
		{
			API.StopCamPointing(Handle);
		}

		/// <summary>
		/// Starts a transition between this <see cref="Camera"/> and the new camera. (Old (incorrect) function, please use the overload function!)
		/// </summary>
		/// <param name="to"></param>
		/// <param name="duration"></param>
		/// <param name="easePosition"></param>
		/// <param name="easeRotation"></param>
		public void InterpTo(Camera to, int duration, bool easePosition, bool easeRotation)
		{
			API.SetCamActiveWithInterp(to.Handle, Handle, duration, easePosition ? 1 : 0, easeRotation ? 1 : 0);
		}
		/// <summary>
		/// Starts a transition between this <see cref="Camera"/> and the new camera.
		/// </summary>
		/// <param name="to"></param>
		/// <param name="duration"></param>
		/// <param name="easePosition"></param>
		/// <param name="easeRotation"></param>
		public void InterpTo(Camera to, int duration, int easePosition, int easeRotation)
		{
			API.SetCamActiveWithInterp(to.Handle, Handle, duration, easePosition, easeRotation);
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Camera"/> is interpolating.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Camera"/> is interpolating; otherwise, <c>false</c>.
		/// </value>
		public bool IsInterpolating
		{
			get
			{
				return API.IsCamInterpolating(Handle);
			}
		}

		/// <summary>
		/// Attaches this <see cref="Camera"/> to a specific <see cref="Entity"/>.
		/// </summary>
		/// <param name="entity">The <see cref="Entity"/> to attach to.</param>
		/// <param name="offset">The offset from the <paramref name="entity"/> to attach to.</param>
		public void AttachTo(Entity entity, Vector3 offset)
		{
			API.AttachCamToEntity(Handle, entity.Handle, offset.X, offset.Y, offset.Z, true);
		}
		/// <summary>
		/// Attaches this <see cref="Camera"/> to a specific <see cref="PedBone"/>.
		/// </summary>
		/// <param name="pedBone">The <see cref="PedBone"/> to attach to.</param>
		/// <param name="offset">The offset from the <paramref name="pedBone"/> to attach to.</param>
		public void AttachTo(PedBone pedBone, Vector3 offset)
		{
			API.AttachCamToPedBone(Handle, pedBone.Owner.Handle, pedBone, offset.X, offset.Y, offset.Z, true);
		}
		/// <summary>
		/// Detaches this <see cref="Camera"/> from any <see cref="Entity"/> or <see cref="PedBone"/> it may be attached to.
		/// </summary>
		public void Detach()
		{
			API.DetachCam(Handle);
		}

		/// <summary>
		/// Destroys this <see cref="Camera"/>.
		/// </summary>
		public override void Delete()
		{
			API.DestroyCam(Handle, false);
		}

		public override bool Exists()
		{
			return API.DoesCamExist(Handle);
		}
		public static bool Exists(Camera camera)
		{
			return !ReferenceEquals(camera, null) && camera.Exists();
		}

		public bool Equals(Camera camera)
		{
			return !ReferenceEquals(camera, null) && Handle == camera.Handle;
		}
		public override bool Equals(object obj)
		{
			return !ReferenceEquals(obj, null) && obj.GetType() == GetType() && Equals((Camera)obj);
		}

		public override int GetHashCode()
		{
			return Handle;
		}

		public static bool operator ==(Camera left, Camera right)
		{
			return ReferenceEquals(left, null) ? ReferenceEquals(right, null) : left.Equals(right);
		}
		public static bool operator !=(Camera left, Camera right)
		{
			return !(left == right);
		}
	}
	public static class GameplayCamera
	{
		/// <summary>
		/// Gets the memory address of the <see cref="GameplayCamera"/>.
		/// </summary>
		public static IntPtr MemoryAddress
		{
			//get { return MemoryAccess.GetGameplayCameraAddress(); }
			get { return IntPtr.Zero; }
		}

		/// <summary>
		/// Gets the position of the <see cref="GameplayCamera"/>.
		/// </summary>
		public static Vector3 Position
		{
			get
			{
				return API.GetGameplayCamCoord();
			}
		}
		/// <summary>
		/// Gets the rotation of the <see cref="GameplayCamera"/>.
		/// </summary>
		/// <value>
		/// The yaw, pitch and roll rotations measured in degrees.
		/// </value>
		public static Vector3 Rotation
		{
			get
			{
				return API.GetGameplayCamRot(2);
			}
		}
		/// <summary>
		/// Gets the direction the <see cref="GameplayCamera"/> is pointing in.
		/// </summary>
		public static Vector3 Direction
		{
			get
			{
				return ForwardVector;
			}
		}

		/// <summary>
		/// Gets the up vector of the <see cref="GameplayCamera"/>.
		/// </summary>
		public static Vector3 UpVector
		{
			get { return MemoryAccess.ReadVector3(MemoryAddress + 0x210); }
		}
		/// <summary>
		/// Gets the forward vector of the <see cref="GameplayCamera"/>, see also <seealso cref="Direction"/>.
		/// </summary>
		public static Vector3 ForwardVector
		{
			//get { return MemoryAccess.ReadVector3(MemoryAddress + 0x200); } 
			get
			{
				Vector3 rot = Rotation;
				float rotX = (float)rot.X / 57.295779513082320876798154814105f;
				float rotZ = (float)rot.Z / 57.295779513082320876798154814105f;
				float multXY = (float)Math.Abs(Math.Cos(rotX));
				return new Vector3((float)(-Math.Sin(rotZ) * multXY), (float)(Math.Cos(rotZ) * multXY), (float)System.Math.Sin(rotX));
			}
		}
		/// <summary>
		/// Gets the right vector of the <see cref="GameplayCamera"/>.
		/// </summary>
		public static Vector3 RightVector
		{
			get { return MemoryAccess.ReadVector3(MemoryAddress + 0x1F0); }
		}

		/// <summary>
		/// Gets the matrix of the <see cref="GameplayCamera"/>.
		/// </summary>
		public static Matrix Matrix
		{
			get { return MemoryAccess.ReadMatrix(MemoryAddress + 0x1F0); }
		}

		/// <summary>
		/// Gets the position in world coords of an offset relative to the <see cref="GameplayCamera"/>
		/// </summary>
		/// <param name="offset">The offset from the <see cref="GameplayCamera"/>.</param>
		public static Vector3 GetOffsetPosition(Vector3 offset)
		{
			return Vector3.TransformCoordinate(offset, Matrix);
		}
		/// <summary>
		/// Gets the relative offset of the <see cref="GameplayCamera"/> from a world coords position
		/// </summary>
		/// <param name="worldCoords">The world coords.</param>
		public static Vector3 GetPositionOffset(Vector3 worldCoords)
		{
			return default(Vector3);
			//return Matrix.InverseTransformPoint(worldCoords);
		}

		/// <summary>
		/// Gets or sets the relative pitch of the <see cref="GameplayCamera"/>.
		/// </summary>
		public static float RelativePitch
		{
			get
			{
				return API.GetGameplayCamRelativePitch();
			}
			set
			{
				API.SetGameplayCamRelativePitch(value, 1f);
			}
		}
		/// <summary>
		/// Gets or sets the relative heading of the <see cref="GameplayCamera"/>.
		/// </summary>
		public static float RelativeHeading
		{
			get
			{
				return API.GetGameplayCamRelativeHeading();
			}
			set
			{
				API.SetGameplayCamRelativeHeading(value);
			}
		}

		/// <summary>
		/// Clamps the yaw of the <see cref="GameplayCamera"/>.
		/// </summary>
		/// <param name="min">The minimum yaw value.</param>
		/// <param name="max">The maximum yaw value.</param>
		public static void ClampYaw(float min, float max)
		{
			API.ClampGameplayCamYaw(min, max);
		}
		/// <summary>
		/// Clamps the pitch of the <see cref="GameplayCamera"/>.
		/// </summary>
		/// <param name="min">The minimum pitch value.</param>
		/// <param name="max">The maximum pitch value.</param>
		public static void ClampPitch(float min, float max)
		{
			API.ClampGameplayCamPitch(min, max);
		}

		/// <summary>
		/// Gets the zoom of the <see cref="GameplayCamera"/>.
		/// </summary>
		public static float Zoom
		{
			get
			{
				return API.GetGameplayCamZoom();
			}
		}
		/// <summary>
		/// Gets the field of view of the <see cref="GameplayCamera"/>.
		/// </summary>
		public static float FieldOfView
		{
			get
			{
				return API.GetGameplayCamFov();
			}
		}

		/// <summary>
		/// Gets a value indicating whether the <see cref="GameplayCamera"/> is rendering.
		/// </summary>
		/// <value>
		/// <c>true</c> if the <see cref="GameplayCamera"/> is rendering; otherwise, <c>false</c>.
		/// </value>
		public static bool IsRendering
		{
			get
			{
				return API.IsGameplayCamRendering();
			}
		}
		/// <summary>
		/// Gets a value indicating whether the aiming camera is rendering.
		/// </summary>
		/// <value>
		/// <c>true</c> if the aiming camera is rendering; otherwise, <c>false</c>.
		/// </value>
		public static bool IsAimCamActive
		{
			get
			{
				return API.IsAimCamActive();
			}
		}
		/// <summary>
		/// Gets a value indicating whether the first person aiming camera is rendering.
		/// </summary>
		/// <value>
		/// <c>true</c> if the aiming camera is rendering; otherwise, <c>false</c>.
		/// </value>
		public static bool IsFirstPersonAimCamActive
		{
			get
			{
				return API.IsFirstPersonAimCamActive();
			}
		}

		/// <summary>
		/// Gets a value indicating whether the <see cref="GameplayCamera"/> is looking behind.
		/// </summary>
		/// <value>
		/// <c>true</c> if the <see cref="GameplayCamera"/> is looking behind; otherwise, <c>false</c>.
		/// </value>
		public static bool IsLookingBehind
		{
			get
			{
				return API.IsGameplayCamLookingBehind();
			}
		}
		/// <summary>
		/// Shakes the <see cref="GameplayCamera"/>.
		/// </summary>
		/// <param name="shakeType">Type of the shake to apply.</param>
		/// <param name="amplitude">The amplitude of the shaking.</param>
		public static void Shake(CameraShake shakeType, float amplitude)
		{
			API.ShakeGameplayCam(Camera._shakeNames[(int)shakeType], amplitude);
		}
		/// <summary>
		/// Stops shaking the <see cref="GameplayCamera"/>.
		/// </summary>
		public static void StopShaking()
		{
			API.StopGameplayCamShaking(true);
		}
		/// <summary>
		/// Gets a value indicating whether the <see cref="GameplayCamera"/> is shaking.
		/// </summary>
		/// <value>
		/// <c>true</c> if the <see cref="GameplayCamera"/> is shaking; otherwise, <c>false</c>.
		/// </value>

		public static bool IsShaking
		{
			get
			{
				return API.IsGameplayCamShaking();
			}
		}
		/// <summary>
		/// Sets the shake amplitude for the <see cref="GameplayCamera"/>.
		/// </summary>
		public static float ShakeAmplitude
		{
			set
			{
				API.SetGameplayCamShakeAmplitude(value);
			}
		}
	}
}
