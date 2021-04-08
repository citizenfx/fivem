using System;
using CitizenFX.Core.Native;

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

		public void GoTo(Vector3 position, int timeout = -1)
		{
			API.TaskGoStraightToCoord(_ped.Handle, position.X, position.Y, position.Z, 1f, timeout, 0f, 0f);
		}

		public void HandsUp(int duration)
		{
			API.TaskHandsUp(_ped.Handle, duration, 0, -1, false);
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

		public void PlayAnimation(string animDict, string animName, float blendInSpeed, float blendOutSpeed, int duration, AnimationFlags flags, float playbackRate)
		{
			API.TaskPlayAnim(_ped.Handle, animDict, animName, blendInSpeed, blendOutSpeed, duration, (int)flags, playbackRate, false, false, false);
		}

		public void ReactAndFlee(Ped ped)
		{
			API.TaskReactAndFleePed(_ped.Handle, ped.Handle);
		}

		public void RunTo(Vector3 position, int timeout = -1)
		{
			API.TaskGoStraightToCoord(_ped.Handle, position.X, position.Y, position.Z, 4f, timeout, 0f, 0f);
		}

		public void ShootAt(Ped target, int duration = -1, FiringPattern pattern = FiringPattern.Default)
		{
			API.TaskShootAtEntity(_ped.Handle, target.Handle, duration, (uint)pattern);
		}

		public void ShootAt(Vector3 position, int duration = -1, FiringPattern pattern = FiringPattern.Default)
		{
			API.TaskShootAtCoord(_ped.Handle, position.X, position.Y, position.Z, duration, (uint)pattern);
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
		
		public void ClearSecondary()
		{
			API.ClearPedSecondaryTask(_ped.Handle);
		}
	}
}