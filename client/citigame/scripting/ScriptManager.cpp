#include "StdInc.h"
#include "scrEngine.h"

class ScriptManager : public GtaThread
{
public:
	virtual void DoRun();

	virtual rage::eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount);

	virtual void Kill();
};

void ScriptManager::DoRun()
{

}

rage::eThreadState ScriptManager::Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount)
{
	return GtaThread::Reset(scriptHash, pArgs, argCount);
}

void ScriptManager::Kill()
{
	GtaThread::Kill();
}

static InitFunction initFunction([] ()
{
	rage::scrEngine::SetInitHook([] (void*)
	{
		// create the script manager
		static ScriptManager* scriptManager = new ScriptManager();

		atexit([] ()
		{
			delete scriptManager;
		});

		rage::scrEngine::CreateThread(scriptManager);
	});
});