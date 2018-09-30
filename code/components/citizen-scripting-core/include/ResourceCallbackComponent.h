#pragma once

#include <ResourceManager.h>

#include <msgpack.hpp>

#include <fxScripting.h>

#include <ScriptEngine.h>

#include <mutex>

namespace fx
{
	class ResourceCallbackScriptRuntime : public fx::OMClass<ResourceCallbackScriptRuntime, IScriptRuntime, IScriptRefRuntime>
	{
	private:
		struct RefData
		{
			std::atomic<int32_t> refCount;
			std::function<void(const msgpack::unpacked&)> callback;

			RefData(std::function<void(const msgpack::unpacked&)> cb)
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

	class FunctionRef
	{
	public:
		inline FunctionRef()
		{
			
		}

		inline explicit FunctionRef(const std::string& ref)
			: m_ref(ref)
		{
			fx::ScriptContextBuffer cxt;
			cxt.Push(ref.c_str());

			static auto nativeHandler = ScriptEngine::GetNativeHandler(HashString("DUPLICATE_FUNCTION_REFERENCE"));
			(*nativeHandler)(cxt);
		}

		FunctionRef(const FunctionRef&) = delete;

		inline FunctionRef(FunctionRef&& oldRef)
		{
			m_ref.swap(oldRef.m_ref);
		}

		inline FunctionRef& operator=(FunctionRef&& right)
		{
			m_ref.swap(right.m_ref);

			return *this;
		}

		inline ~FunctionRef()
		{
			if (!m_ref.empty())
			{
				fx::ScriptContextBuffer cxt;
				cxt.Push(m_ref.c_str());

				static auto nativeHandler = ScriptEngine::GetNativeHandler(HashString("DELETE_FUNCTION_REFERENCE"));
				(*nativeHandler)(cxt);
			}
		}

		inline operator bool() const
		{
			return (!m_ref.empty());
		}

		inline const std::string& GetRef() const
		{
			assert(!m_ref.empty());

			return m_ref;
		}

	private:
		std::string m_ref;
	};
}

DECLARE_INSTANCE_TYPE(fx::ResourceCallbackComponent);
