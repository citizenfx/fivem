#pragma once

#include <ResourceManager.h>

#include <msgpack.hpp>

#include <fxScripting.h>

#include <mutex>

namespace fx
{
	class ResourceCallbackScriptRuntime : public fx::OMClass<ResourceCallbackScriptRuntime, IScriptRuntime, IScriptRefRuntime>
	{
	private:
		fx::Resource* m_resource;

		IScriptHost* m_scriptHost;

		std::map<int32_t, std::function<void(const msgpack::unpacked&)>> m_refs;

		std::recursive_mutex m_refMutex;

		int32_t m_refIdx;

	public:
		ResourceCallbackScriptRuntime(fx::Resource* resource, IScriptHost* scriptHost);

		NS_DECL_ISCRIPTRUNTIME;

		NS_DECL_ISCRIPTREFRUNTIME;

		std::string AddCallbackRef(const std::function<void(const msgpack::unpacked&)>& resultCallback);
	};

	class ResourceCallbackComponent : public fwRefCountable
	{
	private:
		fwRefContainer<Resource> m_resource;

		ResourceManager* m_manager;

		ResourceCallbackScriptRuntime* m_scriptRuntime;

	public:
		struct CallbackRef
		{
			std::string reference;

			template<typename Packer>
			void msgpack_pack(Packer& pk) const
			{
				pk
					.pack_ext(reference.size(), 11)
					.pack_ext_body(reference.c_str(), reference.size());
			}

			void msgpack_unpack(msgpack::object const& o)
			{

			}
		};

	public:
		ResourceCallbackComponent(ResourceManager* resource);

		inline ResourceCallbackScriptRuntime* GetScriptRuntime()
		{
			return m_scriptRuntime;
		}

		virtual CallbackRef CreateCallback(const std::function<void(const msgpack::unpacked&)>&);
	};
}

DECLARE_INSTANCE_TYPE(fx::ResourceCallbackComponent);
