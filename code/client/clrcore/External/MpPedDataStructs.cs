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

	public enum TattooZoneData
	{
		ZONE_TORSO = 0,
		ZONE_HEAD = 1,
		ZONE_LEFT_ARM = 2,
		ZONE_RIGHT_ARM = 3,
		ZONE_LEFT_LEG = 4,
		ZONE_RIGHT_LEG = 5,
		ZONE_UNKNOWN = 6,
		ZONE_NONE = 7,
	};

	/// <summary>
	/// This is used to GET the data. The GetData() function returns the data into a usable struct for scripts to use safely.
	/// </summary>
	[StructLayout(LayoutKind.Explicit, Size = 0x40)]
	[SecurityCritical]
	internal unsafe struct UnsafeTattooCollectionData
	{
		[FieldOffset(0x00)] private uint unkHash1;

		[FieldOffset(0x08)] private uint unkHash2;

		[FieldOffset(0x10)] private uint tattooCollectionHash;

		[FieldOffset(0x18)] private uint tattooNameHash;

		[FieldOffset(0x20)] private uint unkHash3;

		[FieldOffset(0x28)] private TattooZoneData tattooZoneId;

		[FieldOffset(0x30)] private uint unkHash4;

		[FieldOffset(0x38)] private uint unkHash5;

		public TattooCollectionData GetData()
		{
			return new TattooCollectionData(unkHash1, unkHash2, tattooCollectionHash, tattooNameHash, unkHash3, tattooZoneId, unkHash4, unkHash5);
		}
	}

	public struct TattooCollectionData
	{
		public uint UnkHash1;
		public uint UnkHash2;
		public uint TattooCollectionHash;
		public uint TattooNameHash;
		public uint UnkHash3;
		public TattooZoneData TattooZone;
		public uint UnkHash4;
		public uint UnkHash5;

		public TattooCollectionData(uint unk1, uint unk2, uint collectionHash, uint nameHash, uint unk3, TattooZoneData zone, uint unk4, uint unk5)
		{
			UnkHash1 = unk1;
			UnkHash2 = unk2;
			UnkHash3 = unk3;
			UnkHash4 = unk4;
			UnkHash5 = unk5;
			TattooCollectionHash = collectionHash;
			TattooNameHash = nameHash;
			TattooZone = zone;
		}
	}

}
