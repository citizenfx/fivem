#pragma once

#include <scrThread.h>
#include <sysAllocator.h>

#ifdef COMPILING_GTA_MISSION_CLEANUP_NY
#define CLEANUP_EXPORT __declspec(dllexport)
#else
#define CLEANUP_EXPORT __declspec(dllimport)
#endif

class CLEANUP_EXPORT CMissionCleanupEntry
{
private:
	uint8_t m_type;
	uint32_t m_data;
	GtaThread* m_thread;
	uint32_t m_unk1;
	uint32_t m_unkAlways0;
	uint8_t pad[24];

public:
	CMissionCleanupEntry();

	void Reset();
};

class CLEANUP_EXPORT CMissionCleanup : public rage::sysUseAllocator
{
private:
	CMissionCleanupEntry m_scriptEntries[256];
	int m_unknown;
	CMissionCleanupEntry m_dependentEntries[200];

public:
	CMissionCleanup();

	virtual ~CMissionCleanup();

	virtual void CleanUp(GtaThread* scriptThread);

	void Initialize();

	void CheckIfCollisionHasLoadedForMissionObjects();

public:
	//
	// An event which should return a mission cleanup instance to be used.
	// Callbacks should ensure to only return one if they actually intend to use
	// it, as there's no checking for multiple triggers.
	//
	static fwEvent<CMissionCleanup*&> OnQueryMissionCleanup;

	//
	// An event, upon receipt of which the handlers should call 
	// CheckIfCollisionHasLoadedForMissionObjects on all CMissionCleanup instances
	// they own.
	//
	static fwEvent<> OnCheckCollision;
};