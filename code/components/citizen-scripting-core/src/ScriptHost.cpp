/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include "fxScripting.h"
#include "om/OMComponent.h"

#include <ScriptEngine.h>

#include <Resource.h>
#include <ResourceManager.h>
#include <VFSManager.h>

#include <ResourceMetaDataComponent.h>
#include <ResourceScriptingComponent.h>

#include <stack>
#include <mutex>

#include <string_view>

#include <ConsoleHost.h>
#include <msgpack.hpp>

#include <sstream>

#ifdef __has_feature
#define HAS_FEATURE(x) __has_feature(x)
#else
#define HAS_FEATURE(x) 0
#endif

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

class TestScriptHost : public OMClass<TestScriptHost, IScriptHost, IScriptHostWithResourceData, IScriptHostWithManifest>
{
public:
	NS_DECL_ISCRIPTHOST;

	NS_DECL_ISCRIPTHOSTWITHRESOURCEDATA;

	NS_DECL_ISCRIPTHOSTWITHMANIFEST;

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
#if SCRT_HAS_CALLNATIVEHANDLER
	// prepare an invocation context
	fx::ScriptContextRaw scriptContext(context.arguments, context.numArguments);

	// call the native handler
	try
	{
		ScriptEngine::CallNativeHandler(context.nativeIdentifier, scriptContext);
	}
	catch (std::exception& e)
	{
		trace("%s: execution failed: %s\n", __func__, e.what());

		return FX_E_INVALIDARG;
	}
#else
	// get a native handler for the identifier
	auto nativeHandler = ScriptEngine::GetNativeHandler(context.nativeIdentifier);

	if (nativeHandler)
	{
		// prepare an invocation context
		fx::ScriptContextRaw scriptContext(context.arguments, context.numArguments);
		
		// invoke the native handler
		try
		{
			(*nativeHandler)(scriptContext);
		}
		catch (std::exception& e)
		{
// https://github.com/google/sanitizers/issues/749
#if !HAS_FEATURE(address_sanitizer)
			trace("%s: execution failed: %s\n", __func__, e.what());
#endif

			return FX_E_INVALIDARG;
		}

		// set the result value
		context.numResults = 1;
	}
	else
	{
		trace("WARNING: NON-EXISTENT NATIVE %016llx\n", context.nativeIdentifier);
	}
#endif

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

	// return not-found
	return 0x80070002;
}

result_t TestScriptHost::ScriptTrace(char* string)
{
	return FX_S_OK;
}

result_t TestScriptHost::OpenSystemFile(char *fileName, fxIStream * *stream)
{
	m_resource->GetComponent<fx::ResourceScriptingComponent>()->OnOpenScript(fileName, fileName);

	fwRefContainer<vfs::Stream> nativeStream = vfs::OpenRead(fileName);

	return WrapVFSStreamResult(nativeStream, stream);
}

result_t TestScriptHost::OpenHostFile(char *fileName, fxIStream * *stream)
{
	std::string_view fn = fileName;
	std::string fileNameStr = m_resource->GetPath() + "/" + fileName;

	if (fn.length() > 1 && fn[0] == '@')
	{
		std::string_view resName = fn.substr(1, fn.find_first_of('/') - 1);
		fn = fn.substr(1 + resName.length() + 1);

		auto resourceManager = fx::ResourceManager::GetCurrent();
		auto resource = resourceManager->GetResource(std::string(resName));

		if (!resource.GetRef())
		{
			return 0x80070002;
		}

		fileNameStr = resource->GetPath() + "/" + std::string(fn);
	}

	m_resource->GetComponent<fx::ResourceScriptingComponent>()->OnOpenScript(fileNameStr, "@" + m_resource->GetName() + "/" + fileName);

	fwRefContainer<vfs::Stream> nativeStream = vfs::OpenRead(fileNameStr);
	
	return WrapVFSStreamResult(nativeStream, stream);
}

result_t TestScriptHost::CanonicalizeRef(int32_t refIdx, int32_t instanceId, char** outRefText)
{
	// format a string first
	const char* refString = va("%s:%d:%d", m_resource->GetName().c_str(), instanceId, refIdx);

	*outRefText = reinterpret_cast<char*>(fwAlloc(strlen(refString) + 1));
	strcpy(*outRefText, refString);

	return FX_S_OK;
}

result_t TestScriptHost::GetResourceName(char** outResourceName)
{
	*outResourceName = const_cast<char*>(m_resource->GetName().c_str());
	return FX_S_OK;
}

result_t TestScriptHost::GetNumResourceMetaData(char* metaDataName, int32_t* entryCount)
{
	fwRefContainer<ResourceMetaDataComponent> metaData = m_resource->GetComponent<ResourceMetaDataComponent>();

	auto entries = metaData->GetEntries(metaDataName);

	*entryCount = static_cast<int32_t>(std::distance(entries.begin(), entries.end()));

	return FX_S_OK;
}

result_t TestScriptHost::GetResourceMetaData(char* metaDataName, int32_t entryIndex, char** outMetaData)
{
	fwRefContainer<ResourceMetaDataComponent> metaData = m_resource->GetComponent<ResourceMetaDataComponent>();
	
	auto entries = metaData->GetEntries(metaDataName);

	// and loop over the entries to see if we find anything
	int i = 0;

	for (auto& entry : entries)
	{
		if (entryIndex == i)
		{
			*outMetaData = const_cast<char*>(entry.second.c_str());
			return FX_S_OK;
		}

		i++;
	}

	// return not-found
	return 0x80070490;
}

result_t TestScriptHost::IsManifestVersionBetween(const guid_t & lowerBound, const guid_t & upperBound, bool *_retval)
{
	// get the manifest version
	auto metaData = m_resource->GetComponent<ResourceMetaDataComponent>();

	auto retval = metaData->IsManifestVersionBetween(lowerBound, upperBound);

	if (retval)
	{
		*_retval = *retval;

		return FX_S_OK;
	}

	return FX_E_INVALIDARG;
}

result_t TestScriptHost::IsManifestVersionV2Between(char* lowerBound, char* upperBound, bool* _retval)
{
	// get the manifest version
	auto metaData = m_resource->GetComponent<ResourceMetaDataComponent>();

	auto retval = metaData->IsManifestVersionBetween(lowerBound, upperBound);

	if (retval)
	{
		*_retval = *retval;

		return FX_S_OK;
	}

	return FX_E_INVALIDARG;
}

using Boundary = std::vector<uint8_t>;
using BoundaryPair = std::pair<std::optional<Boundary>, std::optional<Boundary>>;

static std::deque<IScriptRuntime*> ms_runtimeStack;
static std::deque<BoundaryPair> ms_boundaryStack;

static std::recursive_mutex ms_runtimeMutex;

static bool g_suppressErrors;

result_t TestScriptHost::SubmitBoundaryStart(char* boundaryData, int boundarySize)
{
	if (!ms_boundaryStack.empty())
	{
		ms_boundaryStack.front().first = Boundary{ (uint8_t*)boundaryData, (uint8_t*)boundaryData + boundarySize };
	}

	return FX_S_OK;
}

result_t TestScriptHost::SubmitBoundaryEnd(char* boundaryData, int boundarySize)
{
	if (!ms_boundaryStack.empty())
	{
		if (boundaryData)
		{
			ms_boundaryStack.front().second = Boundary{ (uint8_t*)boundaryData, (uint8_t*)boundaryData + boundarySize };
		}
		else
		{
			ms_boundaryStack.front().second = {};
		}
	}

	return FX_S_OK;
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
public:
	NS_DECL_ISCRIPTRUNTIMEHANDLER;
};

result_t ScriptRuntimeHandler::PushRuntime(IScriptRuntime* runtime)
{
	ms_runtimeMutex.lock();

	ms_runtimeStack.push_front(runtime);
	ms_boundaryStack.push_front({});

	fx::Resource* parentResource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

	if (parentResource)
	{
		parentResource->OnActivate();
	}
	
	return FX_S_OK;
}

result_t ScriptRuntimeHandler::PopRuntime(IScriptRuntime* runtime)
{
	IScriptRuntime* poppedRuntime = ms_runtimeStack.front();
	assert(poppedRuntime == runtime);

	fx::Resource* parentResource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

	if (parentResource)
	{
		parentResource->OnDeactivate();
	}

	ms_boundaryStack.pop_front();
	ms_runtimeStack.pop_front();

	if (ms_runtimeStack.empty())
	{
		g_suppressErrors = false;
	}

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

	*runtime = ms_runtimeStack.front();

	// conventions state we should AddRef anything we return, so we will
	(*runtime)->AddRef();

	return FX_S_OK;
}

result_t ScriptRuntimeHandler::GetInvokingRuntime(IScriptRuntime** runtime)
{
	if (ms_runtimeStack.empty())
	{
		*runtime = nullptr;
		return FX_E_INVALIDARG;
	}

	// std::stack is bad, even more so as we're copying the entire stack
	// TODO: redo since we're a deque now
	auto copyStack = ms_runtimeStack;
	copyStack.pop_front();

	if (copyStack.empty())
	{
		*runtime = nullptr;
	}
	else
	{
		auto lastRuntime = copyStack.front();
		*runtime = lastRuntime;

		// conventions state we should AddRef anything we return, so we will
		(*runtime)->AddRef();
	}

	return FX_S_OK;
}

// {C41E7194-7556-4C02-BA45-A9C84D18AD43}

FX_IMPLEMENTS(CLSID_ScriptRuntimeHandler, IScriptRuntimeHandler);
FX_NEW_FACTORY(ScriptRuntimeHandler);

}

struct StringifyingStackVisitor : fx::OMClass<StringifyingStackVisitor, IScriptStackWalkVisitor>
{
	std::stringstream stringTrace;

	NS_DECL_ISCRIPTSTACKWALKVISITOR;
};

struct ScriptStackFrame
{
	std::string name;
	std::string file;
	std::string sourcefile;
	int line;

	MSGPACK_DEFINE_MAP(name, file, sourcefile, line);
};

result_t StringifyingStackVisitor::SubmitStackFrame(char* blob, uint32_t size)
{
	msgpack::unpacked up = msgpack::unpack(blob, size);
	auto frame = up.get().as<ScriptStackFrame>();

	stringTrace << fmt::sprintf("^3> %s^7 (^5%s^7%s:%d)\n", frame.name, frame.file, (!frame.sourcefile.empty()) ? " <- " + frame.sourcefile : "", frame.line);

	return FX_S_OK;
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("FORMAT_STACK_TRACE", [](fx::ScriptContext& context)
	{
		if (fx::g_suppressErrors)
		{
			context.SetResult(nullptr);
			return;
		}

		fx::g_suppressErrors = true;

		auto topLevelStackBlob = context.GetArgument<char*>(0);
		auto topLevelStackSize = context.GetArgument<uint32_t>(1);

		auto vis = fx::MakeNew<StringifyingStackVisitor>();

		if (topLevelStackBlob)
		{
			msgpack::unpacked up = msgpack::unpack(topLevelStackBlob, topLevelStackSize);

			auto o = up.get().as<std::vector<msgpack::object>>();

			for (auto& e : o)
			{
				msgpack::sbuffer sb;
				msgpack::pack(sb, e);

				vis->SubmitStackFrame(sb.data(), sb.size());
			}
		}

		for (int i = 0; i < fx::ms_runtimeStack.size(); i++)
		{
			auto rit = fx::ms_runtimeStack.begin() + i;
			auto bit = fx::ms_boundaryStack.begin() + i;

			fx::OMPtr<IScriptRuntime> rt(*rit);
			fx::OMPtr<IScriptStackWalkingRuntime> srt;

			if (FX_SUCCEEDED(rt.As(&srt)))
			{
				auto& b = bit->first;
				auto& e = bit->second;

				srt->WalkStack(e ? (char*)e->data() : NULL, e ? e->size() : 0, b ? (char*)b->data() : NULL, b ? b->size() : NULL, vis.GetRef());
			}
		}

		static std::string stackTraceBuffer;
		stackTraceBuffer = vis->stringTrace.str();

		context.SetResult(stackTraceBuffer.c_str());
	});
});
