/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "V8Platform.h"

V8Platform::V8Platform()
{
	m_startTime = GetTickCount64();
}

void V8Platform::RunTickForIsolate(v8::Isolate* isolate)
{
	// get the entries for the isolate
	auto& tickEntries = m_foregroundTasks[isolate];
	uint64_t curTime = GetTickCount64();

	// iterate through
	for (auto it = tickEntries.begin(); it != tickEntries.end(); )
	{
		// if it's time to run
		if (it->first >= curTime)
		{
			// run and free
			it->second->Run();
			delete it->second;

			it = tickEntries.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void V8Platform::CallOnBackgroundThread(v8::Task* task, ExpectedRuntime expected_runtime)
{
	// enqueue on the Windows thread pool
	QueueUserWorkItem([] (void* taskPtr) -> DWORD
	{
		// cast and run
		v8::Task* task = reinterpret_cast<v8::Task*>(taskPtr);
		task->Run();

		// delete
		delete task;

		return 0;
	}, task, (expected_runtime == kLongRunningTask) ? WT_EXECUTELONGFUNCTION : 0);
}

void V8Platform::CallOnForegroundThread(v8::Isolate* isolate, v8::Task* task)
{
	CallDelayedOnForegroundThread(isolate, task, 0.0);
}

void V8Platform::CallDelayedOnForegroundThread(v8::Isolate* isolate, v8::Task* task, double delay_in_seconds)
{
	// get the task list
	auto& taskList = m_foregroundTasks[isolate];

	// add an entry to the list
	uint64_t timeToExecute = GetTickCount64() + static_cast<uint64_t>(delay_in_seconds * 1000.0);

	taskList.push_back({ timeToExecute, task });
}

double V8Platform::MonotonicallyIncreasingTime()
{
	return (GetTickCount64() - m_startTime) / 1000.0;
}