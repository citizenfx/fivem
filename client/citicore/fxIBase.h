/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM ../../client/citicore/fxIBase.idl
 */

#ifndef __gen_fxIBase_h__
#define __gen_fxIBase_h__


#ifndef __gen_fxcore_h__
#include "fxcore.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#if 0

/* starting interface:    fxIBase */
#define FXIBASE_IID_STR "00000000-0000-0000-c000-000000000046"

#define FXIBASE_IID \
  {0x00000000, 0x0000, 0x0000, \
    { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }}

class NS_NO_VTABLE fxIBase {
 public:

  NS_DECLARE_STATIC_IID_ACCESSOR(FXIBASE_IID)

  /* void QueryInterface (in fxIIDRef riid, [iid_is (riid), retval] out fxQIResult outObject); */
  NS_IMETHOD QueryInterface(const guid_t & riid, void **outObject) = 0;

  /* [notxpcom] fxrefcnt AddRef (); */
  NS_IMETHOD_(fxrefcnt) AddRef(void) = 0;

  /* [notxpcom] fxrefcnt Release (); */
  NS_IMETHOD_(fxrefcnt) Release(void) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(fxIBase, FXIBASE_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_FXIBASE \
  NS_IMETHOD QueryInterface(const guid_t & riid, void **outObject) override; \
  NS_IMETHOD_(fxrefcnt) AddRef(void) override; \
  NS_IMETHOD_(fxrefcnt) Release(void) override; 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_FXIBASE(_to) \
  NS_IMETHOD QueryInterface(const guid_t & riid, void **outObject) override { return _to QueryInterface(riid, outObject); } \
  NS_IMETHOD_(fxrefcnt) AddRef(void) override { return _to AddRef(); } \
  NS_IMETHOD_(fxrefcnt) Release(void) override { return _to Release(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_FXIBASE(_to) \
  NS_IMETHOD QueryInterface(const guid_t & riid, void **outObject) override { return !_to ? NS_ERROR_NULL_POINTER : _to->QueryInterface(riid, outObject); } \
  NS_IMETHOD_(fxrefcnt) AddRef(void) override; \
  NS_IMETHOD_(fxrefcnt) Release(void) override; 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class fxBase : public fxIBase
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_FXIBASE

  fxBase();

private:
  ~fxBase();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS(fxBase, fxIBase)

fxBase::fxBase()
{
  /* member initializers and constructor code */
}

fxBase::~fxBase()
{
  /* destructor code */
}

/* void QueryInterface (in fxIIDRef riid, [iid_is (riid), retval] out fxQIResult outObject); */
NS_IMETHODIMP fxBase::QueryInterface(const guid_t & riid, void **outObject)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [notxpcom] fxrefcnt AddRef (); */
NS_IMETHODIMP_(fxrefcnt) fxBase::AddRef()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [notxpcom] fxrefcnt Release (); */
NS_IMETHODIMP_(fxrefcnt) fxBase::Release()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#endif
#include <om/IBase.h>

#endif /* __gen_fxIBase_h__ */
