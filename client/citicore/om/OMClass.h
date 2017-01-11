/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/repeat.hpp>

namespace fx
{
template<class... T> class OMClassParent;

#define _DECLARE_CLASS_ENTRY(z, n, text) BOOST_PP_COMMA_IF(n) text T ## n

#define _DECLARE_OM_PARENT_TEMPLATE(x) BOOST_PP_REPEAT(x, _DECLARE_CLASS_ENTRY, class)
#define _DECLARE_OM_PARENT_BASES(x) BOOST_PP_REPEAT(x, _DECLARE_CLASS_ENTRY, public)
#define _DECLARE_OM_PARENT_SPEC(x) BOOST_PP_REPEAT(x, _DECLARE_CLASS_ENTRY, )
#define DECLARE_OM_PARENT(x) template<_DECLARE_OM_PARENT_TEMPLATE(x)> \
	class OMClassParent<_DECLARE_OM_PARENT_SPEC(x)> : _DECLARE_OM_PARENT_BASES(x) { \
		protected: \
			fxIBase* GetBaseRef() { return static_cast<T0*>(this); } \
	};

DECLARE_OM_PARENT(1)
DECLARE_OM_PARENT(2)
DECLARE_OM_PARENT(3)
DECLARE_OM_PARENT(4)
DECLARE_OM_PARENT(5)
DECLARE_OM_PARENT(6)
DECLARE_OM_PARENT(7)
DECLARE_OM_PARENT(8)

template<class TClass, class... TInterface>
class OMClass : public OMClassParent<TInterface...>
{
private:
	class RefCount
	{
	private:
		uint32_t m_count;

	public:
		RefCount()
			: m_count(0)
		{
		}

		inline uint32_t& GetCount()
		{
			return m_count;
		}
	};

	struct pass
	{
		template<typename ...T> pass(T...) {}
	};

	RefCount m_refCount;

protected:
	OMClass() {}

	virtual ~OMClass() {}

private:
	void* operator new(size_t size)
	{
		return nullptr;
	}

	void* operator new(size_t size, void* ptr)
	{
		return ptr;
	}

public:
	template<typename TClass, typename... TArg>
	friend OMPtr<TClass> MakeNew(TArg...);

	template<typename TClass, typename... TArg>
	friend fxIBase* MakeNewBase(TArg...);

public:
	virtual result_t QueryInterface(const guid_t& riid, void** outObject) override
	{
		result_t result = FX_E_NOINTERFACE;

		pass{ ([&]
		{
			if (result == FX_E_NOINTERFACE)
			{
				if (riid == TInterface::GetIID())
				{
					result = FX_S_OK;
					*outObject = static_cast<TInterface*>(this);

					AddRef();
				}
			}
		}(), 1)... };

		if (result == FX_E_NOINTERFACE)
		{
			if (riid == guid_t{ 0x00000000, 0x0000, 0x0000,{ 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } })
			{
				result = FX_S_OK;
				*outObject = this;

				AddRef();
			}
		}

		return result;
	}

	virtual uint32_t AddRef() override
	{
#ifdef OM_REF_DEBUG
		if (m_refCount.GetCount() == 0)
		{
			trace(__FUNCTION__ " ref'ing %p\n", this);
		}
#endif

        return InterlockedIncrement(&m_refCount.GetCount());
	}

	virtual uint32_t Release() override
	{
        auto c = InterlockedDecrement(&m_refCount.GetCount());

		if (c <= 0)
		{
#ifdef OM_REF_DEBUG
			trace(__FUNCTION__ " deleting %p\n", this);
#endif

			this->~OMClass();
			fwFree(this);

			return true;
		}

		return false;
	}
};

template<typename TClass, typename... TArg>
fxIBase* MakeNewBase(TArg... args)
{
	TClass* inst = new(fwAlloc(sizeof(TClass))) TClass(args...);
	inst->AddRef();

	return inst->GetBaseRef();
}

template<typename TClass, typename... TArg>
OMPtr<TClass> MakeNew(TArg... args)
{
	OMPtr<TClass> retval;
	TClass* inst = new(fwAlloc(sizeof(TClass))) TClass(args...);
	
	inst->AddRef();
	*(retval.GetAddressOf()) = inst;

	return retval;
}
}