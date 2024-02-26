/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <StdInc.h>

struct GRectF
{
	float left, top, right, bottom;
};

struct GMatrix2D
{
	float matrix[2][3];

	inline GMatrix2D()
	{
		memset(matrix, 0, sizeof(matrix));
		matrix[0][0] = 1.0f;
		matrix[1][1] = 1.0f;
	}

	inline void AppendScaling(float sx, float sy)
	{
		matrix[0][0] *= sx;
		matrix[0][1] *= sx;
		matrix[0][2] *= sx;
		matrix[1][0] *= sy;
		matrix[1][1] *= sy;
		matrix[1][2] *= sy;
	}

	inline void AppendTranslation(float dx, float dy)
	{
		matrix[0][2] += dx;
		matrix[1][2] += dy;
	}

	inline void Prepend(const GMatrix2D& m)
	{
		GMatrix2D t = *this;
		matrix[0][0] = t.matrix[0][0] * m.matrix[0][0] + t.matrix[0][1] * m.matrix[1][0];
		matrix[1][0] = t.matrix[1][0] * m.matrix[0][0] + t.matrix[1][1] * m.matrix[1][0];
		matrix[0][1] = t.matrix[0][0] * m.matrix[0][1] + t.matrix[0][1] * m.matrix[1][1];
		matrix[1][1] = t.matrix[1][0] * m.matrix[0][1] + t.matrix[1][1] * m.matrix[1][1];
		matrix[0][2] = t.matrix[0][0] * m.matrix[0][2] + t.matrix[0][1] * m.matrix[1][2] + t.matrix[0][2];
		matrix[1][2] = t.matrix[1][0] * m.matrix[0][2] + t.matrix[1][1] * m.matrix[1][2] + t.matrix[1][2];
	}
};

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

	char pad_08[0xB8];
	bool useLocks;
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

	inline void AddRef()
	{
		InterlockedIncrement(&RefCount);
	}

	inline void Release()
	{
		if (!InterlockedDecrement(&RefCount))
		{
			delete this;
		}
	}

private:
	volatile long RefCount;
};

class GRenderer
{
public:
	struct CXform
	{
		float matrix[4][2];

		CXform()
		{
			matrix[0][0] = 1.0f;
			matrix[1][0] = 1.0f;
			matrix[2][0] = 1.0f;
			matrix[3][0] = 1.0f;
			matrix[0][1] = 0.0f;
			matrix[1][1] = 0.0f;
			matrix[2][1] = 0.0f;
			matrix[3][1] = 0.0f;
		}
	};
};

class GFxObjectInterface;
class GFxMovieDef;
class GFxValue;

class GFxCharacter : public GRefCountBase
{
};

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

class GFxTextImageDesc : public GRefCountBase
{
public:
	void* imageShape;
	GFxCharacter* spriteShape;
	int baseLineX;
	int baseLineY;
	uint32_t screenWidth;
	uint32_t screenHeight;
	GMatrix2D matrix;
};

class GFxStyledText : public GRefCountBase
{
public:
	struct HTMLImageTagInfo
	{
		GFxTextImageDesc* textImageDesc;
		char pad[32];
		uint32_t Width, Height;
	};
};

class GFxMovieRoot : public GRefCountBase
{
public:
	// virtual void m_00() = 0;
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

public:
	char pad_10[0x28];
	GMemoryHeap *pHeap;
	char pad_40[0x88];
	GRectF VisibleFrameRect;
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

class GFxSprite : public GFxCharacter
{
public:
	// virtual void m0() = 0;
	virtual void m1() = 0;
	virtual void m2() = 0;
	virtual void m3() = 0;
	virtual void m4() = 0;
	virtual void m5() = 0;
	virtual void m6() = 0;
	virtual void m7() = 0;
	virtual void m8() = 0;
	virtual void m9() = 0;
	virtual void m10() = 0;
	virtual void m11() = 0;
	virtual void m12() = 0;
	virtual void m13() = 0;
	virtual void m14() = 0;
	virtual void m15() = 0;
	virtual GRectF GetRectBounds(const GMatrix2D& matrix) = 0;
	virtual void m17() = 0;
	virtual void m18() = 0;
	virtual void m19() = 0;
	virtual void m20() = 0;
	virtual void m21() = 0;
	virtual void m22() = 0;
	virtual void m23() = 0;
	virtual void m24() = 0;
	virtual void m25() = 0;
	virtual void m26() = 0;
	virtual void m27() = 0;
	virtual void m28() = 0;
	virtual void Display(void* context) = 0;
	virtual void Restart() = 0;
};

struct GFxDisplayContext
{
	GRenderer::CXform* parentCxform;
	GMatrix2D* parentMatrix;
};

struct GFxTextFormat
{
	char pad[66];
	uint16_t presentMask;
	char pad2[12];
};

struct GFxElemDesc
{
	char pad[32];
	GFxTextFormat fmt;
	char paraFmt[24];
};

class GFxResource
{
public:
	virtual void m_0() = 0;
	virtual void m_1() = 0;
	virtual int GetType() = 0;

	inline bool IsSprite()
	{
		return ((GetType() >> 8) & 0xff) == 0x84;
	}
};

class GFxSpriteDef : public GFxResource
{
public:
	virtual void m_s0() = 0;
	virtual void m_s1() = 0;
	virtual void m_s2() = 0;
	virtual void m_s3() = 0;
	virtual void m_s4() = 0;
	virtual void m_s5() = 0;
	virtual void* CreateCharacterInstance(void* parent, uint32_t* id, void* pbindingImpl) = 0;
};

#ifdef _DEBUG
static_assert(offsetof(GFxMovieRoot, pHeap) == 56);
static_assert(offsetof(GFxMovieRoot, VisibleFrameRect) == 200);
static_assert(offsetof(GMemoryHeap, useLocks) == 192);
#endif
