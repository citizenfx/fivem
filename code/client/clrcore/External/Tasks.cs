using System;

#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;
using Task = CitizenFX.Core.Coroutine;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;
using System.Threading.Tasks;

namespace CitizenFX.Core
#endif
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
			API.TaskAchieveHeading(_ped.Handle, heading, timeout);
		}

		public void AimAt(Entity target, int duration)
		{
			API.TaskAimGunAtEntity(_ped.Handle, target.Handle, duration, false);
		}
		public void AimAt(Vector3 target, int duration)
		{
			API.TaskAimGunAtCoord(_ped.Handle, target.X, target.Y, target.Z, duration, false, false);
		}

		public void Arrest(Ped ped)
		{
			API.TaskArrestPed(_ped.Handle, ped.Handle);
		}

		public void ChatTo(Ped ped)
		{
			API.TaskChatToPed(_ped.Handle, ped.Handle, 16, 0f, 0f, 0f, 0f, 0f);
		}

		public void Jump()
		{
			API.TaskJump(_ped.Handle, true);
		}
		public void Climb()
		{
			API.TaskClimb(_ped.Handle, true);
		}

		public void ClimbLadder()
		{
			API.TaskClimbLadder(_ped.Handle, 1);
		}

		public void Cower(int duration)
		{
			API.TaskCower(_ped.Handle, duration);
		}

		public void ChaseWithGroundVehicle(Entity target)
		{
			API.TaskVehicleChase(_ped.Handle, target.Handle);
		}

		public void ChaseWithHelicopter(Entity target, Vector3 offset)
		{
			API.TaskHeliChase(_ped.Handle, target.Handle, offset.X, offset.Y, offset.Z);
		}

		public void ChaseWithPlane(Entity target, Vector3 offset)
		{
			API.TaskPlaneChase(_ped.Handle, target.Handle, offset.X, offset.Y, offset.Z);
		}

		public void CruiseWithVehicle(Vehicle vehicle, float speed, int drivingstyle = 0)
		{
			API.TaskVehicleDriveWander(_ped.Handle, vehicle.Handle, speed, drivingstyle);
		}

		public void DriveTo(Vehicle vehicle, Vector3 target, float radius, float speed, int drivingstyle = 0)
		{
			API.TaskVehicleDriveToCoordLongrange(_ped.Handle, vehicle.Handle, target.X, target.Y, target.Z, speed, drivingstyle, radius);
		}

		public void EnterAnyVehicle(VehicleSeat seat = VehicleSeat.Any, int timeout = -1, float speed = 0f, int flag = 0)
		{
			API.TaskEnterVehicle(_ped.Handle, 0, timeout, (int)seat, speed, flag, 0);
		}

		public void EnterVehicle(Vehicle vehicle, VehicleSeat seat = VehicleSeat.Any, int timeout = -1, float speed = 0f, int flag = 0)
		{
			API.TaskEnterVehicle(_ped.Handle, vehicle.Handle, timeout, (int)seat, speed, flag, 0);
		}

		public static void EveryoneLeaveVehicle(Vehicle vehicle)
		{
			API.TaskEveryoneLeaveVehicle(vehicle.Handle);
		}

		public void FightAgainst(Ped target)
		{
			API.TaskCombatPed(_ped.Handle, target.Handle, 0, 16);
		}
		public void FightAgainst(Ped target, int duration)
		{
			API.TaskCombatPedTimed(_ped.Handle, target.Handle, duration, 0);
		}
		public void FightAgainstHatedTargets(float radius)
		{
			API.TaskCombatHatedTargetsAroundPed(_ped.Handle, radius, 0);
		}
		public void FightAgainstHatedTargets(float radius, int duration)
		{
			API.TaskCombatHatedTargetsAroundPedTimed(_ped.Handle, radius, duration, 0);
		}

		public void FleeFrom(Ped ped, int duration = -1)
		{
			API.TaskSmartFleePed(_ped.Handle, ped.Handle, 100f, duration, false, false);
		}
		public void FleeFrom(Vector3 position, int duration = -1)
		{
			API.TaskSmartFleeCoord(_ped.Handle, position.X, position.Y, position.Z, 100f, duration, false, false);
		}

		public void FollowPointRoute(params Vector3[] points)
		{
			API.TaskFlushRoute();

			foreach (var point in points)
			{
				API.TaskExtendRoute(point.X, point.Y, point.Z);
			}

			API.TaskFollowPointRoute(_ped.Handle, 1f, 0);
		}

		public void FollowToOffsetFromEntity(Entity target, Vector3 offset, int timeout, float stoppingRange)
		{
			FollowToOffsetFromEntity(target, offset, 1f, timeout, stoppingRange, true);
		}
		public void FollowToOffsetFromEntity(Entity target, Vector3 offset, float movementSpeed, int timeout, float stoppingRange, bool persistFollowing)
		{
			API.TaskFollowToOffsetOfEntity(_ped.Handle, target.Handle, offset.X, offset.Y, offset.Z, movementSpeed, timeout, stoppingRange, persistFollowing);
		}

		public void GoTo(Entity target)
		{
			GoTo(target, Vector3.Zero, -1);
		}
		public void GoTo(Entity target, Vector3 offset, int timeout = -1)
		{
			API.TaskGotoEntityOffsetXy(_ped.Handle, target.Handle, timeout, offset.X, offset.Y, offset.Z, 1f, true);
		}
		public void GoTo(Vector3 position, bool ignorePaths = false, int timeout = -1)
		{
			if (ignorePaths)
			{
				API.TaskGoStraightToCoord(_ped.Handle, position.X, position.Y, position.Z, 1f, timeout, 0f, 0f);
			}
			else
			{
				API.TaskFollowNavMeshToCoord(_ped.Handle, position.X, position.Y, position.Z, 1f, timeout, 0f, false, 0f);
			}
		}

		public void GuardCurrentPosition()
		{
			API.TaskGuardCurrentPosition(_ped.Handle, 15f, 10f, true);
		}

		public void HandsUp(int duration)
		{
			API.TaskHandsUp(_ped.Handle, duration, 0, -1, false);
		}

		public void LandPlane(Vector3 startPosition, Vector3 touchdownPosition, Vehicle plane = null)
		{
			if (plane == null)
			{
				plane = _ped.CurrentVehicle;
			}

			if (plane == null || !plane.Exists() || plane.IsDead)
			{
				API.TaskPlaneLand(_ped.Handle, 0, startPosition.X, startPosition.Y, startPosition.Z, touchdownPosition.X, touchdownPosition.Y, touchdownPosition.Z);
			}
			else
			{
				API.TaskPlaneLand(_ped.Handle, plane.Handle, startPosition.X, startPosition.Y, startPosition.Z, touchdownPosition.X, touchdownPosition.Y, touchdownPosition.Z);
			}
		}

		public void LeaveVehicle(LeaveVehicleFlags flags = LeaveVehicleFlags.None)
		{
			API.TaskLeaveAnyVehicle(_ped.Handle, 0, (int)flags);
		}
		public void LeaveVehicle(Vehicle vehicle, bool closeDoor)
		{
			LeaveVehicle(vehicle, closeDoor ? LeaveVehicleFlags.None : LeaveVehicleFlags.LeaveDoorOpen);
		}
		public void LeaveVehicle(Vehicle vehicle, LeaveVehicleFlags flags)
		{
			API.TaskLeaveVehicle(_ped.Handle, vehicle.Handle, (int)flags);
		}

		/// <summary>
		/// Looks at the specified <see cref="Entity"/>.
		/// </summary>
		/// <param name="target"></param>
		/// <param name="duration">Must be greater than 0 for the ped to actually move their head.</param>
		public void LookAt(Entity target, int duration = 1)
		{
			API.TaskLookAtEntity(_ped.Handle, target.Handle, duration, 0, 2);
		}
		/// <summary>
		/// Looks at the specified <see cref="Vector3"/> position.
		/// </summary>
		/// <param name="position"></param>
		/// <param name="duration">Must be greater than 0 for the ped to actually move their head.</param>
		public void LookAt(Vector3 position, int duration = 1)
		{
			API.TaskLookAtCoord(_ped.Handle, position.X, position.Y, position.Z, duration, 0, 2);
		}

		public void ParachuteTo(Vector3 position)
		{
			API.TaskParachuteToTarget(_ped.Handle, position.X, position.Y, position.Z);
		}

		public void ParkVehicle(Vehicle vehicle, Vector3 position, float heading, float radius = 20.0f, bool keepEngineOn = false)
		{
			API.TaskVehiclePark(_ped.Handle, vehicle.Handle, position.X, position.Y, position.Z, heading, 1, radius, keepEngineOn);
		}

		public void PerformSequence(TaskSequence sequence)
		{
			if (!sequence.IsClosed)
			{
				sequence.Close(false);
			}

			ClearAll();
			_ped.BlockPermanentEvents = true;

			API.TaskPerformSequence(_ped.Handle, sequence.Handle);
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
			if (!API.HasAnimDictLoaded(animDict))
			{
				API.RequestAnimDict(animDict);
			}

			DateTime endtime = DateTime.UtcNow + new TimeSpan(0, 0, 0, 0, 1000);

			while (!API.HasAnimDictLoaded(animDict))
			{
				await BaseScript.Delay(0);

				if (DateTime.UtcNow >= endtime)
				{
					return;
				}
			}

			API.TaskPlayAnim(_ped.Handle, animDict, animName, blendInSpeed, blendOutSpeed, duration, (int)flags, playbackRate, false, false, false);
		}

		public void ReactAndFlee(Ped ped)
		{
			API.TaskReactAndFleePed(_ped.Handle, ped.Handle);
		}

		public void ReloadWeapon()
		{
			API.TaskReloadWeapon(_ped.Handle, true);
		}

		public void RunTo(Vector3 position, bool ignorePaths = false, int timeout = -1)
		{
			if (ignorePaths)
			{
				API.TaskGoStraightToCoord(_ped.Handle, position.X, position.Y, position.Z, 4f, timeout, 0f, 0f);
			}
			else
			{
				API.TaskFollowNavMeshToCoord(_ped.Handle, position.X, position.Y, position.Z, 4f, timeout, 0f, false, 0f);
			}
		}


		public void ShootAt(Ped target, int duration = -1, FiringPattern pattern = FiringPattern.Default)
		{
			API.TaskShootAtEntity(_ped.Handle, target.Handle, duration, (uint)pattern);
		}
		public void ShootAt(Vector3 position, int duration = -1, FiringPattern pattern = FiringPattern.Default)
		{
			API.TaskShootAtCoord(_ped.Handle, position.X, position.Y, position.Z, duration, (uint)pattern);
		}

		public void ShuffleToNextVehicleSeat(Vehicle vehicle)
		{
			API.TaskShuffleToNextVehicleSeat(_ped.Handle, vehicle.Handle);
		}

		public void Skydive()
		{
			API.TaskSkyDive(_ped.Handle);
		}

		public void SlideTo(Vector3 position, float heading)
		{
			API.TaskPedSlideToCoord(_ped.Handle, position.X, position.Y, position.Z, heading, 0.7f);
		}

		public void StandStill(int duration)
		{
			API.TaskStandStill(_ped.Handle, duration);
		}

		public void StartScenario(string name, Vector3 position)
		{
			API.TaskStartScenarioAtPosition(_ped.Handle, name, position.X, position.Y, position.Z, 0f, 0, false, true);
		}

		public void SwapWeapon()
		{
			API.TaskSwapWeapon(_ped.Handle, false);
		}

		public void TurnTo(Entity target, int duration = -1)
		{
			API.TaskTurnPedToFaceEntity(_ped.Handle, target.Handle, duration);
		}

		public void TurnTo(Vector3 position, int duration = -1)
		{
			API.TaskTurnPedToFaceCoord(_ped.Handle, position.X, position.Y, position.Z, duration);
		}

		public void UseParachute()
		{
			API.TaskParachute(_ped.Handle, true);
		}
		public void UseMobilePhone()
		{
			API.TaskUseMobilePhone(_ped.Handle, 1);
		}
		public void UseMobilePhone(int duration)
		{
			API.TaskUseMobilePhoneTimed(_ped.Handle, duration);
		}
		public void PutAwayParachute()
		{
			API.TaskParachute(_ped.Handle, false);
		}
		public void PutAwayMobilePhone()
		{
			API.TaskUseMobilePhone(_ped.Handle, 0);
		}

		public void VehicleChase(Ped target)
		{
			API.TaskVehicleChase(_ped.Handle, target.Handle);
		}
		public void VehicleShootAtPed(Ped target)
		{
			API.TaskVehicleShootAtPed(_ped.Handle, target.Handle, 20f);
		}

		public void Wait(int duration)
		{
			API.TaskPause(_ped.Handle, duration);
		}

		public void WanderAround()
		{
			API.TaskWanderStandard(_ped.Handle, 0, 0);
		}
		public void WanderAround(Vector3 position, float radius)
		{
			API.TaskWanderInArea(_ped.Handle, position.X, position.Y, position.Z, radius, 0, 0);
		}

		public void WarpIntoVehicle(Vehicle vehicle, VehicleSeat seat)
		{
			API.TaskWarpPedIntoVehicle(_ped.Handle, vehicle.Handle, (int)seat);
		}
		public void WarpOutOfVehicle(Vehicle vehicle)
		{
			API.TaskLeaveVehicle(_ped.Handle, vehicle.Handle, 16);
		}

		public void ClearAll()
		{
			API.ClearPedTasks(_ped.Handle);
		}
		public void ClearAllImmediately()
		{
			API.ClearPedTasksImmediately(_ped.Handle);
		}
		public void ClearLookAt()
		{
			API.TaskClearLookAt(_ped.Handle);
		}
		public void ClearSecondary()
		{
			API.ClearPedSecondaryTask(_ped.Handle);
		}
		public void ClearAnimation(string animSet, string animName)
		{
			API.StopAnimTask(_ped.Handle, animSet, animName, -4f);
		}
	}

	public sealed class TaskSequence : IDisposable
	{
		#region Fields
		static Ped _nullPed = null;
		#endregion


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

		private void Create()
		{
			int handle = 0;

			API.OpenSequenceTask(ref handle);

			Handle = handle;
		}

		public void Dispose()
		{
			int handle = Handle;
			API.ClearSequenceTask(ref handle);
			Handle = 0;
			GC.SuppressFinalize(this);
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

			API.SetSequenceToRepeat(Handle, repeat);
			API.CloseSequenceTask(Handle);

			IsClosed = true;
		}
	}
}
