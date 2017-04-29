using System;
using CitizenFX.Core.Native;
using System.Security;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
	public enum FiringPattern : uint
	{
		Default,
		FullAuto = 3337513804u,
		BurstFire = 3607063905u,
		BurstInCover = 40051185u,
		BurstFireDriveby = 3541198322u,
		FromGround = 577037782u,
		DelayFireByOneSec = 2055493265u,
		SingleShot = 1566631136u,
		BurstFirePistol = 2685983626u,
		BurstFireSMG = 3507334638u,
		BurstFireRifle = 2624893958u,
		BurstFireMG = 3044263348u,
		BurstFirePumpShotGun = 12239771u,
		BurstFireHeli = 2437838959u,
		BurstFireMicro = 1122960381u,
		BurstFireBursts = 1122960381u,
		BurstFireTank = 3804904049u
	}
	[Flags]
	public enum AnimationFlags
	{
		None = 0,
		Loop = 1,
		StayInEndFrame = 2,
		UpperBodyOnly = 16,
		AllowRotation = 32,
		CancelableWithMovement = 128,
		RagdollOnCollision = 4194304
	}
	[Flags]
	public enum LeaveVehicleFlags
	{
		None = 0,
		WarpOut = 16,
		LeaveDoorOpen = 256,
		BailOut = 4096
	}

	public class Tasks
	{
		#region Fields
		Ped _ped;
		#endregion

		internal Tasks(Ped ped)
		{
			_ped = ped;
		}

		public void AchieveHeading(float heading, int timeout = 0)
		{
			Function.Call(Hash.TASK_ACHIEVE_HEADING, _ped.Handle, heading, timeout);
		}

		public void AimAt(Entity target, int duration)
		{
			Function.Call(Hash.TASK_AIM_GUN_AT_ENTITY, _ped.Handle, target.Handle, duration, 0);
		}
		public void AimAt(Vector3 target, int duration)
		{
			Function.Call(Hash.TASK_AIM_GUN_AT_COORD, _ped.Handle, target.X, target.Y, target.Z, duration, 0, 0);
		}

		public void Arrest(Ped ped)
		{
			Function.Call(Hash.TASK_ARREST_PED, _ped.Handle, ped.Handle);
		}

		public void ChatTo(Ped ped)
		{
			Function.Call(Hash.TASK_CHAT_TO_PED, _ped.Handle, ped.Handle, 16, 0f, 0f, 0f, 0f, 0f);
		}

		public void Jump()
		{
			Function.Call(Hash.TASK_JUMP, _ped.Handle, true);
		}
		public void Climb()
		{
			Function.Call(Hash.TASK_CLIMB, _ped.Handle, true);
		}

		public void ClimbLadder()
		{
			Function.Call(Hash.TASK_CLIMB_LADDER, _ped.Handle, 1);
		}

		public void Cower(int duration)
		{
			Function.Call(Hash.TASK_COWER, _ped.Handle, duration);
		}

		public void ChaseWithGroundVehicle(Entity target)
		{
			Function.Call(Hash.TASK_VEHICLE_CHASE, _ped.Handle, target.Handle);
		}

		public void ChaseWithHelicopter(Entity target, Vector3 offset)
		{
			Function.Call(Hash.TASK_HELI_CHASE, _ped.Handle, target.Handle, offset.X, offset.Y, offset.Z);
		}

		public void ChaseWithPlane(Entity target, Vector3 offset)
		{
			Function.Call(Hash.TASK_PLANE_CHASE, _ped.Handle, target.Handle, offset.X, offset.Y, offset.Z);
		}

		public void CruiseWithVehicle(Vehicle vehicle, float speed, int drivingstyle = 0)
		{
			Function.Call(Hash.TASK_VEHICLE_DRIVE_WANDER, _ped.Handle, vehicle.Handle, speed, drivingstyle);
		}

		public void DriveTo(Vehicle vehicle, Vector3 target, float radius, float speed, int drivingstyle = 0)
		{
			Function.Call(Hash.TASK_VEHICLE_DRIVE_TO_COORD_LONGRANGE, _ped.Handle, vehicle.Handle, target.X, target.Y, target.Z, speed, drivingstyle, radius);
		}

		public void EnterAnyVehicle(VehicleSeat seat = VehicleSeat.Any, int timeout = -1, float speed = 0f, int flag = 0)
		{
			Function.Call(Hash.TASK_ENTER_VEHICLE, _ped.Handle, 0, timeout, seat, speed, flag, 0);
		}

		public void EnterVehicle(Vehicle vehicle, VehicleSeat seat = VehicleSeat.Any, int timeout = -1, float speed = 0f, int flag = 0)
		{
			Function.Call(Hash.TASK_ENTER_VEHICLE, _ped.Handle, vehicle.Handle, timeout, seat, speed, flag, 0);
		}

		public static void EveryoneLeaveVehicle(Vehicle vehicle)
		{
			Function.Call(Hash.TASK_EVERYONE_LEAVE_VEHICLE, vehicle.Handle);
		}

		public void FightAgainst(Ped target)
		{
			Function.Call(Hash.TASK_COMBAT_PED, _ped.Handle, target.Handle, 0, 16);
		}
		public void FightAgainst(Ped target, int duration)
		{
			Function.Call(Hash.TASK_COMBAT_PED_TIMED, _ped.Handle, target.Handle, duration, 0);
		}
		public void FightAgainstHatedTargets(float radius)
		{
			Function.Call(Hash.TASK_COMBAT_HATED_TARGETS_AROUND_PED, _ped.Handle, radius, 0);
		}
		public void FightAgainstHatedTargets(float radius, int duration)
		{
			Function.Call(Hash.TASK_COMBAT_HATED_TARGETS_AROUND_PED_TIMED, _ped.Handle, radius, duration, 0);
		}

		public void FleeFrom(Ped ped, int duration = -1)
		{
			Function.Call(Hash.TASK_SMART_FLEE_PED, _ped.Handle, ped.Handle, 100f, duration, 0, 0);
		}
		public void FleeFrom(Vector3 position, int duration = -1)
		{
			Function.Call(Hash.TASK_SMART_FLEE_COORD, _ped.Handle, position.X, position.Y, position.Z, 100f, duration, 0, 0);
		}

		public void FollowPointRoute(params Vector3[] points)
		{
			Function.Call(Hash.TASK_FLUSH_ROUTE);

			foreach (var point in points)
			{
				Function.Call(Hash.TASK_EXTEND_ROUTE, point.X, point.Y, point.Z);
			}

			Function.Call(Hash.TASK_FOLLOW_POINT_ROUTE, _ped.Handle, 1f, 0);
		}

		public void FollowToOffsetFromEntity(Entity target, Vector3 offset, int timeout, float stoppingRange)
		{
			FollowToOffsetFromEntity(target, offset, 1f, timeout, stoppingRange, true);
		}
		public void FollowToOffsetFromEntity(Entity target, Vector3 offset, float movementSpeed, int timeout, float stoppingRange, bool persistFollowing)
		{
			Function.Call(Hash.TASK_FOLLOW_NAV_MESH_TO_COORD, _ped.Handle, target.Handle, offset.X, offset.Y, offset.Z, movementSpeed, timeout, stoppingRange, persistFollowing);
		}

		public void GoTo(Entity target)
		{
			GoTo(target, Vector3.Zero, -1);
		}
		public void GoTo(Entity target, Vector3 offset, int timeout = -1)
		{
			Function.Call(Hash.TASK_GOTO_ENTITY_OFFSET_XY, _ped.Handle, target.Handle, timeout, offset.X, offset.Y, offset.Z, 1f, true);
		}
		public void GoTo(Vector3 position, bool ignorePaths = false, int timeout = -1)
		{
			if (ignorePaths)
			{
				Function.Call(Hash.TASK_GO_STRAIGHT_TO_COORD, _ped.Handle, position.X, position.Y, position.Z, 1f, timeout, 0f, 0f);
			}
			else
			{
				Function.Call(Hash.TASK_FOLLOW_NAV_MESH_TO_COORD, _ped.Handle, position.X, position.Y, position.Z, 1f, timeout, 0f, 0, 0f);
			}
		}

		public void GuardCurrentPosition()
		{
			Function.Call(Hash.TASK_GUARD_CURRENT_POSITION, _ped.Handle, 15f, 10f, true);
		}

		public void HandsUp(int duration)
		{
			Function.Call(Hash.TASK_HANDS_UP, _ped.Handle, duration, 0, -1, false);
		}

		public void LandPlane(Vector3 startPosition, Vector3 touchdownPosition, Vehicle plane = null)
		{
			if (plane == null)
			{
				plane = _ped.CurrentVehicle;
			}
			Function.Call(Hash.TASK_PLANE_LAND, _ped.Handle, plane, startPosition.X, startPosition.Y, startPosition.Z, touchdownPosition.X, touchdownPosition.Y, touchdownPosition.Z);
		}

		public void LeaveVehicle(LeaveVehicleFlags flags = LeaveVehicleFlags.None)
		{
			Function.Call(Hash.TASK_LEAVE_ANY_VEHICLE, _ped.Handle, 0, flags);
		}
		public void LeaveVehicle(Vehicle vehicle, bool closeDoor)
		{
			LeaveVehicle(vehicle, closeDoor ? LeaveVehicleFlags.None : LeaveVehicleFlags.LeaveDoorOpen);
		}
		public void LeaveVehicle(Vehicle vehicle, LeaveVehicleFlags flags)
		{
			Function.Call(Hash.TASK_LEAVE_VEHICLE, _ped.Handle, vehicle.Handle, flags);
		}

		public void LookAt(Entity target, int duration = -1)
		{
			Function.Call(Hash.TASK_LOOK_AT_ENTITY, _ped.Handle, target.Handle, duration, 0, 2);
		}
		public void LookAt(Vector3 position, int duration = -1)
		{
			Function.Call(Hash.TASK_LOOK_AT_COORD, _ped.Handle, position.X, position.Y, position.Z, duration, 0, 2);
		}

		public void ParachuteTo(Vector3 position)
		{
			Function.Call(Hash.TASK_PARACHUTE_TO_TARGET, _ped.Handle, position.X, position.Y, position.Z);
		}

		public void ParkVehicle(Vehicle vehicle, Vector3 position, float heading, float radius = 20.0f, bool keepEngineOn = false)
		{
			Function.Call(Hash.TASK_VEHICLE_PARK, _ped.Handle, vehicle.Handle, position.X, position.Y, position.Z, heading, 1, radius, keepEngineOn);
		}

		public void PerformSequence(TaskSequence sequence)
		{
			if (!sequence.IsClosed)
			{
				sequence.Close(false);
			}

			ClearAll();
			_ped.BlockPermanentEvents = true;

			Function.Call(Hash.TASK_PERFORM_SEQUENCE, _ped.Handle, sequence.Handle);
		}

		public void PlayAnimation(string animDict, string animName)
		{
			PlayAnimation(animDict, animName, 8f, -8f, -1, AnimationFlags.None, 0f);
		}
		public void PlayAnimation(string animDict, string animName, float speed, int duration, float playbackRate)
		{
			PlayAnimation(animDict, animName, speed, -speed, duration, AnimationFlags.None, playbackRate);
		}
		public void PlayAnimation(string animDict, string animName, float blendInSpeed, int duration, AnimationFlags flags)
		{
			PlayAnimation(animDict, animName, blendInSpeed, -8f, duration, flags, 0f);
		}
		public async Task PlayAnimation(string animDict, string animName, float blendInSpeed, float blendOutSpeed, int duration, AnimationFlags flags, float playbackRate)
		{
			Function.Call(Hash.REQUEST_ANIM_DICT, animDict);

			DateTime endtime = DateTime.UtcNow + new TimeSpan(0, 0, 0, 0, 1000);

			while (!Function.Call<bool>(Hash.HAS_ANIM_DICT_LOADED, animDict))
			{
                await BaseScript.Delay(0);

                if (DateTime.UtcNow >= endtime)
				{
					return;
				}
			}

			Function.Call(Hash.TASK_PLAY_ANIM, _ped.Handle, animDict, animName, blendInSpeed, blendOutSpeed, duration, flags, playbackRate, 0, 0, 0);
		}

		public void ReactAndFlee(Ped ped)
		{
			Function.Call(Hash.TASK_REACT_AND_FLEE_PED, _ped.Handle, ped.Handle);
		}

		public void ReloadWeapon()
		{
			Function.Call(Hash.TASK_RELOAD_WEAPON, _ped.Handle, true);
		}

		public void RunTo(Vector3 position, bool ignorePaths = false, int timeout = -1)
		{
			if (ignorePaths)
			{
				Function.Call(Hash.TASK_GO_STRAIGHT_TO_COORD, _ped.Handle, position.X, position.Y, position.Z, 4f, timeout, 0f, 0f);
			}
			else
			{
				Function.Call(Hash.TASK_FOLLOW_NAV_MESH_TO_COORD, _ped.Handle, position.X, position.Y, position.Z, 4f, timeout, 0f, 0, 0f);
			}
		}


		public void ShootAt(Ped target, int duration = -1, FiringPattern pattern = FiringPattern.Default)
		{
			Function.Call(Hash.TASK_SHOOT_AT_ENTITY, _ped.Handle, target.Handle, duration, pattern);
		}
		public void ShootAt(Vector3 position, int duration = -1, FiringPattern pattern = FiringPattern.Default)
		{
			Function.Call(Hash.TASK_SHOOT_AT_COORD, _ped.Handle, position.X, position.Y, position.Z, duration, pattern);
		}

		public void ShuffleToNextVehicleSeat(Vehicle vehicle)
		{
			Function.Call(Hash.TASK_SHUFFLE_TO_NEXT_VEHICLE_SEAT, _ped.Handle, vehicle.Handle);
		}

		public void Skydive()
		{
			Function.Call(Hash.TASK_SKY_DIVE, _ped.Handle);
		}

		public void SlideTo(Vector3 position, float heading)
		{
			Function.Call(Hash.TASK_PED_SLIDE_TO_COORD, _ped.Handle, position.X, position.Y, position.Z, heading, 0.7f);
		}

		public void StandStill(int duration)
		{
			Function.Call(Hash.TASK_STAND_STILL, _ped.Handle, duration);
		}

		public void StartScenario(string name, Vector3 position)
		{
			Function.Call(Hash.TASK_START_SCENARIO_AT_POSITION, _ped.Handle, name, position.X, position.Y, position.Z, 0f, 0, 0, 1);
		}

		public void SwapWeapon()
		{
			Function.Call(Hash.TASK_SWAP_WEAPON, _ped.Handle, false);
		}

		public void TurnTo(Entity target, int duration = -1)
		{
			Function.Call(Hash.TASK_TURN_PED_TO_FACE_ENTITY, _ped.Handle, target.Handle, duration);
		}

		public void TurnTo(Vector3 position, int duration = -1)
		{
			Function.Call(Hash.TASK_TURN_PED_TO_FACE_COORD, _ped.Handle, position.X, position.Y, position.Z, duration);
		}

		public void UseParachute()
		{
			Function.Call(Hash.TASK_PARACHUTE, _ped.Handle, true);
		}
		public void UseMobilePhone()
		{
			Function.Call(Hash.TASK_USE_MOBILE_PHONE, _ped.Handle, true);
		}
		public void UseMobilePhone(int duration)
		{
			Function.Call(Hash.TASK_USE_MOBILE_PHONE_TIMED, _ped.Handle, duration);
		}
		public void PutAwayParachute()
		{
			Function.Call(Hash.TASK_PARACHUTE, _ped.Handle, false);
		}
		public void PutAwayMobilePhone()
		{
			Function.Call(Hash.TASK_USE_MOBILE_PHONE, _ped.Handle, false);
		}

		public void VehicleChase(Ped target)
		{
			Function.Call(Hash.TASK_VEHICLE_CHASE, _ped.Handle, target.Handle);
		}
		public void VehicleShootAtPed(Ped target)
		{
			Function.Call(Hash.TASK_VEHICLE_SHOOT_AT_PED, _ped.Handle, target.Handle, 20f);
		}

		public void Wait(int duration)
		{
			Function.Call(Hash.TASK_PAUSE, _ped.Handle, duration);
		}

		public void WanderAround()
		{
			Function.Call(Hash.TASK_WANDER_STANDARD, _ped.Handle, 0, 0);
		}
		public void WanderAround(Vector3 position, float radius)
		{
			Function.Call(Hash.TASK_WANDER_IN_AREA, _ped.Handle, position.X, position.Y, position.Z, radius, 0, 0);
		}

		public void WarpIntoVehicle(Vehicle vehicle, VehicleSeat seat)
		{
			Function.Call(Hash.TASK_WARP_PED_INTO_VEHICLE, _ped.Handle, vehicle.Handle, seat);
		}
		public void WarpOutOfVehicle(Vehicle vehicle)
		{
			Function.Call(Hash.TASK_LEAVE_VEHICLE, _ped.Handle, vehicle.Handle, 16);
		}

		public void ClearAll()
		{
			Function.Call(Hash.CLEAR_PED_TASKS, _ped.Handle);
		}
		public void ClearAllImmediately()
		{
			Function.Call(Hash.CLEAR_PED_TASKS_IMMEDIATELY, _ped.Handle);
		}
		public void ClearLookAt()
		{
			Function.Call(Hash.TASK_CLEAR_LOOK_AT, _ped.Handle);
		}
		public void ClearSecondary()
		{
			Function.Call(Hash.CLEAR_PED_SECONDARY_TASK, _ped.Handle);
		}
		public void ClearAnimation(string animSet, string animName)
		{
			Function.Call(Hash.STOP_ANIM_TASK, _ped.Handle, animSet, animName, -4f);
		}
	}

	public sealed class TaskSequence : IDisposable
	{
		#region Fields
		static Ped _nullPed = null;
		#endregion

        [SecuritySafeCritical]
		public TaskSequence()
		{
            Create();

			if (ReferenceEquals(_nullPed, null))
			{
				_nullPed = new Ped(0);
			}
		}
		public TaskSequence(int handle)
		{
			Handle = handle;

			if (ReferenceEquals(_nullPed, null))
			{
				_nullPed = new Ped(0);
			}
		}

        [SecuritySafeCritical]
        private void Create()
        {
            int handle;
            unsafe
            {
                Function.Call(Hash.OPEN_SEQUENCE_TASK, &handle);
            }

            Handle = handle;
        }

        [SecuritySafeCritical]
        public void Dispose()
		{
            ClearHandle();
			Handle = 0;
			GC.SuppressFinalize(this);
		}

        [SecuritySafeCritical]
        private void ClearHandle()
        {
            int handle = Handle;
            unsafe
            {
                Function.Call(Hash.CLEAR_SEQUENCE_TASK, &handle);
            }
        }

		public int Handle { get; private set; }

		public int Count { get; private set; }
		public bool IsClosed { get; private set; }

		public Tasks AddTask
		{
			get
			{
				if (IsClosed)
				{
					throw new Exception("You can't add tasks to a closed sequence!");
				}

				Count++;
				return _nullPed.Task;
			}
		}

		public void Close()
		{
			Close(false);
		}
		public void Close(bool repeat)
		{
			if (IsClosed)
			{
				return;
			}

			Function.Call(Hash.SET_SEQUENCE_TO_REPEAT, Handle, repeat);
			Function.Call(Hash.CLOSE_SEQUENCE_TASK, Handle);

			IsClosed = true;
		}
	}
}
