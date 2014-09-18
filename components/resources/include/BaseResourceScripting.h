#pragma once

class CMissionCleanup;
class GtaThread;

// temp
typedef int ScriptFunctionRef;

class
#ifdef COMPILING_RESOURCES
	__declspec(dllexport)
#endif
	BaseScriptEnvironment : public fwRefCountable
{
public:
	virtual ~BaseScriptEnvironment();

	virtual const char* GetEnvironmentName() = 0;

	virtual bool Create() = 0;

	virtual void Destroy() = 0;

	virtual void Tick() = 0;

	virtual void TriggerEvent(fwString& eventName, fwString& argsSerialized, int source) = 0;

	virtual bool DoInitFile(bool isPreParse) = 0;

	virtual bool LoadScripts() = 0;

	virtual fwString CallExport(ScriptFunctionRef ref, fwString& argsSerialized) = 0;

	virtual uint32_t GetInstanceId() = 0;

	virtual ScriptFunctionRef DuplicateRef(ScriptFunctionRef ref) = 0;

	virtual void RemoveRef(ScriptFunctionRef ref) = 0;

	virtual CMissionCleanup* GetMissionCleanup();

	virtual void Activate();

public:
	static BaseScriptEnvironment* GetCurrentEnvironment();

	static BaseScriptEnvironment* GetInvokingEnvironment();
};

class
#ifdef COMPILING_RESOURCES
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	PushEnvironment
{
protected:
	BaseScriptEnvironment* m_oldEnvironment;

public:
	PushEnvironment(BaseScriptEnvironment* environment);

	virtual ~PushEnvironment();

	static fwEvent<> OnDeactivateLastEnvironment;
};

extern
#ifdef COMPILING_RESOURCES
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	GtaThread* TheScriptManager;

extern
#ifdef COMPILING_RESOURCES
__declspec(dllexport)
#else
__declspec(dllimport)
#endif
	fwEvent<fwString, bool> OnSetWorldAssetConfig;