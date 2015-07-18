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
	class OMClassParent<_DECLARE_OM_PARENT_SPEC(x)> : _DECLARE_OM_PARENT_BASES(x) {};

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
		std::atomic<uint32_t> m_count;

	public:
		RefCount()
			: m_count(0)
		{
		}

		inline std::atomic<uint32_t>& GetCount()
		{
			return m_count;
		}
	};

	struct pass
	{
		template<typename ...T> pass(T...) {}
	};

	RefCount m_refCount;

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

		return result;
	}

	virtual uint32_t AddRef() override
	{
		return ++m_refCount.GetCount();
	}

	virtual uint32_t Release() override
	{
		uint32_t c = m_refCount.GetCount().fetch_sub(1);

		if (c <= 1)
		{
			delete this;
			return true;
		}

		return false;
	}
};
}