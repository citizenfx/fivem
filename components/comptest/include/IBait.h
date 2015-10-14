/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM ../../components/comptest/include/IBait.idl
 */

#ifndef __gen_IBait_h__
#define __gen_IBait_h__


#ifndef __gen_fxIBase_h__
#include "fxIBase.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    IBait */
#define IBAIT_IID_STR "47c905f1-266e-4f85-bfa6-0ba3c8b27d10"

#define IBAIT_IID \
  {0x47c905f1, 0x266e, 0x4f85, \
    { 0xbf, 0xa6, 0x0b, 0xa3, 0xc8, 0xb2, 0x7d, 0x10 }}

class NS_NO_VTABLE IBait : public fxIBase {
 public:

  NS_DECLARE_STATIC_IID_ACCESSOR(IBAIT_IID)

  /* void Bait (in int32_t amount); */
  NS_IMETHOD Bait(int32_t amount) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(IBait, IBAIT_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_IBAIT \
  NS_IMETHOD Bait(int32_t amount) override; 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_IBAIT(_to) \
  NS_IMETHOD Bait(int32_t amount) override { return _to Bait(amount); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_IBAIT(_to) \
  NS_IMETHOD Bait(int32_t amount) override { return !_to ? NS_ERROR_NULL_POINTER : _to->Bait(amount); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public IBait
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IBAIT

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS(_MYCLASS_, IBait)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void Bait (in int32_t amount); */
NS_IMETHODIMP _MYCLASS_::Bait(int32_t amount)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_IBait_h__ */
