/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <include/v8-platform.h>

class
#ifdef COMPILING_CITIZEN_SCRIPTING_V8
	DLL_EXPORT
#else
	DLL_IMPORT
#endif
	V8Platform : public v8::Platform
{
private:
	uint64_t m_startTime;

	std::map<v8::Isolate*, std::vector<std::pair<uint64_t, v8::Task*>>> m_foregroundTasks;

public:
	V8Platform();

	void RunTickForIsolate(v8::Isolate* isolate);

	virtual void CallOnBackgroundThread(v8::Task* task, ExpectedRuntime expected_runtime) override;

	virtual void CallOnForegroundThread(v8::Isolate* isolate, v8::Task* task) override;

	virtual void CallDelayedOnForegroundThread(v8::Isolate* isolate, v8::Task* task, double delay_in_seconds) override;

	virtual double MonotonicallyIncreasingTime() override;
};