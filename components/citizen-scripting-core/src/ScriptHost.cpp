/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include "fxScripting.h"
#include "om/OMComponent.h"

#include <Resource.h>
#include <VFSManager.h>

namespace fx
{
class StreamWrapper : public OMClass<StreamWrapper, fxIStream>
{
private:
	fwRefContainer<vfs::Stream> m_stream;

public:
	StreamWrapper(const fwRefContainer<vfs::Stream>& stream);

public:
	NS_DECL_FXISTREAM;
};

StreamWrapper::StreamWrapper(const fwRefContainer<vfs::Stream>& ref)
	: m_stream(ref)
{
	
}

result_t StreamWrapper::Read(void *data, uint32_t size, uint32_t *bytesRead)
{
	size_t read = m_stream->Read(data, size);

	if (bytesRead)
	{
		*bytesRead = read;
	}

	return FX_S_OK;
}

result_t StreamWrapper::Write(void *data, uint32_t size, uint32_t *bytesWritten)
{
	return FX_E_NOTIMPL;
}

result_t StreamWrapper::Seek(int64_t offset, int32_t origin, uint64_t *newPosition)
{
	size_t newPos = m_stream->Seek(offset, origin);

	if (newPosition)
	{
		*newPosition = newPos;
	}

	return FX_S_OK;
}

result_t StreamWrapper::GetLength(uint64_t *length)
{
	return FX_E_NOTIMPL;
}

class TestScriptHost : public OMClass<TestScriptHost, IScriptHost>
{
public:
	NS_DECL_ISCRIPTHOST;

private:
	Resource* m_resource;

public:
	TestScriptHost(Resource* resource)
		: m_resource(resource)
	{

	}
};

result_t TestScriptHost::InvokeNative(fxNativeContext & context)
{
	return FX_S_OK;
}

result_t TestScriptHost::OpenHostFile(char *fileName, fxIStream * *stream)
{
	fwRefContainer<vfs::Stream> nativeStream = vfs::OpenRead(m_resource->GetPath() + "/" + fileName);

	if (nativeStream.GetRef())
	{
		fxIStream* streamWrapper = new StreamWrapper(nativeStream);
		streamWrapper->AddRef();

		*stream = streamWrapper;

		return FX_S_OK;
	}

	// TODO: replace with other error?
	return FX_E_INVALIDARG;
}

// TODO: don't ship with this in
OMPtr<IScriptHost> GetScriptHostForResource(Resource* resource)
{
	OMPtr<IScriptHost> retval;
	auto ptr = MakeNew<TestScriptHost>(resource);

	ptr.As(&retval);

	return retval;
}

// {441CA62C-7A70-4349-8A97-2BCBF7EAA61F}
FX_DEFINE_GUID(CLSID_TestScriptHost,
			   0x441ca62c, 0x7a70, 0x4349, 0x8a, 0x97, 0x2b, 0xcb, 0xf7, 0xea, 0xa6, 0x1f);

FX_IMPLEMENTS(CLSID_TestScriptHost, IScriptHost);
}