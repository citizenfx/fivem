/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

// CFX core object model base header file.

// Generic result type.
typedef uint32_t result_t;

// Common error code definitions
#define FX_S_OK					0x0
#define FX_E_NOTIMPL			0x80004001
#define FX_E_NOINTERFACE		0x80004002
#define FX_E_INVALIDARG			0x80070057

// Success/failure macros
#define FX_SUCCEEDED(x)			(((x) & 0x80000000) == 0)
#define FX_FAILED(x)			(!FX_SUCCEEDED(x))

// GUID type
struct guid_t
{
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t  data4[8];
};

// definition macro
#define FX_DEFINE_GUID(name, d1, d2, d3, a, b, c, d, e, f, g, h) \
	static const guid_t name = { d1, d2, d3, { a, b, c, d, e, f, g, h} };

// helper functions
namespace fx
{
// null GUID
inline guid_t GetNullGuid()
{
	return guid_t{ 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0} };
}

// GUID equivalence testing
inline bool GuidEquals(const guid_t* left, const guid_t* right)
{
	static_assert(std::is_pod_v<guid_t>, "guid isn't a POD type");

	return (memcmp(left, right, sizeof(guid_t)) == 0);
}

// GUID equivalence testing, C++ edition
inline bool GuidEquals(const guid_t& left, const guid_t& right)
{
	return (memcmp(&left, &right, sizeof(guid_t)) == 0);
}

// checking for null GUID
inline bool IsNullGuid(const guid_t& guid)
{
	return (GuidEquals(guid, GetNullGuid()));
}
}

// and operators, too
inline bool operator==(const guid_t& left, const guid_t& right)
{
	return (fx::GuidEquals(left, right));
}

inline bool operator!=(const guid_t& left, const guid_t& right)
{
	return !(left == right);
}

inline bool operator<(const guid_t& left, const guid_t& right)
{
	return (memcmp(&left, &right, sizeof(right)) < 0);
}

// definitions for various things used by m.o's IDL compiler
#ifdef _MSC_VER
#define NS_NO_VTABLE __declspec(novtable)
#else
#define NS_NO_VTABLE
#endif

#define NS_ERROR_NULL_POINTER			FX_E_INVALIDARG
#define NS_ERROR_NOT_IMPLEMENTED		FX_E_NOTIMPL

#define NS_DECLARE_STATIC_IID_ACCESSOR(iid_const) \
	static inline guid_t GetIID() { return iid_const; }

#ifdef _WIN32
#define OM_DECL __stdcall
#else
#define OM_DECL
#endif

#define NS_IMETHOD_(rv) virtual rv OM_DECL
#define NS_IMETHOD NS_IMETHOD_(result_t)

#define NS_IMETHODIMP_(rv) virtual rv
#define NS_IMETHODIMP NS_IMETHODIMP_(result_t)

// no-op
#define NS_DEFINE_STATIC_IID_ACCESSOR(x, y)

// Start a search, returning the first class implementation matching the IID.
extern "C" intptr_t fxFindFirstImpl(const guid_t& iid, guid_t* clsid);

// Continue a fxFindFirstImpl search.
extern "C" int32_t fxFindNextImpl(intptr_t findHandle, guid_t* clsid);

// Close a fxFindFirstImpl handle.
extern "C" void fxFindImplClose(intptr_t findHandle);

// Instance creation routine
extern "C" result_t fxCreateObjectInstance(const guid_t& guid, const guid_t& iid, void** objectRef);

#ifndef FXOM_NO_HELPERS
// C++ helpers one may want
#include <om/IBase.h>

#include <om/OMPtr.h>
#include <om/OMClass.h>

// OMComponent helpers
#define FX_IMPLEMENTS(clsid, iid) \
	static OMImplements _implements##clsid##iid(clsid, iid::GetIID());

#define FX_NEW_FACTORY(type) \
	static OMFactoryDefinition _factory##type(CLSID_##type, fx::MakeNewBase<type>);
#endif