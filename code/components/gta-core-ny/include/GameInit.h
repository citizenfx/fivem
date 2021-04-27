#pragma once

#include <grcTexture.h>
#include <ICoreGameInit.h>

class 
#ifdef COMPILING_GTA_CORE_NY
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	GameInit : public ICoreGameInit
{
public:
	GameInit();

	virtual bool GetGameLoaded() override;

	virtual void LoadGameFirstLaunch(bool (*callBeforeLoad)()) override;

	static void SetLoadScreens();

	virtual void ReloadGame() override;

	static void MurderGame();

	virtual void KillNetwork(const wchar_t* reason) override;

	static void PrepareSwitchToCustomLoad(rage::grcTexture* texture);

	static rage::grcTexture* GetLastCustomLoadTexture();

	static bool ShouldSwitchToCustomLoad();

	inline virtual bool TriggerError(const char* errorString) override { return false; }

	inline virtual bool TryDisconnect()
	{
		if (tryDisconnectFlag > 0)
		{
			tryDisconnectFlag--;
			return false;
		}
		else
		{
			return true;
		}
	}

	virtual void SetPreventSavePointer(bool* preventSaveValue) override;

private:
	int tryDisconnectFlag;
};

extern
	#ifdef COMPILING_GTA_CORE_NY
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	fwEvent<const char*> OnKillNetwork;

extern
	#ifdef COMPILING_GTA_CORE_NY
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	fwEvent<> OnPreGameLoad;

extern
	#ifdef COMPILING_GTA_CORE_NY
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	fwEvent<> OnKillNetworkDone;
