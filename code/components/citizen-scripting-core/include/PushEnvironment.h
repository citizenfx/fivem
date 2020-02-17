/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <optional>

FX_DEFINE_GUID(CLSID_ScriptRuntimeHandler,
			   0xc41e7194, 0x7556, 0x4c02, 0xba, 0x45, 0xa9, 0xc8, 0x4d, 0x18, 0xad, 0x43);

namespace fx
{
	class PushEnvironment
	{
	private:
		OMPtr<IScriptRuntimeHandler> m_handler;

		OMPtr<IScriptRuntime> m_curRuntime;

	public:
		inline PushEnvironment()
		{

		}

		template<typename TRuntime>
		PushEnvironment(OMPtr<TRuntime> runtime)
		{
			fx::MakeInterface(&m_handler, CLSID_ScriptRuntimeHandler);

			assert(FX_SUCCEEDED(runtime.As(&m_curRuntime)));

			m_handler->PushRuntime(m_curRuntime.GetRef());
		}

		template<typename TRuntime>
		PushEnvironment(TRuntime* runtime)
			: PushEnvironment(OMPtr<TRuntime>(runtime))
		{
			
		}

		inline PushEnvironment(PushEnvironment&& right)
			: m_handler(right.m_handler), m_curRuntime(right.m_curRuntime)
		{
			right.m_curRuntime = {};
			right.m_handler = {};
		}

		inline PushEnvironment& operator=(PushEnvironment&& right)
		{
			m_handler = right.m_handler;
			m_curRuntime = right.m_curRuntime;

			right.m_curRuntime = {};
			right.m_handler = {};

			return *this;
		}

	private:
		PushEnvironment(const OMPtr<IScriptRuntime>& curRuntime, const OMPtr<IScriptRuntimeHandler>& handler)
			: m_handler(handler), m_curRuntime(curRuntime)
		{

		}

	public:
		inline ~PushEnvironment()
		{
			if (m_curRuntime.GetRef())
			{
				m_handler->PopRuntime(m_curRuntime.GetRef());
			}
		}

		template<typename TRuntime>
		static bool TryPush(OMPtr<TRuntime> runtime, PushEnvironment& out)
		{
			OMPtr<IScriptRuntimeHandler> handler;
			fx::MakeInterface(&handler, CLSID_ScriptRuntimeHandler);

			OMPtr<IScriptRuntime> curRuntime;
			assert(FX_SUCCEEDED(runtime.As(&curRuntime)));

			if (FX_SUCCEEDED(handler->TryPushRuntime(curRuntime.GetRef())))
			{
				out = PushEnvironment{ curRuntime, handler };
				return true;
			}

			return false;
		}
	};

	template<typename TRuntime>
	inline result_t GetCurrentScriptRuntime(OMPtr<TRuntime>* runtime)
	{
		static OMPtr<IScriptRuntimeHandler> handler;

		if (handler.GetRef() == nullptr)
		{
			fx::MakeInterface(&handler, CLSID_ScriptRuntimeHandler);
		}

		OMPtr<IScriptRuntime> runtimePtr;

		result_t hr = handler->GetCurrentRuntime(runtimePtr.GetAddressOf());

		if (FX_SUCCEEDED(hr))
		{
			if constexpr (std::is_same_v<TRuntime, IScriptRuntime>)
			{
				*runtime = runtimePtr;
			}
			else
			{
				assert(FX_SUCCEEDED(runtimePtr.As(runtime)));
			}
		}

		return hr;
	}

	template<typename TRuntime>
	inline result_t GetInvokingScriptRuntime(OMPtr<TRuntime>* runtime)
	{
		static OMPtr<IScriptRuntimeHandler> handler;

		if (handler.GetRef() == nullptr)
		{
			fx::MakeInterface(&handler, CLSID_ScriptRuntimeHandler);
		}

		OMPtr<IScriptRuntime> runtimePtr;

		result_t hr = handler->GetInvokingRuntime(runtimePtr.GetAddressOf());

		*runtime = nullptr;

		if (FX_SUCCEEDED(hr) && runtimePtr.GetRef())
		{
			assert(FX_SUCCEEDED(runtimePtr.As(runtime)));
		}

		return hr;
	}
}
