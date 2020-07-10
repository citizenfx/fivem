/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_GTA_MISSION_CLEANUP_FIVE
#define MISCLEAN_EXPORT DLL_EXPORT
#else
#define MISCLEAN_EXPORT DLL_IMPORT
#endif

#include <scrEngine.h>

class GtaThread;

namespace rage
{
	class scrThread;

	class scriptId
	{
	public:
		virtual ~scriptId() = 0;

		virtual void m_8() = 0;

		virtual void m_10() = 0;

		virtual void m_18() = 0;

		virtual uint32_t* GetIdentifier(uint32_t* outValue) = 0;
	};

	class scriptResource
	{
	public:
		virtual ~scriptResource() = default;

		virtual const char* GetResourceName() = 0;

		virtual void* GetInvalidReference() = 0;

		virtual scriptResource* Clone() = 0;

		virtual void Create() = 0;

		virtual void Destroy() = 0;

		virtual void Detach() = 0;

		virtual bool DetachOnCleanup() = 0;

		virtual bool LeaveForOtherScripts() = 0;

		virtual uint32_t* GetStreamingIndex(uint32_t* outIndex) = 0;

	public:
		char m_pad[24];

		scriptResource* next;
	};

	class scriptHandler
	{
	public:
		virtual ~scriptHandler() = 0;

		virtual void m_8() = 0;

		virtual void m_10() = 0;

		// aka 'Destroy'
		virtual void CleanupObjectList() = 0;

		virtual scriptId* GetScriptId() = 0;

		virtual scriptId* GetScriptId_2() = 0;

		virtual bool IsNetworkScript() = 0;

		virtual void CreateNetComponent() = 0;

		// more functions that aren't needed
		/*
		m_38

		m_40

		CleanupMissionState

		AddScriptObject

		m_58

		AddScriptResource

		...
		*/

	private:
		char pad[40];

		rage::scriptResource* m_resourceFirst;
		rage::scriptResource* m_resourceHead;
		uint32_t m_numResources;

	public:
		inline void ForAllResources(const std::function<void(rage::scriptResource*)>& cb)
		{
			for (auto ptr = m_resourceHead; ptr; ptr = ptr->next)
			{
				cb(ptr);
			}
		}
	};

	// to allow construction
	class MISCLEAN_EXPORT scriptHandlerImplemented : public scriptHandler
	{
	public:
		virtual ~scriptHandlerImplemented();

		virtual void m_8();

		virtual void m_10();

		// aka 'Destroy'
		virtual void CleanupObjectList();

		virtual scriptId* GetScriptId();

		virtual scriptId* GetScriptId_2();

		virtual bool IsNetworkScript();

		virtual inline void CreateNetComponent()
		{

		}
	};
}

class MISCLEAN_EXPORT CGameScriptHandlerNetwork : public rage::scriptHandlerImplemented
{
public:
	CGameScriptHandlerNetwork(rage::scrThread* thread);

	// size is ignored as this is a pool allocator
	void* operator new(size_t size);
};

class MISCLEAN_EXPORT CGameScriptHandlerMgr : public rage::scriptHandlerMgr
{
private:
	struct scriptHandlerHashMap
	{
		void MISCLEAN_EXPORT Set(uint32_t* hash, rage::scriptHandler** handler);
	};

private:
	char m_pad[72];

	// actually atHashMap<rage::scriptHandler*>
	scriptHandlerHashMap* m_handlers;

public:
	static CGameScriptHandlerMgr* GetInstance();

	inline void AddScriptHandler(rage::scriptHandler* handler)
	{
		uint32_t hashStorage;
		rage::scriptId* scriptId = handler->GetScriptId();

		m_handlers->Set(scriptId->GetIdentifier(&hashStorage), &handler);
	}
};

extern MISCLEAN_EXPORT fwEvent<rage::scrThread*, const std::string&> OnCreateResourceThread;
extern MISCLEAN_EXPORT fwEvent<rage::scrThread*> OnDeleteResourceThread;
