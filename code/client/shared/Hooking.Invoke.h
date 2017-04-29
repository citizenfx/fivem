/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "HookFunction.h"

namespace hook
{
	namespace details
	{
		class function_stub_base
		{
		protected:
			void* m_functionAddress;

		public:
			inline void SetFunctionAddress(void* address)
			{
				m_functionAddress = address;
			}
		};

		class StubInitFunction : public HookFunctionBase
		{
		private:
			void*(*m_function)();

			function_stub_base* m_stub;

		public:
			StubInitFunction(function_stub_base* stub, void*(*function)())
			{
				m_function = function;
				m_stub = stub;
			}

			virtual void Run()
			{
				m_stub->SetFunctionAddress(m_function());
			}
		};

		template<typename TRet, typename... Args>
		class thiscall_stub_ : public function_stub_base
		{
		private:
			StubInitFunction m_hookFunction;

		public:
			thiscall_stub_(void*(*getter)())
				: m_hookFunction(this, getter)
			{

			}

			inline TRet operator()(Args... args)
			{
				return reinterpret_cast<TRet(__thiscall*)(Args...)>(m_functionAddress)(args...);
			}
		};

		template<typename TRet, typename... Args>
		class cdecl_stub_ : public function_stub_base
		{
		private:
			StubInitFunction m_hookFunction;

		public:
			cdecl_stub_(void*(*getter)())
				: m_hookFunction(this, getter)
			{

			}

			inline TRet operator()(Args... args)
			{
				return reinterpret_cast<TRet(__cdecl*)(Args...)>(m_functionAddress)(args...);
			}
		};
	}

	template<typename TRet>
	class thiscall_stub {};

	template<typename TRet, typename... Args>
	class thiscall_stub<TRet(Args...)>
		: public details::thiscall_stub_<TRet, Args...>
	{
	public:
		thiscall_stub(void*(*getter)())
			: thiscall_stub_(getter)
		{

		}
	};

	template<typename TRet>
	class cdecl_stub {};

	template<typename TRet, typename... Args>
	class cdecl_stub<TRet(Args...)>
		: public details::cdecl_stub_ < TRet, Args... >
	{
	public:
		cdecl_stub(void*(*getter)())
			: cdecl_stub_(getter)
		{

		}
	};
}