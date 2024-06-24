#include "StdInc.h"
#include <ResourceCallbackComponent.h>
#include <ResourceScriptingComponent.h>
#include <fxScriptBuffer.h>

class FakeResource : public fx::Resource
{
public:
	virtual const std::string& GetName() override
	{
		return m_name;
	}

	virtual const std::string& GetIdentifier() override
	{
		return m_empty;
	}

	virtual const std::string& GetPath() override
	{
		return m_path;
	}

	virtual fx::ResourceState GetState() override
	{
		return fx::ResourceState::Started;
	}

	virtual bool LoadFrom(const std::string& rootPath, std::string* errorResult) override
	{
		return false;
	}

	virtual bool Start() override
	{
		return false;
	}

	virtual bool Stop() override
	{
		return false;
	}

	virtual void Run(std::function<void()>&& func) override
	{
	}

	virtual fx::ResourceManager* GetManager() override
	{
		return nullptr;
	}

private:
	std::string m_name{ "_cfx_internal" };
	std::string m_empty;
	std::string m_path{ "memory:" };
};

namespace fx
{
ResourceCallbackScriptRuntime::ResourceCallbackScriptRuntime()
	: m_refIdx(1)
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
	static FakeResource resource;
	return &resource;
}

void ResourceCallbackScriptRuntime::SetParentObject(void*)
{

}

int ResourceCallbackScriptRuntime::GetInstanceId()
{
	return 'CBRT';
}

result_t ResourceCallbackScriptRuntime::CallRef(int32_t refIdx, char* argsSerialized, uint32_t argsLength, IScriptBuffer** retval)
{
	// preset retval to be null
	{
		static auto rv = ([]()
		{
			msgpack::sbuffer sb;

			msgpack::packer<msgpack::sbuffer> packer(sb);
			packer.pack_array(0);
			return fx::MemoryScriptBuffer::Make(sb.data(), static_cast<uint32_t>(sb.size()));
		})();
		
		rv.CopyTo(retval);
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
	// keep the ref destructor outside of the mutex to prevent deadlock
	std::unique_ptr<RefData> deleteRef;

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
			deleteRef = std::move(refData);
			m_refs.erase(refIdx);
		}
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

	// matches TestScriptHost::CanonicalizeRef
	return fmt::sprintf("%s:%d:%d", "_cfx_internal", 0, idx);
}

ResourceCallbackComponent::ResourceCallbackComponent(fx::ResourceManager* manager)
	: m_manager(manager)
{
	fx::OMPtr<ResourceCallbackScriptRuntime> runtime = fx::MakeNew<ResourceCallbackScriptRuntime>();

	// convert to IScriptRuntime
	fx::OMPtr<IScriptRuntime> baseRuntime;
	runtime.As(&baseRuntime);

	// and set the local runtime
	m_scriptRuntime = runtime;
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
