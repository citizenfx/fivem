#include "StdInc.h"
#include "ResourceManager.h"
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
	TheResources.Tick();

	static bool testState;

	if (GetAsyncKeyState(VK_F11))
	{
		if (!testState)
		{
			((void(*)(int, int, int))0x423CE0)(1, 0, 0);

			*(BYTE*)0x7BD9F0 = 0xC3;
			*(BYTE*)0x18A823A = 1;
			*(BYTE*)0x18A825C = 1;
			testState = true;
		}
	}
}

rage::eThreadState ScriptManager::Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount)
{
	std::string gameInit = "gameInit";
	TheResources.GetResource(gameInit)->Start();

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
	return GtaThread::Kill();
}

static InitFunction initFunction([] ()
{
	rage::scrEngine::SetInitHook([] (void*)
	{
		// create the script manager
		static ScriptManager* scriptManager = new ScriptManager();

		rage::scrEngine::CreateThread(scriptManager);
	});
});