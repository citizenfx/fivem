using System;
using System.Runtime.InteropServices;
using System.Security;

namespace CitizenFX.Core
{
	/// <summary>
	/// This is used to GET the data. The GetData() function returns the data into a usable struct for scripts to use safely.
	/// </summary>
	[StructLayout(LayoutKind.Explicit, Size = 0x50)]
	[SecurityCritical]
	internal unsafe struct UnsafePedHeadBlendData
	{
		[FieldOffset(0x00)] private int firstFaceShape;

		[FieldOffset(0x08)] private int secondFaceShape;

		[FieldOffset(0x10)] private int thirdFaceShape;

		[FieldOffset(0x18)] private int firstSkinTone;

		[FieldOffset(0x20)] private int secondSkinTone;

		[FieldOffset(0x28)] private int thirdSkinTone;

		[FieldOffset(0x30)] private float parentFaceShapePercent;

		[FieldOffset(0x38)] private float parentSkinTonePercent;

		[FieldOffset(0x40)] private float parentThirdUnkPercent;

		[FieldOffset(0x48)] private bool isParentInheritance;

		public PedHeadBlendData GetData()
		{
			return new PedHeadBlendData(firstFaceShape, secondFaceShape, thirdFaceShape, firstSkinTone, secondSkinTone, thirdSkinTone, parentFaceShapePercent, parentSkinTonePercent, parentThirdUnkPercent, isParentInheritance);
		}
	}

	/// <summary>
	/// A struct containing all ped head blend data. Used for MP (freemode) characters.
	/// </summary>
	public struct PedHeadBlendData
	{
		public int FirstFaceShape { get; }
		public int SecondFaceShape { get; }
		public int ThirdFaceShape { get; }
		public int FirstSkinTone { get; }
		public int SecondSkinTone { get; }
		public int ThirdSkinTone { get; }
		public float ParentFaceShapePercent { get; }
		public float ParentSkinTonePercent { get; }
		public float ParentThirdUnkPercent { get; }
		public bool IsParentInheritance { get; }

		public PedHeadBlendData(int firstFaceShape, int secondFaceShape, int thirdFaceShape, int firstSkinTone, int secondSkinTone, int thirdSkinTone, float parentFaceShapePercent, float parentSkinTonePercent, float parentThirdUnkPercent, bool isParentInheritance)
		{
			FirstFaceShape = firstFaceShape;
			SecondFaceShape = secondFaceShape;
			ThirdFaceShape = thirdFaceShape;
			FirstSkinTone = firstSkinTone;
			SecondSkinTone = secondSkinTone;
			ThirdSkinTone = thirdSkinTone;
			ParentFaceShapePercent = parentFaceShapePercent;
			ParentSkinTonePercent = parentSkinTonePercent;
			ParentThirdUnkPercent = parentThirdUnkPercent;
			IsParentInheritance = isParentInheritance;
		}
	}
}
