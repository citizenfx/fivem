#pragma once

#include "CrossBuildRuntime.h"
#include <boost/preprocessor/slot/counter.hpp>

namespace xbr::virt
{
template<int BaseCounter, int WatershedBuild, ptrdiff_t Start, ptrdiff_t Offset>
class Base
{
private:
	static inline ptrdiff_t ms_offset;
	static inline InitFunction ms_initFunction{
		[] {
			ms_offset = xbr::IsGameBuildOrGreater<WatershedBuild>() ? (Offset * sizeof(void*)) : 0;
		}
	};

	void* m_vtbl;

protected:
	template<ptrdiff_t Idx>
	auto GetDispatch(void* self)
	{
		constexpr auto OffsetIdx = (Idx - BaseCounter) * 8;
		auto vtbl = *(char**)self;
		auto idx = OffsetIdx;

		if constexpr (OffsetIdx >= Start)
		{
			idx += ms_offset;
		}

		return *(void**)(&vtbl[idx]);
	}

public:
	// no-op: we actually delete in the original deleting destructor
	inline void operator delete(void*)
	{
	}

	inline void operator delete[](void*)
	{
	}
};

template<typename>
struct PrependArg
{

};

template<typename TRet, typename... TArgs>
struct PrependArg<TRet (*)(TArgs...)>
{
	using Type = TRet(__thiscall*)(void*, TArgs...);
};

template<typename T>
using PrependArgT = typename PrependArg<T>::Type;
}

#define XBR_VIRTUAL_BASE(WatershedBuild, Start, Offset) \
	public xbr::virt::Base<__COUNTER__ + 1, WatershedBuild, Start, Offset>

#define XBR_VIRTUAL_BASE_2802(Start) \
	public xbr::virt::Base<__COUNTER__ + 1, 2802, Start, 6>

#if !defined(__EDG__) && defined(GTA_FIVE)
#define XBR_VIRTUAL_METHOD(Return, Name, Args) \
	private: \
		using XBRDispatch_##Name = Return (*) Args; \
	\
	public: \
		template<typename... TArgs> \
		inline Return Name(TArgs&&... args) \
		{ \
			auto dispatch = (xbr::virt::PrependArgT<XBRDispatch_##Name>)GetDispatch<__COUNTER__>(this); \
			return dispatch(this, std::forward<TArgs>(args)...); \
		}
	
#define XBR_VIRTUAL_DTOR(Name) \
	public: \
	inline ~Name()             \
	{                          \
		auto dispatch = (void(__thiscall*)(void*, int))GetDispatch<__COUNTER__>(this); \
		dispatch(this, 1); \
	}
#else
#define XBR_VIRTUAL_METHOD(Return, Name, Args) \
	virtual Return Name Args = 0;

#define XBR_VIRTUAL_DTOR(Name) \
	virtual ~Name() = 0;
#endif
