#include "StdInc.h"
#include "ResourceManager.h"
#include "scrEngine.h"

GtaThread* TheScriptManager;

class ScriptManager : public GtaThread
{
public:
	virtual void DoRun();

	virtual rage::eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount);

	virtual void Kill();
};

void ScriptManager::DoRun()
{
	TheResources.Tick();
}

rage::eThreadState ScriptManager::Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount)
{
	trace("script reset\n");

	//std::string gameInit = "gameInit";
	//TheResources.GetResource(gameInit)->Start();

	/*std::string resourcePath = "citizen:/resources/";
	TheResources.ScanResources(fiDevice::GetDevice("citizen:/setup2.xml", true), resourcePath);

	std::string gameInit = "gameInit";
	TheResources.GetResource(gameInit)->Start();

	std::string lovely = "lovely";
	TheResources.GetResource(lovely)->Start();*/

	ScriptEnvironment::SignalScriptReset.emit();

	return GtaThread::Reset(scriptHash, pArgs, argCount);
}

void ScriptManager::Kill()
{
	trace("script kill\n");

	return GtaThread::Kill();
}

static InitFunction initFunction([] ()
{
	rage::scrEngine::SetInitHook([] (void*)
	{
		// create the script manager
		static ScriptManager* scriptManager = new ScriptManager();

		TheScriptManager = scriptManager;

		rage::scrEngine::CreateThread(scriptManager);
	});
});