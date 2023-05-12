/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <scrEngine.h>
#include <ScriptHandlerMgr.h>
#include <nutsnbolts.h>
#include <DrawCommands.h>

#include "ICoreGameInit.h"
#include <InputHook.h>
#include <IteratorView.h>

#include <LaunchMode.h>
#include <CrossBuildRuntime.h>

#include <memory>

#include <Error.h>

#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>

class FishScript
{
private:
	HANDLE m_fiber;
	uint32_t m_wakeAt;
	void(*m_function)();

private:
	void Run();
	
public:
	inline FishScript(void(*function)())
	{
		m_fiber = nullptr;
		m_function = function;
		m_wakeAt = timeGetTime();
	}

	inline ~FishScript()
	{
		if (m_fiber)
		{
			DeleteFiber(m_fiber);
		}
	}

	void Tick();

	void Yield(uint32_t time);

	inline void(*GetFunction())()
	{
		return m_function;
	}
};

static HANDLE g_mainFiber;
static FishScript* g_fishScript;

void FishScript::Tick()
{
	if (!CfxIsSinglePlayer())
	{
		if (!Instance<ICoreGameInit>::Get()->ShAllowed) return;
		if (!Instance<ICoreGameInit>::Get()->HasVariable("networkInited")) return;
	}

	if (g_mainFiber == nullptr)
	{
		g_mainFiber = ConvertThreadToFiber(nullptr);
	}

	if (timeGetTime() < m_wakeAt)
	{
		return;
	}

	if (m_fiber)
	{
		g_fishScript = this;
		SwitchToFiber(m_fiber);
		g_fishScript = nullptr;
	}
	else
	{
		m_fiber = CreateFiber(NULL, [] (LPVOID handler)
		{
			reinterpret_cast<FishScript*>(handler)->Run();
		}, this);
	}
}

void FishScript::Run()
{
	m_function();
}

void FishScript::Yield(uint32_t time)
{
	m_wakeAt = timeGetTime() + time;
	SwitchToFiber(g_mainFiber);
}

class FishThread : public GtaThread
{
private:
	std::vector<std::shared_ptr<FishScript>> m_scripts;

	bool m_initedNet;

	bool m_hasScriptHandler;

public:
	virtual void DoRun() override
	{
		if (!m_hasScriptHandler)
		{
			CGameScriptHandlerMgr::GetInstance()->AttachScript(this);

			m_hasScriptHandler = true;
		}

		if (!m_initedNet)
		{
			// make this a network script
			NativeInvoke::Invoke<0x1CA59E306ECB80A5, int>(32, false, -1);

			m_initedNet = true;
		}

		std::vector<std::shared_ptr<FishScript>> thisIterScripts(m_scripts);

		for (auto& script : thisIterScripts)
		{
			script->Tick();
		}
	}

	virtual void Kill() override;

	virtual rage::eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount) override;

	void AddScript(void(*fn)());

	void RemoveScript(void(*fn)());
};

rage::eThreadState FishThread::Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount)
{
	// uninit net
	m_initedNet = false;

	// collect all script functions
	std::vector<void(*)()> scriptFunctions;

	for (auto&& script : m_scripts)
	{
		scriptFunctions.push_back(script->GetFunction());
	}

	// clear the script list
	m_scripts.clear();

	// start all script functions
	for (auto&& fn : scriptFunctions)
	{
		AddScript(fn);
	}

	return GtaThread::Reset(scriptHash, pArgs, argCount);
}

void FishThread::Kill()
{
	m_hasScriptHandler = false;
}

void FishThread::AddScript(void(*fn)())
{
	m_scripts.push_back(std::make_shared<FishScript>(fn));
}

void FishThread::RemoveScript(void(*fn)())
{
	for (auto it = m_scripts.begin(); it != m_scripts.end(); it++)
	{
		if ((*it)->GetFunction() == fn)
		{
			m_scripts.erase(it);
			return;
		}
	}
}

static FishThread g_fish;

static std::multimap<HMODULE, void(*)()> g_modulesToFunctions;

void DLL_EXPORT scriptWait(unsigned long waitTime)
{
	if (g_fishScript)
	{
		g_fishScript->Yield(waitTime);
	}
	else
	{
		const char* moduleBaseString = "";
		HMODULE module;

		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)_ReturnAddress(), &module))
		{
			char filename[MAX_PATH];
			GetModuleFileNameA(module, filename, _countof(filename));

			moduleBaseString = va(" - %s+%X", strrchr(filename, '\\') + 1, (char*)_ReturnAddress() - (char*)module);
		}

		GlobalError("Attempted to call ViSH scriptWait() while no ViSH script hook thread was active. (called from %p%s)", _ReturnAddress(), moduleBaseString);
	}
}

void DLL_EXPORT scriptRegister(HMODULE hMod, void(*function)())
{
	g_fish.AddScript(function);
	g_modulesToFunctions.insert({ hMod, function });
}

void DLL_EXPORT scriptRegisterAdditionalThread(HMODULE hMod, void(*function)())
{
	scriptRegister(hMod, function);
}

void DLL_EXPORT scriptUnregister(void(*function)())
{
	g_fish.RemoveScript(function);
}

void DLL_EXPORT scriptUnregister(HMODULE hMod) // 372+ API
{
	auto iteratorView = fx::GetIteratorView(g_modulesToFunctions.equal_range(hMod));

	for (auto& entry : iteratorView)
	{
		g_fish.RemoveScript(entry.second);
	}
}

// CitizenFX doesn't have globals normally - this is therefore a no-op
DLL_EXPORT uint64_t* getGlobalPtr(int)
{
	// let the user party on a dummy global (allocate some buffer size at the end, though)
	static uint64_t dummyGlobal[128];

	return dummyGlobal;
}

enum eGameVersion : int
{
	VER_1_0_372_2_NOSTEAM = 5,
	VER_1_0_1604_0_NOSTEAM = 47,
	VER_1_0_2060_0_NOSTEAM = 60,
	VER_1_0_2189_0_NOSTEAM = 64,
	VER_1_0_2372_0_NOSTEAM = 70,
	VER_1_0_2545_0_NOSTEAM = 72,
	VER_1_0_2612_1_NOSTEAM = 74,
	VER_1_0_2699_0_NOSTEAM = 78,
	VER_1_0_2802_0_NOSTEAM = 80,
};

// ScriptHookV uses incremental numbers instead of build
DLL_EXPORT eGameVersion getGameVersion()
{
	if (xbr::IsGameBuildOrGreater<2802>()) return VER_1_0_2802_0_NOSTEAM;
	if (xbr::IsGameBuildOrGreater<2699>()) return VER_1_0_2699_0_NOSTEAM;
	if (xbr::IsGameBuildOrGreater<2612>()) return VER_1_0_2612_1_NOSTEAM;
	if (xbr::IsGameBuildOrGreater<2545>()) return VER_1_0_2545_0_NOSTEAM;
	if (xbr::IsGameBuildOrGreater<2372>()) return VER_1_0_2372_0_NOSTEAM;
	if (xbr::IsGameBuildOrGreater<2189>()) return VER_1_0_2189_0_NOSTEAM;
	if (xbr::IsGameBuildOrGreater<2060>()) return VER_1_0_2060_0_NOSTEAM;
	if (xbr::IsGameBuildOrGreater<1604>()) return VER_1_0_1604_0_NOSTEAM;
	if (xbr::IsGameBuildOrGreater<372>()) return VER_1_0_372_2_NOSTEAM;

	return VER_1_0_1604_0_NOSTEAM; // Default build
}

class FishNativeContext : public NativeContext
{
public:
	FishNativeContext()
		: NativeContext()
	{

	}

	void Reset()
	{
		m_nArgCount = 0;
		m_nDataCount = 0;
	}

	inline void* GetResultPointer()
	{
		return m_pReturn;
	}
};

static FishNativeContext g_context;
static uint64_t g_hash;

void DLL_EXPORT nativeInit(uint64_t hash)
{
	g_context.Reset();
	g_hash = hash;
}

void DLL_EXPORT nativePush64(uint64_t value)
{
	g_context.Push(value);
}

DLL_EXPORT uint64_t* nativeCall()
{
	auto fn = rage::scrEngine::GetNativeHandler(g_hash);

	bool valid = true;
	static bool lastWasHash = false;

	if (valid)
	{
		if (g_hash == 0x00A1CADD00108836)
		{
			if (lastWasHash)
			{
				valid = false;
			}
		}

		// sequence: GET_HASH_KEY(a_c_sharktiger), PLAYER_ID, SET_PLAYER_MODEL

		if (g_hash == 0xD24D37CC275948CC && strcmp(g_context.GetArgument<const char*>(0), "a_c_sharktiger") == 0)
		{
			lastWasHash = true;
		}
		else if (g_hash != 0x4F8644AF03D0E0D6)
		{
			lastWasHash = false;
		}
	}

	if (valid)
	{
		if (!CfxIsSinglePlayer())
		{
			if (!Instance<ICoreGameInit>::Get()->ShAllowed) valid = false;
			if (!Instance<ICoreGameInit>::Get()->HasVariable("networkInited")) valid = false;
		}
	}

	if (fn != 0 && valid)
	{
		void* returnAddress = _ReturnAddress();

#ifndef _DEBUG
		__try
		{
#endif
			fn(&g_context);
#ifndef _DEBUG
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			const char* moduleBaseString = "";
			HMODULE module;

			if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)returnAddress, &module))
			{
				char filename[MAX_PATH];
				GetModuleFileNameA(module, filename, _countof(filename));

				moduleBaseString = va(" - %s+%X", strrchr(filename, '\\') + 1, (char*)returnAddress - (char*)module);
			}

			FatalError("An exception occurred executing native 0x%llx in a ViSH plugin (%p%s). The game has been terminated.", g_hash, returnAddress, moduleBaseString);
		}
#endif

		g_context.SetVectorResults();
	}

	return reinterpret_cast<uint64_t*>(g_context.GetResultPointer());
}

typedef void(*TKeyboardFn)(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow);

static std::set<TKeyboardFn> g_keyboardFunctions;

void DLL_EXPORT keyboardHandlerRegister(TKeyboardFn function)
{
	g_keyboardFunctions.insert(function);
}

void DLL_EXPORT keyboardHandlerUnregister(TKeyboardFn function)
{
	g_keyboardFunctions.erase(function);
}

// dummy pool functions (need implementation)
int DLL_EXPORT worldGetAllVehicles(int* array, int arraySize)
{
	return 0;
}

int DLL_EXPORT worldGetAllPeds(int* array, int arraySize)
{
	return 0;
}

int DLL_EXPORT worldGetAllObjects(int* array, int arraySize)
{
	return 0;
}

typedef void(*PresentCallback)(void*);

static std::set<PresentCallback> g_presentCallbacks;

void DLL_EXPORT presentCallbackRegister(PresentCallback cb)
{
	g_presentCallbacks.insert(cb);
}

void DLL_EXPORT presentCallbackUnregister(PresentCallback cb)
{
	g_presentCallbacks.erase(cb);
}

static InitFunction initFunction([]()
{
	rage::scrEngine::OnScriptInit.Connect([]()
	{
		rage::scrEngine::CreateThread(&g_fish);
	});

	InputHook::QueryInputTarget.Connect([](std::vector<InputTarget*>& targets)
	{
		static struct : InputTarget
		{
			virtual inline void KeyDown(UINT vKey, UINT scanCode) override
			{
				auto functions = g_keyboardFunctions;

				for (auto& function : functions)
				{
					function(vKey, 0, 0, FALSE, FALSE, FALSE, FALSE);
				}
			}

			virtual inline void KeyUp(UINT vKey, UINT scanCode) override
			{
				auto functions = g_keyboardFunctions;

				for (auto& function : functions)
				{
					function(vKey, 0, 0, FALSE, FALSE, FALSE, TRUE);
				}
			}

		} tgt;

		targets.push_back(&tgt);
	});

	InputHook::DeprecatedOnWndProc.Connect([] (HWND, UINT wMsg, WPARAM wParam, LPARAM lParam, bool&, LRESULT& result)
	{
		if (wMsg == WM_KEYDOWN || wMsg == WM_KEYUP || wMsg == WM_SYSKEYDOWN || wMsg == WM_SYSKEYUP)
		{
			auto functions = g_keyboardFunctions;

			for (auto& function : functions)
			{
				function(wParam, lParam & 0xFFFF, (lParam >> 16) & 0xFF, (lParam >> 24) & 1, (wMsg == WM_SYSKEYDOWN || wMsg == WM_SYSKEYUP), (lParam >> 30) & 1, (wMsg == WM_SYSKEYUP || wMsg == WM_KEYUP));
			}
		}
	});
});
