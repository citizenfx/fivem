/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <StdInc.h>

class GMemoryHeap
{
public:
	virtual void m_00() = 0;
	virtual void m_08() = 0;
	virtual void m_10() = 0;
	virtual void m_18() = 0;
	virtual void m_20() = 0;
	virtual void m_28() = 0;
	virtual void m_30() = 0;
	virtual void m_38() = 0;
	virtual void m_40() = 0;
	virtual void m_48() = 0;
	virtual void* Alloc(uint32_t size) = 0;
	virtual void m_58() = 0;
	virtual void Free(void* memory) = 0;
};

// Unification of GRefCountBase and GRefCountBaseNTS. Once the Scaleform bits
// are further researched this will require splitting up.
class GRefCountBase
{
public:
	virtual ~GRefCountBase() = default;

	void* operator new(size_t size);
	void operator delete(void* ptr);
	void operator delete[](void* ptr);

private:
	volatile long RefCount;
};

class GFxObjectInterface;
class GFxValue;

class GFxFunctionHandler : public GRefCountBase
{
public:
	struct Params
	{
		GFxValue* pRetVal;
		void* pMovie;
		GFxValue* pThis;
		GFxValue* pArgsWithThisRef;
		GFxValue* pArgs;
		uint32_t ArgCount;
		void* pUserData;
	};

	virtual ~GFxFunctionHandler() {}
	virtual void Call(const Params& params) = 0;
};

class GFxMovieRoot
{
public:
	virtual void m_00() = 0;
	virtual void m_08() = 0;
	virtual void m_10() = 0;
	virtual void m_18() = 0;
	virtual void m_20() = 0;
	virtual void m_28() = 0;
	virtual void m_30() = 0;
	virtual void m_38() = 0;
	virtual void m_40() = 0;
	virtual void m_48() = 0;
	virtual void m_50() = 0;
	virtual void m_58() = 0;
	virtual void m_60() = 0;
	virtual void m_68() = 0;
	virtual void m_70() = 0;
	virtual void CreateFunction(GFxValue* value, GFxFunctionHandler* pfc, void* puserData = nullptr) = 0;
	virtual void SetVariable(const char* path, const GFxValue& value, int type) = 0;
};

class GFxValue
{
public:
	struct DisplayInfo
	{
		double X;
		double Y;
		double Rotation;
		double XScale;
		double YScale;
		double Alpha;
		bool Visible;
		double Z;
		double XRotation;
		double YRotation;
		double ZScale;
		double FOV;
		float ViewMatrix3D[4][4];
		float ProjectionMatrix3D[4][4];
		uint16_t VarsSet;
	};

protected:
	enum ValueTypeControl
	{
		VTC_ConvertBit = 0x80,
		VTC_ManagedBit = 0x40,

		VTC_TypeMask = VTC_ConvertBit | 0x0F,
	};

public:
	enum ValueType
	{
		VT_Undefined = 0x00,
		VT_Null = 0x01,
		VT_Boolean = 0x02,
		VT_Number = 0x03,
		VT_String = 0x04,
		VT_StringW = 0x05,
		VT_Object = 0x06,
		VT_Array = 0x07,
		VT_DisplayObject = 0x08,

		VT_ConvertBoolean = VTC_ConvertBit | VT_Boolean,
		VT_ConvertNumber = VTC_ConvertBit | VT_Number,
		VT_ConvertString = VTC_ConvertBit | VT_String,
		VT_ConvertStringW = VTC_ConvertBit | VT_StringW
	};

	union ValueUnion
	{
		double NValue;
		bool BValue;
		const char* pString;
		const char** pStringManaged;
		const wchar_t* pStringW;
		void* pData;
	};

	inline GFxValue()
	{
		pObjectInterface = nullptr;
		Type = VT_Undefined;
		mValue.pData = nullptr;
	}

	inline GFxValue(const char* str)
	{
		pObjectInterface = nullptr;
		Type = VT_String;
		mValue.pString = str;
	}

	inline ~GFxValue()
	{
		if (Type & VTC_ManagedBit)
		{
			ReleaseManagedValue();
		}
	}

	inline ValueType GetType() const
	{
		return ValueType(Type & VTC_TypeMask);
	}

	inline bool IsDisplayObject() const
	{
		return (Type & VTC_TypeMask) == VT_DisplayObject;
	}

	inline double GetNumber() const
	{
		assert(GetType() == VT_Number);
		return mValue.NValue;
	}

	inline const char* GetString() const
	{
		assert((Type & VTC_TypeMask) == VT_String);
		return (Type & VTC_ManagedBit) ? *mValue.pStringManaged : mValue.pString;
	}

	void ReleaseManagedValue();
	bool CreateEmptyMovieClip(GFxValue* movieClip, const char* instanceName, int depth);
	bool GetDisplayInfo(DisplayInfo* info);
	bool SetDisplayInfo(const DisplayInfo& info);
	bool Invoke(const char* name, GFxValue* presult, const GFxValue* pargs, int nargs);
	bool GetMember(const char* name, GFxValue* pval) const;

private:
	GFxObjectInterface* pObjectInterface;
	ValueType Type;
	ValueUnion mValue;
};
