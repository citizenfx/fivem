#if MONO_V2
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{
	public class PedBone : EntityBone
	{
		#region Fields
		private new readonly Ped _owner;
		#endregion

		public new Ped Owner
		{
			get { return _owner; }
		}

		internal PedBone(Ped owner, int boneIndex) : base(owner, boneIndex)
		{
			_owner = owner;
		}

		internal PedBone(Ped owner, string boneName) : base(owner, boneName)
		{
			_owner = owner;
		}

		internal PedBone(Ped owner, Bone boneId)
			: base(owner, API.GetPedBoneIndex(owner.Handle, (int)boneId))
		{
			_owner = owner;
		}

		public new bool IsValid
		{
			get
			{
				return Ped.Exists(Owner) && Index != -1;
			}
		}
	}
}
