using CitizenFX.Core;
using CitizenFX.RedM.Native;


namespace CitizenFX.RedM
{
    public enum Harness : uint
    {
        RearLeft = 0,
        RearRight,
        MiddleLeft,
        MiddleRight,
        FrontLeft,
        FrontRight,
    }

    public enum WheelIndex : uint
    {
        FrontLeft = 0,
        FrontRight = 1,
        BackLeft = 4,
        BackRight = 5,
        Length = 6
    }

    public class Vehicle : Entity, Shared.IVehicle
    {
        public Vehicle(int handle) : base(handle)
        {
        }


        /// <summary>
        /// Returns the <see cref="Vehicle" /> or null if <paramref name="handle"/> is 0
        /// </summary>
        public static Vehicle GetVehicleOrNull(int handle) {
            return handle == 0 ? null : new Vehicle(handle);
        }

        /// <summary>
        /// Returns true if the vehicle is stopped, false otherwise
        /// </summary>
        public bool IsStopped => Natives.IsVehicleStopped(Handle);

        /// <summary>
        /// Returns true if the vehicle is a draft vehicle, false otherwise
        /// </summary>
        // TODO: Swap to actual native call when it doesn't turn ulong
        public bool IsDraft => Natives.Call<bool>(Hash.IS_DRAFT_VEHICLE, Handle);


        /// <summary>
        /// Gets the current driver of the vehicle
        /// </summary>
        /// <value>
        /// The ped in the specified <paramref name="harness"/>, or null if it
        /// doesnt exist.
        /// </value>
        public Ped GetPedInHarness(Harness harness) => Ped.GetPedOrNull(Natives.GetPedInDraftHarness(Handle, (int)harness));

        /// <summary>
        /// Gets the current driver of the vehicle
        /// </summary>
        /// <value>
        /// The ped in the driver seat or null if the Ped didn't exist
        /// </value>
        public Ped Driver => Ped.GetPedOrNull(Natives.GetDriverOfVehicle(Handle));

        // TODO: Test and Document
        // public void AddTemporaryTrainStop(int trackIndex, Vector3 pos)
        // {
        //     Natives.AddTrainTemporaryStop(Handle, trackIndex, pos.X, pos.Y, pos.Z);
        // }

        /// <summary>
        /// Returns true if there are any seats available in the current vehicle
        /// </summary>
        public bool AreAnySeatsAvailable => Natives.AreAnyVehicleSeatsFree(Handle);

        /// <summary>
        /// Attaches the specified <paramref name="ped"/> to the specified <paramref name="harness"/>
        /// **This will not work for networked peds, this also does not get sync'd to
        /// other clients**
        /// </summary>
        /// <value>
        /// Returns true if the ped got attached, false otherwise
        /// </value>
        public bool AttachPedToHarness(Ped ped, Harness harness)
        {
            // TODO: Swap to actual native call when this returns a bool instead of ulong
            return Natives.Call<bool>(Hash._ATTACH_DRAFT_VEHICLE_HARNESS_PED, ped.Handle, Handle, (int)harness);
        }

        /// <summary>
        /// Breaks off the wheel at <paramref name="wheelIndex"/> with <paramref name="destroyingForce"/>
        /// If you want to get
        /// </summary>
        public void BreakOffDraftWheel(WheelIndex wheelIndex, float destroyingForce = 100f)
        {
            Natives.BreakOffDraftWheel(Handle, (int)wheelIndex, destroyingForce);
        }


        /// <summary>
        /// Breaks off the wheel at <paramref name="wheelIndex"/>
        /// </summary>
        /// <value>
        /// Returns the <see cref="Prop" /> for the broken off wheel or null if the wheel didn't exist
        /// </value>
        public Prop BreakOffVehicleWheel(WheelIndex wheelIndex) => Prop.GetPropOrNull(Natives.BreakOffVehicleWheel(Handle, (int)wheelIndex));

        /// <summary>
        /// Halts the vehicle after <paramref name="distance"/> units. The
        /// vehicle will not move until <paramref name="durInSeconds"/> have
        /// passed.
        /// </summary>
        public void HaltVehicle(float distance, int durInSeconds, bool unk)
        {
            Natives.BringVehicleToHalt(Handle, distance, durInSeconds, unk);
        }

		/// <summary>
		/// Fixes the current vehicle, this doesn't make the vehicle able to be remounted if it lost all of its wheels.
		/// </summary>
        public void Fix()
        {
            Natives.SetVehicleFixed(Handle);
        }

        /// <summary>
        /// Fades out this <see cref="Vehicle" /> and deletes & invalidates the
        /// entity handle
        /// </summary>
        public void FadeAndDestroy()
        {
            int handle = Handle;
            IsMissionEntity = true;
            Natives.FadeAndDestroyVehicle(ref handle);
            Handle = handle;
        }
    }
}
