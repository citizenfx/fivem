/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM ../../components/citizen-scripting-core/include/fxScripting.idl
 */

#ifndef __gen_fxScripting_h__
#define __gen_fxScripting_h__


#ifndef __gen_fxIBase_h__
#include "fxIBase.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
#include "fxNativeContext.h"

/* starting interface:    fxIStream */
#define FXISTREAM_IID_STR "82ec2441-dbb4-4512-81e9-3a98ce9ffcab"

#define FXISTREAM_IID \
  {0x82ec2441, 0xdbb4, 0x4512, \
    { 0x81, 0xe9, 0x3a, 0x98, 0xce, 0x9f, 0xfc, 0xab }}

class NS_NO_VTABLE fxIStream : public fxIBase {
 public:

  NS_DECLARE_STATIC_IID_ACCESSOR(FXISTREAM_IID)

  /* void Read (in voidPtr data, in uint32_t size, out uint32_t bytesRead); */
  NS_IMETHOD Read(void *data, uint32_t size, uint32_t *bytesRead) = 0;

  /* void Write (in voidPtr data, in uint32_t size, out uint32_t bytesWritten); */
  NS_IMETHOD Write(void *data, uint32_t size, uint32_t *bytesWritten) = 0;

  /* void Seek (in int64_t offset, in int32_t origin, out uint64_t newPosition); */
  NS_IMETHOD Seek(int64_t offset, int32_t origin, uint64_t *newPosition) = 0;

  /* void GetLength (out uint64_t length); */
  NS_IMETHOD GetLength(uint64_t *length) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(fxIStream, FXISTREAM_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_FXISTREAM \
  NS_IMETHOD Read(void *data, uint32_t size, uint32_t *bytesRead) override; \
  NS_IMETHOD Write(void *data, uint32_t size, uint32_t *bytesWritten) override; \
  NS_IMETHOD Seek(int64_t offset, int32_t origin, uint64_t *newPosition) override; \
  NS_IMETHOD GetLength(uint64_t *length) override; 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_FXISTREAM(_to) \
  NS_IMETHOD Read(void *data, uint32_t size, uint32_t *bytesRead) override { return _to Read(data, size, bytesRead); } \
  NS_IMETHOD Write(void *data, uint32_t size, uint32_t *bytesWritten) override { return _to Write(data, size, bytesWritten); } \
  NS_IMETHOD Seek(int64_t offset, int32_t origin, uint64_t *newPosition) override { return _to Seek(offset, origin, newPosition); } \
  NS_IMETHOD GetLength(uint64_t *length) override { return _to GetLength(length); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_FXISTREAM(_to) \
  NS_IMETHOD Read(void *data, uint32_t size, uint32_t *bytesRead) override { return !_to ? NS_ERROR_NULL_POINTER : _to->Read(data, size, bytesRead); } \
  NS_IMETHOD Write(void *data, uint32_t size, uint32_t *bytesWritten) override { return !_to ? NS_ERROR_NULL_POINTER : _to->Write(data, size, bytesWritten); } \
  NS_IMETHOD Seek(int64_t offset, int32_t origin, uint64_t *newPosition) override { return !_to ? NS_ERROR_NULL_POINTER : _to->Seek(offset, origin, newPosition); } \
  NS_IMETHOD GetLength(uint64_t *length) override { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLength(length); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class fxStream : public fxIStream
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_FXISTREAM

  fxStream();

private:
  ~fxStream();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS(fxStream, fxIStream)

fxStream::fxStream()
{
  /* member initializers and constructor code */
}

fxStream::~fxStream()
{
  /* destructor code */
}

/* void Read (in voidPtr data, in uint32_t size, out uint32_t bytesRead); */
NS_IMETHODIMP fxStream::Read(void *data, uint32_t size, uint32_t *bytesRead)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void Write (in voidPtr data, in uint32_t size, out uint32_t bytesWritten); */
NS_IMETHODIMP fxStream::Write(void *data, uint32_t size, uint32_t *bytesWritten)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void Seek (in int64_t offset, in int32_t origin, out uint64_t newPosition); */
NS_IMETHODIMP fxStream::Seek(int64_t offset, int32_t origin, uint64_t *newPosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void GetLength (out uint64_t length); */
NS_IMETHODIMP fxStream::GetLength(uint64_t *length)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    IScriptHost */
#define ISCRIPTHOST_IID_STR "8ffdc384-4767-4ea2-a935-3bfcad1db7bf"

#define ISCRIPTHOST_IID \
  {0x8ffdc384, 0x4767, 0x4ea2, \
    { 0xa9, 0x35, 0x3b, 0xfc, 0xad, 0x1d, 0xb7, 0xbf }}

class NS_NO_VTABLE IScriptHost : public fxIBase {
 public:

  NS_DECLARE_STATIC_IID_ACCESSOR(ISCRIPTHOST_IID)

  /* void InvokeNative (inout NativeCtx context); */
  NS_IMETHOD InvokeNative(fxNativeContext & context) = 0;

  /* void OpenSystemFile (in charPtr fileName, out fxIStream stream); */
  NS_IMETHOD OpenSystemFile(char *fileName, fxIStream * *stream) = 0;

  /* void OpenHostFile (in charPtr fileName, out fxIStream stream); */
  NS_IMETHOD OpenHostFile(char *fileName, fxIStream * *stream) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(IScriptHost, ISCRIPTHOST_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_ISCRIPTHOST \
  NS_IMETHOD InvokeNative(fxNativeContext & context) override; \
  NS_IMETHOD OpenSystemFile(char *fileName, fxIStream * *stream) override; \
  NS_IMETHOD OpenHostFile(char *fileName, fxIStream * *stream) override; 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_ISCRIPTHOST(_to) \
  NS_IMETHOD InvokeNative(fxNativeContext & context) override { return _to InvokeNative(context); } \
  NS_IMETHOD OpenSystemFile(char *fileName, fxIStream * *stream) override { return _to OpenSystemFile(fileName, stream); } \
  NS_IMETHOD OpenHostFile(char *fileName, fxIStream * *stream) override { return _to OpenHostFile(fileName, stream); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_ISCRIPTHOST(_to) \
  NS_IMETHOD InvokeNative(fxNativeContext & context) override { return !_to ? NS_ERROR_NULL_POINTER : _to->InvokeNative(context); } \
  NS_IMETHOD OpenSystemFile(char *fileName, fxIStream * *stream) override { return !_to ? NS_ERROR_NULL_POINTER : _to->OpenSystemFile(fileName, stream); } \
  NS_IMETHOD OpenHostFile(char *fileName, fxIStream * *stream) override { return !_to ? NS_ERROR_NULL_POINTER : _to->OpenHostFile(fileName, stream); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public IScriptHost
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_ISCRIPTHOST

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS(_MYCLASS_, IScriptHost)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void InvokeNative (inout NativeCtx context); */
NS_IMETHODIMP _MYCLASS_::InvokeNative(fxNativeContext & context)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void OpenSystemFile (in charPtr fileName, out fxIStream stream); */
NS_IMETHODIMP _MYCLASS_::OpenSystemFile(char *fileName, fxIStream * *stream)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void OpenHostFile (in charPtr fileName, out fxIStream stream); */
NS_IMETHODIMP _MYCLASS_::OpenHostFile(char *fileName, fxIStream * *stream)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    IScriptRuntime */
#define ISCRIPTRUNTIME_IID_STR "67b28af1-aaf9-4368-8296-f93afc7bde96"

#define ISCRIPTRUNTIME_IID \
  {0x67b28af1, 0xaaf9, 0x4368, \
    { 0x82, 0x96, 0xf9, 0x3a, 0xfc, 0x7b, 0xde, 0x96 }}

class NS_NO_VTABLE IScriptRuntime : public fxIBase {
 public:

  NS_DECLARE_STATIC_IID_ACCESSOR(ISCRIPTRUNTIME_IID)

  /* void Create (in IScriptHost scriptHost); */
  NS_IMETHOD Create(IScriptHost *scriptHost) = 0;

  /* void Destroy (); */
  NS_IMETHOD Destroy(void) = 0;

  /* [notxpcom] int32_t GetInstanceId (); */
  NS_IMETHOD_(int32_t) GetInstanceId(void) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(IScriptRuntime, ISCRIPTRUNTIME_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_ISCRIPTRUNTIME \
  NS_IMETHOD Create(IScriptHost *scriptHost) override; \
  NS_IMETHOD Destroy(void) override; \
  NS_IMETHOD_(int32_t) GetInstanceId(void) override; 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_ISCRIPTRUNTIME(_to) \
  NS_IMETHOD Create(IScriptHost *scriptHost) override { return _to Create(scriptHost); } \
  NS_IMETHOD Destroy(void) override { return _to Destroy(); } \
  NS_IMETHOD_(int32_t) GetInstanceId(void) override { return _to GetInstanceId(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_ISCRIPTRUNTIME(_to) \
  NS_IMETHOD Create(IScriptHost *scriptHost) override { return !_to ? NS_ERROR_NULL_POINTER : _to->Create(scriptHost); } \
  NS_IMETHOD Destroy(void) override { return !_to ? NS_ERROR_NULL_POINTER : _to->Destroy(); } \
  NS_IMETHOD_(int32_t) GetInstanceId(void) override; 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public IScriptRuntime
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_ISCRIPTRUNTIME

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS(_MYCLASS_, IScriptRuntime)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void Create (in IScriptHost scriptHost); */
NS_IMETHODIMP _MYCLASS_::Create(IScriptHost *scriptHost)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void Destroy (); */
NS_IMETHODIMP _MYCLASS_::Destroy()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [notxpcom] int32_t GetInstanceId (); */
NS_IMETHODIMP_(int32_t) _MYCLASS_::GetInstanceId()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    IScriptRuntimeHandler */
#define ISCRIPTRUNTIMEHANDLER_IID_STR "4720a986-eaa6-4ecc-a31f-2ce2bbf569f7"

#define ISCRIPTRUNTIMEHANDLER_IID \
  {0x4720a986, 0xeaa6, 0x4ecc, \
    { 0xa3, 0x1f, 0x2c, 0xe2, 0xbb, 0xf5, 0x69, 0xf7 }}

class NS_NO_VTABLE IScriptRuntimeHandler : public fxIBase {
 public:

  NS_DECLARE_STATIC_IID_ACCESSOR(ISCRIPTRUNTIMEHANDLER_IID)

  /* void PushRuntime (in IScriptRuntime runtime); */
  NS_IMETHOD PushRuntime(IScriptRuntime *runtime) = 0;

  /* void GetCurrentRuntime (out IScriptRuntime runtime); */
  NS_IMETHOD GetCurrentRuntime(IScriptRuntime * *runtime) = 0;

  /* void PopRuntime (in IScriptRuntime runtime); */
  NS_IMETHOD PopRuntime(IScriptRuntime *runtime) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(IScriptRuntimeHandler, ISCRIPTRUNTIMEHANDLER_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_ISCRIPTRUNTIMEHANDLER \
  NS_IMETHOD PushRuntime(IScriptRuntime *runtime) override; \
  NS_IMETHOD GetCurrentRuntime(IScriptRuntime * *runtime) override; \
  NS_IMETHOD PopRuntime(IScriptRuntime *runtime) override; 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_ISCRIPTRUNTIMEHANDLER(_to) \
  NS_IMETHOD PushRuntime(IScriptRuntime *runtime) override { return _to PushRuntime(runtime); } \
  NS_IMETHOD GetCurrentRuntime(IScriptRuntime * *runtime) override { return _to GetCurrentRuntime(runtime); } \
  NS_IMETHOD PopRuntime(IScriptRuntime *runtime) override { return _to PopRuntime(runtime); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_ISCRIPTRUNTIMEHANDLER(_to) \
  NS_IMETHOD PushRuntime(IScriptRuntime *runtime) override { return !_to ? NS_ERROR_NULL_POINTER : _to->PushRuntime(runtime); } \
  NS_IMETHOD GetCurrentRuntime(IScriptRuntime * *runtime) override { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCurrentRuntime(runtime); } \
  NS_IMETHOD PopRuntime(IScriptRuntime *runtime) override { return !_to ? NS_ERROR_NULL_POINTER : _to->PopRuntime(runtime); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public IScriptRuntimeHandler
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_ISCRIPTRUNTIMEHANDLER

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS(_MYCLASS_, IScriptRuntimeHandler)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void PushRuntime (in IScriptRuntime runtime); */
NS_IMETHODIMP _MYCLASS_::PushRuntime(IScriptRuntime *runtime)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void GetCurrentRuntime (out IScriptRuntime runtime); */
NS_IMETHODIMP _MYCLASS_::GetCurrentRuntime(IScriptRuntime * *runtime)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void PopRuntime (in IScriptRuntime runtime); */
NS_IMETHODIMP _MYCLASS_::PopRuntime(IScriptRuntime *runtime)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    IScriptTickRuntime */
#define ISCRIPTTICKRUNTIME_IID_STR "91b203c7-f95a-4902-b463-722d55098366"

#define ISCRIPTTICKRUNTIME_IID \
  {0x91b203c7, 0xf95a, 0x4902, \
    { 0xb4, 0x63, 0x72, 0x2d, 0x55, 0x09, 0x83, 0x66 }}

class NS_NO_VTABLE IScriptTickRuntime : public fxIBase {
 public:

  NS_DECLARE_STATIC_IID_ACCESSOR(ISCRIPTTICKRUNTIME_IID)

  /* void Tick (); */
  NS_IMETHOD Tick(void) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(IScriptTickRuntime, ISCRIPTTICKRUNTIME_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_ISCRIPTTICKRUNTIME \
  NS_IMETHOD Tick(void) override; 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_ISCRIPTTICKRUNTIME(_to) \
  NS_IMETHOD Tick(void) override { return _to Tick(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_ISCRIPTTICKRUNTIME(_to) \
  NS_IMETHOD Tick(void) override { return !_to ? NS_ERROR_NULL_POINTER : _to->Tick(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public IScriptTickRuntime
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_ISCRIPTTICKRUNTIME

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS(_MYCLASS_, IScriptTickRuntime)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void Tick (); */
NS_IMETHODIMP _MYCLASS_::Tick()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    IScriptEventRuntime */
#define ISCRIPTEVENTRUNTIME_IID_STR "637140db-24e5-46bf-a8bd-08f2dbac519a"

#define ISCRIPTEVENTRUNTIME_IID \
  {0x637140db, 0x24e5, 0x46bf, \
    { 0xa8, 0xbd, 0x08, 0xf2, 0xdb, 0xac, 0x51, 0x9a }}

class NS_NO_VTABLE IScriptEventRuntime : public fxIBase {
 public:

  NS_DECLARE_STATIC_IID_ACCESSOR(ISCRIPTEVENTRUNTIME_IID)

  /* void TriggerEvent (in charPtr eventName, in charPtr argsSerialized, in int32_t sourceId); */
  NS_IMETHOD TriggerEvent(char *eventName, char *argsSerialized, int32_t sourceId) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(IScriptEventRuntime, ISCRIPTEVENTRUNTIME_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_ISCRIPTEVENTRUNTIME \
  NS_IMETHOD TriggerEvent(char *eventName, char *argsSerialized, int32_t sourceId) override; 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_ISCRIPTEVENTRUNTIME(_to) \
  NS_IMETHOD TriggerEvent(char *eventName, char *argsSerialized, int32_t sourceId) override { return _to TriggerEvent(eventName, argsSerialized, sourceId); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_ISCRIPTEVENTRUNTIME(_to) \
  NS_IMETHOD TriggerEvent(char *eventName, char *argsSerialized, int32_t sourceId) override { return !_to ? NS_ERROR_NULL_POINTER : _to->TriggerEvent(eventName, argsSerialized, sourceId); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public IScriptEventRuntime
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_ISCRIPTEVENTRUNTIME

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS(_MYCLASS_, IScriptEventRuntime)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void TriggerEvent (in charPtr eventName, in charPtr argsSerialized, in int32_t sourceId); */
NS_IMETHODIMP _MYCLASS_::TriggerEvent(char *eventName, char *argsSerialized, int32_t sourceId)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    IScriptRefRuntime */
#define ISCRIPTREFRUNTIME_IID_STR "a2f1b24b-a29f-4121-8162-86901eca8097"

#define ISCRIPTREFRUNTIME_IID \
  {0xa2f1b24b, 0xa29f, 0x4121, \
    { 0x81, 0x62, 0x86, 0x90, 0x1e, 0xca, 0x80, 0x97 }}

class NS_NO_VTABLE IScriptRefRuntime : public fxIBase {
 public:

  NS_DECLARE_STATIC_IID_ACCESSOR(ISCRIPTREFRUNTIME_IID)

  /* void CallRef (in int32_t refIdx, in charPtr argsSerialized, out charPtr retvalSerialized); */
  NS_IMETHOD CallRef(int32_t refIdx, char *argsSerialized, char **retvalSerialized) = 0;

  /* void DuplicateRef (in int32_t refIdx, out int32_t newRefIdx); */
  NS_IMETHOD DuplicateRef(int32_t refIdx, int32_t *newRefIdx) = 0;

  /* void RemoveRef (in int32_t refIdx); */
  NS_IMETHOD RemoveRef(int32_t refIdx) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(IScriptRefRuntime, ISCRIPTREFRUNTIME_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_ISCRIPTREFRUNTIME \
  NS_IMETHOD CallRef(int32_t refIdx, char *argsSerialized, char **retvalSerialized) override; \
  NS_IMETHOD DuplicateRef(int32_t refIdx, int32_t *newRefIdx) override; \
  NS_IMETHOD RemoveRef(int32_t refIdx) override; 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_ISCRIPTREFRUNTIME(_to) \
  NS_IMETHOD CallRef(int32_t refIdx, char *argsSerialized, char **retvalSerialized) override { return _to CallRef(refIdx, argsSerialized, retvalSerialized); } \
  NS_IMETHOD DuplicateRef(int32_t refIdx, int32_t *newRefIdx) override { return _to DuplicateRef(refIdx, newRefIdx); } \
  NS_IMETHOD RemoveRef(int32_t refIdx) override { return _to RemoveRef(refIdx); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_ISCRIPTREFRUNTIME(_to) \
  NS_IMETHOD CallRef(int32_t refIdx, char *argsSerialized, char **retvalSerialized) override { return !_to ? NS_ERROR_NULL_POINTER : _to->CallRef(refIdx, argsSerialized, retvalSerialized); } \
  NS_IMETHOD DuplicateRef(int32_t refIdx, int32_t *newRefIdx) override { return !_to ? NS_ERROR_NULL_POINTER : _to->DuplicateRef(refIdx, newRefIdx); } \
  NS_IMETHOD RemoveRef(int32_t refIdx) override { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveRef(refIdx); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public IScriptRefRuntime
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_ISCRIPTREFRUNTIME

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS(_MYCLASS_, IScriptRefRuntime)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void CallRef (in int32_t refIdx, in charPtr argsSerialized, out charPtr retvalSerialized); */
NS_IMETHODIMP _MYCLASS_::CallRef(int32_t refIdx, char *argsSerialized, char **retvalSerialized)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void DuplicateRef (in int32_t refIdx, out int32_t newRefIdx); */
NS_IMETHODIMP _MYCLASS_::DuplicateRef(int32_t refIdx, int32_t *newRefIdx)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void RemoveRef (in int32_t refIdx); */
NS_IMETHODIMP _MYCLASS_::RemoveRef(int32_t refIdx)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    IScriptFileHandlingRuntime */
#define ISCRIPTFILEHANDLINGRUNTIME_IID_STR "567634c6-3bdd-4d0e-af39-7472aed479b7"

#define ISCRIPTFILEHANDLINGRUNTIME_IID \
  {0x567634c6, 0x3bdd, 0x4d0e, \
    { 0xaf, 0x39, 0x74, 0x72, 0xae, 0xd4, 0x79, 0xb7 }}

class NS_NO_VTABLE IScriptFileHandlingRuntime : public fxIBase {
 public:

  NS_DECLARE_STATIC_IID_ACCESSOR(ISCRIPTFILEHANDLINGRUNTIME_IID)

  /* [notxpcom] int32_t HandlesFile (in charPtr scriptFile); */
  NS_IMETHOD_(int32_t) HandlesFile(char *scriptFile) = 0;

  /* void LoadFile (in charPtr scriptFile); */
  NS_IMETHOD LoadFile(char *scriptFile) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(IScriptFileHandlingRuntime, ISCRIPTFILEHANDLINGRUNTIME_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_ISCRIPTFILEHANDLINGRUNTIME \
  NS_IMETHOD_(int32_t) HandlesFile(char *scriptFile) override; \
  NS_IMETHOD LoadFile(char *scriptFile) override; 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_ISCRIPTFILEHANDLINGRUNTIME(_to) \
  NS_IMETHOD_(int32_t) HandlesFile(char *scriptFile) override { return _to HandlesFile(scriptFile); } \
  NS_IMETHOD LoadFile(char *scriptFile) override { return _to LoadFile(scriptFile); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_ISCRIPTFILEHANDLINGRUNTIME(_to) \
  NS_IMETHOD_(int32_t) HandlesFile(char *scriptFile) override; \
  NS_IMETHOD LoadFile(char *scriptFile) override { return !_to ? NS_ERROR_NULL_POINTER : _to->LoadFile(scriptFile); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public IScriptFileHandlingRuntime
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_ISCRIPTFILEHANDLINGRUNTIME

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS(_MYCLASS_, IScriptFileHandlingRuntime)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* [notxpcom] int32_t HandlesFile (in charPtr scriptFile); */
NS_IMETHODIMP_(int32_t) _MYCLASS_::HandlesFile(char *scriptFile)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void LoadFile (in charPtr scriptFile); */
NS_IMETHODIMP _MYCLASS_::LoadFile(char *scriptFile)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#include "PushEnvironment.h"

#endif /* __gen_fxScripting_h__ */
