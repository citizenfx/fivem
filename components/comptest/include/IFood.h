/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM ../../components/comptest/include/IFood.idl
 */

#ifndef __gen_IFood_h__
#define __gen_IFood_h__


#ifndef __gen_fxIBase_h__
#include "fxIBase.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    IFood */
#define IFOOD_IID_STR "c1360585-ef2f-43ce-99e8-d4daa9100af6"

#define IFOOD_IID \
  {0xc1360585, 0xef2f, 0x43ce, \
    { 0x99, 0xe8, 0xd4, 0xda, 0xa9, 0x10, 0x0a, 0xf6 }}

class NS_NO_VTABLE IFood : public fxIBase {
 public:

  NS_DECLARE_STATIC_IID_ACCESSOR(IFOOD_IID)

  /* void Eat (in int32_t amount); */
  NS_IMETHOD Eat(int32_t amount) = 0;

  /* void SetSubFood (in IFood food); */
  NS_IMETHOD SetSubFood(IFood *food) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(IFood, IFOOD_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_IFOOD \
  NS_IMETHOD Eat(int32_t amount) override; \
  NS_IMETHOD SetSubFood(IFood *food) override; 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_IFOOD(_to) \
  NS_IMETHOD Eat(int32_t amount) override { return _to Eat(amount); } \
  NS_IMETHOD SetSubFood(IFood *food) override { return _to SetSubFood(food); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_IFOOD(_to) \
  NS_IMETHOD Eat(int32_t amount) override { return !_to ? NS_ERROR_NULL_POINTER : _to->Eat(amount); } \
  NS_IMETHOD SetSubFood(IFood *food) override { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSubFood(food); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public IFood
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IFOOD

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS(_MYCLASS_, IFood)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void Eat (in int32_t amount); */
NS_IMETHODIMP _MYCLASS_::Eat(int32_t amount)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void SetSubFood (in IFood food); */
NS_IMETHODIMP _MYCLASS_::SetSubFood(IFood *food)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_IFood_h__ */
