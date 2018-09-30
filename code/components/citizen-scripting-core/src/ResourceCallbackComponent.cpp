#include "StdInc.h"
#include <ResourceCallbackComponent.h>
#include <ResourceScriptingComponent.h>

namespace fx
{
ResourceCallbackScriptRuntime::ResourceCallbackScriptRuntime(fx::Resource* resource, IScriptHost* scriptHost)
	: m_resource(resource), m_scriptHost(scriptHost), m_refIdx(1)
{

}

result_t ResourceCallbackScriptRuntime::Create(IScriptHost* host)
{
	return FX_S_OK;
}

result_t ResourceCallbackScriptRuntime::Destroy()
{
	return FX_S_OK;
}

void* ResourceCallbackScriptRuntime::GetParentObject()
{
	return m_resource;
}

void ResourceCallbackScriptRuntime::SetParentObject(void*)
{

}

int ResourceCallbackScriptRuntime::GetInstanceId()
{
	return 'CBRT';
}

result_t ResourceCallbackScriptRuntime::CallRef(int32_t refIdx, char* argsSerialized, uint32_t argsLength, char** retvalSerialized, uint32_t* retvalLength)
{
	// preset retval to be null
	{
		static msgpack::sbuffer sb;
		sb.clear();

		msgpack::packer<msgpack::sbuffer> packer(sb);
		packer.pack_array(0);

		*retvalSerialized = sb.data();
		*retvalLength = sb.size();
	}

	// assume we have the ref still
	std::function<void(const msgpack::unpacked&)> cb;

	{
		std::unique_lock<std::recursive_mutex> lock(m_refMutex);

		auto it = m_refs.find(refIdx);

		if (it == m_refs.end())
		{
			return FX_E_INVALIDARG;
		}

		cb = it->second->callback;
	}

	// unpack
	msgpack::unpacked unpacked;
	msgpack::unpack(unpacked, argsSerialized, argsLength);

	// convert
	cb(unpacked);

	return FX_S_OK;
}

result_t ResourceCallbackScriptRuntime::DuplicateRef(int32_t refIdx, int32_t* outRef)
{
	std::unique_lock<std::recursive_mutex> lock(m_refMutex);

	auto it = m_refs.find(refIdx);

	if (it == m_refs.end())
	{
		return FX_E_INVALIDARG;
	}

	auto& refData = it->second;
	++refData->refCount;

	*outRef = refIdx;

	return FX_S_OK;
}

result_t ResourceCallbackScriptRuntime::RemoveRef(int32_t refIdx)
{
	std::unique_lock<std::recursive_mutex> lock(m_refMutex);

	auto it = m_refs.find(refIdx);

	if (it == m_refs.end())
	{
		return FX_E_INVALIDARG;
	}

	auto& refData = it->second;
	if (--refData->refCount <= 0)
	{
		m_refs.erase(refIdx);
	}

	return FX_S_OK;
}

std::string ResourceCallbackScriptRuntime::AddCallbackRef(const std::function<void(const msgpack::unpacked&)>& resultCallback)
{
	std::unique_lock<std::recursive_mutex> lock(m_refMutex);

	// add the ref to the list
	int32_t idx = m_refIdx;
	m_refs.emplace(idx, std::make_unique<RefData>(resultCallback));

	m_refIdx++;

	// canonicalize the ref
	char* refString;
	m_scriptHost->CanonicalizeRef(idx, GetInstanceId(), &refString);

	// turn into a std::string and free
	std::string retval = refString;
	fwFree(refString);

	// return the value
	return retval;
}

ResourceCallbackComponent::ResourceCallbackComponent(fx::ResourceManager* manager)
	: m_manager(manager)
{
	m_resource = manager->CreateResource("_cfx_internal");
	assert(m_resource->Start());

	fwRefContainer<fx::ResourceScriptingComponent> scriptingComponent = m_resource->GetComponent<fx::ResourceScriptingComponent>();
	fx::OMPtr<ResourceCallbackScriptRuntime> runtime = fx::MakeNew<ResourceCallbackScriptRuntime>(m_resource.GetRef(), scriptingComponent->GetScriptHost().GetRef());

	// convert to IScriptRuntime
	fx::OMPtr<IScriptRuntime> baseRuntime;
	runtime.As(&baseRuntime);

	scriptingComponent->AddRuntime(baseRuntime);

	// and set the local runtime
	m_scriptRuntime = runtime.GetRef();
}

auto ResourceCallbackComponent::CreateCallback(const std::function<void(const msgpack::unpacked &)>& cb) -> CallbackRef
{
	return CallbackRef{ this->GetScriptRuntime()->AddCallbackRef(cb) };
}
}

static InitFunction initFunction([]()
{
	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		manager->SetComponent(new fx::ResourceCallbackComponent(manager));
	});
});
