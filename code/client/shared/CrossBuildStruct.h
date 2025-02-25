#pragma once

#include "CrossBuildRuntime.h"

template<int... Versions>
using VersionList = std::integer_sequence<int, Versions...>;

template<typename Versions1, typename Version2, typename Versions = VersionList<>>
struct CombineVersions;

// Left sequence empty
template<int... Versions2, int... Versions>
struct CombineVersions<VersionList<>, VersionList<Versions2...>, VersionList<Versions...>>
{
	using type = VersionList<Versions..., Versions2...>;
};

// Right sequence empty
template<int... Versions1, int... Versions>
struct CombineVersions<VersionList<Versions1...>, VersionList<>, VersionList<Versions...>>
{
	using type = VersionList<Versions..., Versions1...>;
};

// Front values equal
template<int Version, int... Versions1, int... Versions2, int... Versions>
struct CombineVersions<VersionList<Version, Versions1...>, VersionList<Version, Versions2...>, VersionList<Versions...>>
	: CombineVersions<VersionList<Versions1...>, VersionList<Versions2...>, VersionList<Versions..., Version>>
{
};

template<int Version1, int... Versions1, int Version2, int... Versions2, int... Versions>
struct CombineVersions<VersionList<Version1, Versions1...>, VersionList<Version2, Versions2...>, VersionList<Versions...>>
	: std::conditional<(Version1 < Version2),
	  CombineVersions<VersionList<Versions1...>, VersionList<Version2, Versions2...>, VersionList<Versions..., Version1>>,
	  CombineVersions<VersionList<Version1, Versions1...>, VersionList<Versions2...>, VersionList<Versions..., Version2>>>::type
{
};

template<typename Self, template<int> typename Multi, typename Versions>
struct CrossBuildVisitor
{
	template<typename Visitor>
	static inline decltype(auto) Visit(Self* ptr, Visitor&& vis)
	{
		return Visit(ptr, std::forward<Visitor>(vis), xbr::GetGameBuild(), Versions{});
	}

	template<int Version, typename Visitor>
	static inline decltype(auto) Visit(Self* ptr, Visitor&& vis, int /*version*/, VersionList<Version>)
	{
		return vis(reinterpret_cast<Multi<Version>*>(ptr));
	}

	template<int Version1, int Version2, int... ExtraVersions, typename Visitor>
	static inline decltype(auto) Visit(Self* ptr, Visitor&& vis, int version, VersionList<Version1, Version2, ExtraVersions...>)
	{
		if (version < Version2)
		{
			return vis(reinterpret_cast<Multi<Version1>*>(ptr));
		}

		return Visit(ptr, std::forward<Visitor>(vis), version, VersionList<Version2, ExtraVersions...>{});
	}
};

template<typename Self>
struct CrossBuildStruct;

template<typename Self, template<int Version> typename Multi, typename _Versions>
struct CrossBuildStructInfoBase
{
	using Versions = _Versions;
	using Visitor = CrossBuildVisitor<Self, Multi, Versions>;
};

template<typename Self, template<int Version> typename Multi, int... Versions>
struct CrossBuildStructInfo : CrossBuildStructInfoBase<Self, Multi, VersionList<0, Versions...>>
{
};

template<typename Self, typename Parent, template<int Version> typename Multi, int... Versions>
struct CrossBuildChildStructInfo
	: CrossBuildStructInfoBase<Self, Multi,
	  typename CombineVersions<typename CrossBuildStruct<Parent>::Versions, VersionList<Versions...>>::type>
{
};

template<int Version, int GE, int LT>
using VersionBetween = typename std::enable_if<(Version >= GE) && (Version < LT)>::type;

template<int Version, int GE>
using VersionAfter = typename std::enable_if<(Version >= GE)>::type;

template<typename Self, typename Func, typename Visitor = typename CrossBuildStruct<Self>::Visitor>
inline decltype(auto) operator->*(Self* self, Func&& func)
{
	return Visitor::Visit(self, std::forward<Func>(func));
}

template<typename T, typename... Args>
static inline T* xbr_new(Args&&... args)
{
	return static_cast<T*>(nullptr)->*[&](auto p)
	{
		using Self = typename std::remove_pointer<decltype(p)>::type;

		return reinterpret_cast<T*>(new Self(std::forward<Args>(args)...));
	};
}

template<typename T>
static inline void xbr_delete(T* ptr)
{
	if (ptr)
	{
		ptr->*[](auto self)
		{
			delete self;
		};
	}
}

#define XBV(...)                              \
	this->*[&](auto self) -> decltype(auto) { \
		return self->__VA_ARGS__;             \
	}
