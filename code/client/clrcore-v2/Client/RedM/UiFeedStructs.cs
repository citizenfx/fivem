using System;
using System.Runtime.InteropServices;
using System.Security;
using CitizenFX.Core;
using CitizenFX.Core.Native;
using CitizenFX.RedM.Native;

namespace CitizenFX.RedM
{
	public struct UiFeedInfo {
		internal Int64 m_duration;
		internal IntPtr m_varString1;
		internal IntPtr m_varString2;
		
		public UiFeedInfo(Int64 dur) {
			m_duration = dur;
			m_varString1 = IntPtr.Zero;
			m_varString2 = IntPtr.Zero;
		}
		
		public UiFeedInfo(Int64 dur, VarString varString1, VarString? varString2)
		{
			m_duration = dur;
			m_varString1 = varString1.m_varStringPtr;
			m_varString2 = varString2?.m_varStringPtr ?? IntPtr.Zero;
		}
	};
	
	[StructLayout(LayoutKind.Explicit, Size = 0x68)]
	[SecuritySafeCritical]
	internal unsafe struct UnsafeUiInfo
	{
		[FieldOffset(0x00)] private Int64 m_duration;
		
		[FieldOffset(0x08)] private IntPtr m_unk1;
		
		[FieldOffset(0x10)] private IntPtr m_unk2;
		
		[FieldOffset(0x18)] private Int64 m_unk3;
		
		[FieldOffset(0x20)] private Int64 m_unk4;
		
		[FieldOffset(0x28)] private Int64 m_unk5;
		
		[FieldOffset(0x30)] private IntPtr m_unk6;
		
		[FieldOffset(0x38)] private Int64 m_unk7;
		
		[FieldOffset(0x40)] private Int64 m_unk8;
		
		[FieldOffset(0x48)] private Int64 m_unk9;
		
		[FieldOffset(0x50)] private Int64 m_unk10;
		
		[FieldOffset(0x58)] private Int64 m_unk11;
		
		[FieldOffset(0x60)] private Int64 m_unk12;
		
		public UnsafeUiInfo(UiFeedInfo feedInfo) { 
			m_duration = feedInfo.m_duration;
			m_unk1 = IntPtr.Zero;
			m_unk2 = IntPtr.Zero;
			m_unk3 = 0;
			m_unk4 = 0;
			m_unk5 = 0;
			m_unk6 = IntPtr.Zero;
			m_unk7 = 0;
			m_unk8 = 0;
			m_unk9 = 0;
			m_unk10 = 0;
			m_unk11 = 0;
			m_unk12 = 0;
		}
	}
	
	public struct VarString 
	{
		internal IntPtr m_varStringPtr;
		public VarString(params Argument[] arguments)
		{
			m_varStringPtr = Natives.Call<IntPtr>(Hash.VAR_STRING, arguments);
		}
	}
	
	public struct UiFeedData
	{
		internal VarString m_varString1;
		internal VarString? m_varString2;
		internal uint m_iconDict;
		internal uint m_icon;
		internal uint m_color;
		
		public UiFeedData(VarString varStr1, VarString? varStr2, string iconDict = null, string icon = null, string color = null)
		{
			m_varString1 = varStr1;
			m_varString2 = varStr2;
			m_iconDict = Game.GenerateHash(iconDict);
			m_icon = Game.GenerateHash(icon);
			m_color = Game.GenerateHash(color);
		}
	}
	
	
	[StructLayout(LayoutKind.Explicit, Size = 0x40)]
	[SecuritySafeCritical]
	internal unsafe struct UnsafeUiFeedNotification
	{
		[FieldOffset(0x00)] Int64 unk1;
		
		[FieldOffset(0x08)] IntPtr varStringPtr1;
		
		[FieldOffset(0x10)] IntPtr varStringPtr2;
		
		[FieldOffset(0x18)] Int64 unk2;
		
		[FieldOffset(0x20)] UInt64 iconDictHash;
		
		[FieldOffset(0x28)] UInt64 iconHash;
		
		[FieldOffset(0x30)] UInt64 colorHash;
		
		[FieldOffset(0x38)] Int64 unk3;
		
		public UnsafeUiFeedNotification(UiFeedData fd) {
			unk1 = 0;
			varStringPtr1 = fd.m_varString1.m_varStringPtr;
			varStringPtr2 = fd.m_varString2?.m_varStringPtr ?? IntPtr.Zero;
			unk2 = 0;
			iconDictHash = fd.m_iconDict;
			iconHash = fd.m_icon;
			colorHash = fd.m_color;
			unk3 = 0;
		}
	}
}
