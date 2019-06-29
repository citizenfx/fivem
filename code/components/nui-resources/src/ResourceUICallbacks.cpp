/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <ResourceUI.h>

#include <fxScripting.h>
#include <ScriptEngine.h>

#include <ResourceEventComponent.h>
#include <ResourceScriptingComponent.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <MsgpackJson.h>

// TODO: replace this file with something using ResourceCallbackComponent

class ResourceUIScriptRuntime;

class ResourceUICallbackComponent : public fwRefCountable
{
private:
	fx::Resource* m_resource;

	ResourceUIScriptRuntime* m_scriptRuntime;

public:
	ResourceUICallbackComponent(fx::Resource* resource);

	inline ResourceUIScriptRuntime* GetScriptRuntime()
	{
		return m_scriptRuntime;
	}
};

// runtime we abuse for generating refs
class ResourceUIScriptRuntime : public fx::OMClass<ResourceUIScriptRuntime, IScriptRuntime, IScriptRefRuntime>
{
private:
	struct RefData
	{
		std::atomic<int32_t> refCount;
		ResUIResultCallback callback;

		RefData(ResUIResultCallback cb)
			: callback(cb), refCount(0)
		{

		}
	};

	fx::Resource* m_resource;

	IScriptHost* m_scriptHost;

	std::map<int32_t, std::unique_ptr<RefData>> m_refs;

	std::recursive_mutex m_refMutex;

	int32_t m_refIdx;

public:
	ResourceUIScriptRuntime(fx::Resource* resource, IScriptHost* scriptHost);

	NS_DECL_ISCRIPTRUNTIME;

	NS_DECL_ISCRIPTREFRUNTIME;

	std::string AddCallbackRef(ResUIResultCallback resultCallback);
};

ResourceUIScriptRuntime::ResourceUIScriptRuntime(fx::Resource* resource, IScriptHost* scriptHost)
	: m_resource(resource), m_scriptHost(scriptHost), m_refIdx(1)
{

}

result_t ResourceUIScriptRuntime::Create(IScriptHost* host)
{
	return FX_S_OK;
}

result_t ResourceUIScriptRuntime::Destroy()
{
	return FX_S_OK;
}

void* ResourceUIScriptRuntime::GetParentObject()
{
	return m_resource;
}

void ResourceUIScriptRuntime::SetParentObject(void*)
{

}

int ResourceUIScriptRuntime::GetInstanceId()
{
	return 0x6e7569; // 'nui'
}

result_t ResourceUIScriptRuntime::CallRef(int32_t refIdx, char* argsSerialized, uint32_t argsLength, char** retvalSerialized, uint32_t* retvalLength)
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
	ResUIResultCallback cb;

	{
		std::unique_lock<std::recursive_mutex> lock(m_refMutex);

		auto it = m_refs.find(refIdx);

		if (it == m_refs.end())
		{
			return FX_E_INVALIDARG;
		}

		cb = it->second->callback;
	}

	// convert the argument array from msgpack to JSON
	std::vector<msgpack::object> arguments;

	// unpack
	msgpack::unpacked unpacked;
	msgpack::unpack(unpacked, argsSerialized, argsLength);

	// convert
	unpacked.get().convert(arguments);

	// assume there's one argument
	if (arguments.size() == 1)
	{
		// get the object
		const msgpack::object& object = arguments[0];

		// and convert to a rapidjson object
		rapidjson::Document document;
		ConvertToJSON(object, document, document.GetAllocator());

		// write as a json string
		rapidjson::StringBuffer sb;
		rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

		if (document.Accept(writer))
		{
			cb(std::string(sb.GetString(), sb.GetSize()));
		}
	}

	return FX_S_OK;
}

result_t ResourceUIScriptRuntime::DuplicateRef(int32_t refIdx, int32_t* outRef)
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

result_t ResourceUIScriptRuntime::RemoveRef(int32_t refIdx)
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

std::string ResourceUIScriptRuntime::AddCallbackRef(ResUIResultCallback resultCallback)
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

ResourceUICallbackComponent::ResourceUICallbackComponent(fx::Resource* resource)
	: m_resource(resource)
{
	fwRefContainer<fx::ResourceScriptingComponent> scriptingComponent = resource->GetComponent<fx::ResourceScriptingComponent>();
	fx::OMPtr<ResourceUIScriptRuntime> runtime = fx::MakeNew<ResourceUIScriptRuntime>(resource, scriptingComponent->GetScriptHost().GetRef());

	// convert to IScriptRuntime
	fx::OMPtr<IScriptRuntime> baseRuntime;
	runtime.As(&baseRuntime);

	scriptingComponent->AddRuntime(baseRuntime);

	// and set the local runtime
	m_scriptRuntime = runtime.GetRef();
}

DECLARE_INSTANCE_TYPE(ResourceUICallbackComponent);

static ResUICallback MakeUICallback(fx::Resource* resource, const std::string& type)
{
	return [=] (const std::string& postData, ResUIResultCallback cb)
	{
		// get the event component
		fwRefContainer<fx::ResourceEventComponent> eventComponent = resource->GetComponent<fx::ResourceEventComponent>();
		fwRefContainer<ResourceUICallbackComponent> callbacks;
		fwRefContainer<ResourceUI> ui = resource->GetComponent<ResourceUI>();

		if (!ui->HasCallbacks())
		{
			callbacks = new ResourceUICallbackComponent(resource);
			resource->SetComponent(callbacks);

			ui->SetHasCallbacks(true);
		}
		else
		{
			callbacks = resource->GetComponent<ResourceUICallbackComponent>();
		}

		// generate a reference for the result
		std::string reference = callbacks->GetScriptRuntime()->AddCallbackRef(cb);

		// trigger the event with the contained payload
		msgpack::sbuffer buffer;
		msgpack::packer<msgpack::sbuffer> packer(buffer);

		// serialize the post object JSON as a msgpack object
		rapidjson::Document postJSON;
		postJSON.Parse(postData.c_str());

		if (!postJSON.HasParseError())
		{
			msgpack::object postObject;
			msgpack::zone zone;
			ConvertToMsgPack(postJSON, postObject, zone);

			// pack an array of arguments ([postData, cb])
			packer.pack_array(2).
				pack(postObject).
				pack_ext(reference.size(), 10).
					pack_ext_body(reference.c_str(), reference.size());

			eventComponent->QueueEvent("__cfx_nui:" + type, std::string(buffer.data(), buffer.size()), "nui");
		}
	};
}

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("REGISTER_NUI_CALLBACK_TYPE", [] (fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				fwRefContainer<ResourceUI> resourceUI = resource->GetComponent<ResourceUI>();

				if (resourceUI.GetRef())
				{
					std::string type = context.GetArgument<const char*>(0);

					resourceUI->AddCallback(type, MakeUICallback(resource, type));
				}
			}
		}
	});
});
