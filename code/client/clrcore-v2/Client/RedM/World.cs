using CitizenFX.Core;
using CitizenFX.RedM.Native;

namespace CitizenFX.RedM
{
	public static class World
	{
		/// <summary>
		/// Spawns a <see cref="Vehicle"/> of the given <see cref="Model"/> at the position and heading specified
		/// </summary>
		public static async Coroutine<Vehicle> CreateVehicle(Model model, Vector3 pos, float heading, bool shouldNetwork = true, bool registerToScript = true, bool createDraftAnimals = true, bool unk = false)
		{
			if (!model.IsVehicle && !model.IsDraftVehicle && !await model.Request())
			{
				return null;
			}
			
			using (model)
			{
				return new Vehicle(Natives.CreateVehicle(model, pos.X, pos.Y, pos.Z, heading, shouldNetwork, registerToScript, !createDraftAnimals, unk));
			}
		}
	}
}
