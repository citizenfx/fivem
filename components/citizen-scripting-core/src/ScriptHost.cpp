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

#include <stack>
#include <mutex>

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
	*length = m_stream->GetLength();

	return FX_S_OK;
}

class TestScriptHost : public OMClass<TestScriptHost, IScriptHost>
{
public:
	NS_DECL_ISCRIPTHOST;

private:
	Resource* m_resource;

private:
	result_t WrapVFSStreamResult(fwRefContainer<vfs::Stream> stream, fxIStream** result);

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

result_t TestScriptHost::WrapVFSStreamResult(fwRefContainer<vfs::Stream> stream, fxIStream** result)
{
	if (stream.GetRef())
	{
		auto streamWrapper = fx::MakeNew<StreamWrapper>(stream);
		streamWrapper->AddRef();

		*result = streamWrapper.GetRef();

		return FX_S_OK;
	}

	// TODO: replace with other error?
	return FX_E_INVALIDARG;
}

result_t TestScriptHost::OpenSystemFile(char *fileName, fxIStream * *stream)
{
	fwRefContainer<vfs::Stream> nativeStream = vfs::OpenRead(fileName);

	return WrapVFSStreamResult(nativeStream, stream);
}

result_t TestScriptHost::OpenHostFile(char *fileName, fxIStream * *stream)
{
	fwRefContainer<vfs::Stream> nativeStream = vfs::OpenRead(m_resource->GetPath() + "/" + fileName);
	
	return WrapVFSStreamResult(nativeStream, stream);
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

// more stuff
class ScriptRuntimeHandler : public OMClass<ScriptRuntimeHandler, IScriptRuntimeHandler>
{
private:
	static std::stack<IScriptRuntime*> ms_runtimeStack;

	static std::recursive_mutex ms_runtimeMutex;

public:
	NS_DECL_ISCRIPTRUNTIMEHANDLER;
};

std::stack<IScriptRuntime*> ScriptRuntimeHandler::ms_runtimeStack;

std::recursive_mutex ScriptRuntimeHandler::ms_runtimeMutex;

result_t ScriptRuntimeHandler::PushRuntime(IScriptRuntime* runtime)
{
	ms_runtimeMutex.lock();

	ms_runtimeStack.push(runtime);

	return FX_S_OK;
}

result_t ScriptRuntimeHandler::PopRuntime(IScriptRuntime* runtime)
{
	IScriptRuntime* poppedRuntime = ms_runtimeStack.top();
	assert(poppedRuntime == runtime);

	ms_runtimeStack.pop();
	ms_runtimeMutex.unlock();

	return FX_S_OK;
}

result_t ScriptRuntimeHandler::GetCurrentRuntime(IScriptRuntime** runtime)
{
	if (ms_runtimeStack.empty())
	{
		*runtime = nullptr;
		return FX_E_INVALIDARG;
	}

	*runtime = ms_runtimeStack.top();

	// conventions state we should AddRef anything we return, so we will
	(*runtime)->AddRef();

	return FX_S_OK;
}

// {C41E7194-7556-4C02-BA45-A9C84D18AD43}

FX_IMPLEMENTS(CLSID_ScriptRuntimeHandler, IScriptRuntimeHandler);
FX_NEW_FACTORY(ScriptRuntimeHandler);

}