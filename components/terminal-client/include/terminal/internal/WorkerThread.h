/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <functional>
#include <mutex>
#include <thread>
#include <concurrent_vector.h>

namespace terminal
{
class WorkerThread
{
public:
	typedef std::function<void(HANDLE)> HandleFn;

private:
	int m_residualTimeout;

	std::thread m_threadHandle;

	std::mutex m_waitHandleListMutex;

	std::vector<std::pair<HANDLE, HandleFn>> m_waitHandleList;

	concurrency::concurrent_vector<std::function<void()>> m_tickCallbackList;

	concurrency::concurrent_vector<HANDLE> m_removeHandleList;

private:
	void ThreadFunc();

public:
	WorkerThread();

	void RegisterWaitHandle(HANDLE handle, HandleFn callback);

	void RegisterTickCallback(std::function<void()> callback);

	void UnregisterWaitHandle(HANDLE handle);
};
}

DECLARE_INSTANCE_TYPE(terminal::WorkerThread);