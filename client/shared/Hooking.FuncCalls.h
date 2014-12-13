/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <jitasm.h>
#include <stdint.h>

namespace hook
{
struct passed
{
	template<typename ...T> passed(T...) {}
};

class CallStubEx : public jitasm::Frontend
{
private:
	void* m_callback;
	void* m_userData;

	size_t m_argumentSize;

public:
	CallStubEx()
	{

	}

	inline void SetCallback(void* cb, void* userData) { m_callback = cb; m_userData = userData; }

	inline void SetArgumentSize(size_t size) { m_argumentSize = size; }

	virtual void InternalMain()
	{
		int argStart = 4;

		pushad();
		argStart += 8 * sizeof(uintptr_t);

		push((uintptr_t)m_userData);
		argStart += 4;

		sub(esp, m_argumentSize);
		argStart += m_argumentSize;

		lea(esi, dword_ptr[esp]);
		add(esi, argStart);

		lea(edi, dword_ptr[esp]);

		push(ecx);
		mov(ecx, m_argumentSize);

		rep_movsb();

		pop(ecx);

		mov(eax, (uintptr_t)m_callback);
		call(eax);

		popad();

		ret(m_argumentSize);
	}

	void Inject(uintptr_t address)
	{
		auto assembly = new FunctionAssembly(*this);

		int origAddress = *(int*)(address + 1);
		origAddress += 5;
		origAddress += address;

		// and patch
		put<int>(address + 1, (uintptr_t)assembly->GetCode() - (uintptr_t)get_adjusted(address) - 5);
	}
};

template<typename TRet, typename TClass, typename ...Args>
struct thiscall
{
	typedef std::function<TRet(Args...)> TOrigFunc;
	typedef TRet(__thiscall* TOrigCB)(void*, Args...);
	typedef std::function<TRet(TClass*, Args...)> THookFunc;

private:
	struct hookdata
	{
		THookFunc hookFunc;
		TOrigCB origCB;
	};

	static TRet __thiscall InternalHook(void* class_, Args... args, hookdata* hookDataPtr)
	{
		hookdata* data = static_cast<hookdata*>(hookDataPtr);

		return data->hookFunc((TClass*)class_, args...);
	}

public:
	static void inject(uintptr_t address, THookFunc hookFunc)
	{
		auto hookData = new hookdata;
		hookData->hookFunc = hookFunc;

		size_t argSize = 0;

		passed{ ([&]
		{
			int size = min(sizeof(Args), sizeof(uintptr_t));

			argSize += size;
		}(), 1)... };

		CallStubEx callStub;
		callStub.SetCallback((void*)InternalHook, hookData);
		callStub.SetArgumentSize(argSize);

		callStub.Inject(address);
	}
};

template<typename TRet, typename TClass>
struct thiscall<TRet, TClass, void>
{
	typedef std::function<TRet()> TOrigFunc;
	typedef TRet(__thiscall* TOrigCB)(void*);
	typedef std::function<TRet(TClass*)> THookFunc;

private:
	struct hookdata
	{
		THookFunc hookFunc;
		TOrigCB origCB;
	};

	static TRet __thiscall InternalHook(void* class_, hookdata* hookDataPtr)
	{
		hookdata* data = static_cast<hookdata*>(hookDataPtr);

		return data->hookFunc((TClass*)class_);
	}

public:
	static void inject(uintptr_t address, THookFunc hookFunc)
	{
		auto hookData = new hookdata;
		hookData->hookFunc = hookFunc;

		CallStubEx callStub;
		callStub.SetCallback((void*)InternalHook, hookData);
		callStub.SetArgumentSize(0);

		callStub.Inject(address);
	}
};
}