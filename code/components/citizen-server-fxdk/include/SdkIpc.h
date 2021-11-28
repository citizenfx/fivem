/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <condition_variable>

#include <ResourceMonitor.h>

#include <UvLoopManager.h>
#include <ResourceManager.h>
#include <ServerInstanceBase.h>

namespace fxdk
{
	// from nsfw node module, MIT licensed
	// https://github.com/Axosoft/nsfw/blob/e8a6e953b5ec4ed09b1e73cea944fcfc6149ea3e/includes/SingleshotSemaphore.h
	class SingleshotSemaphore {
	public:
		SingleshotSemaphore() : mState(false) {}

		/**
		 * Blocks the calling thread until the semaphore is signaled asynchronously.
		 * If `signal()` has been called on the semaphore already, this won't block.
		 */
		void wait() {
			std::unique_lock<std::mutex> lk(mMutex);

			while (!mState) {
				mCond.wait(lk);
			}
		}

		/**
		 * Blocks the calling thread for a given time period and continues either
		 * when `signal()` was called asynchronously or when the time is up. The
		 * return condition is indicated by the returned boolean.
		 *
		 * \return true if the semaphore was signal()ed; false on timeout reached
		 */
		bool waitFor(std::chrono::milliseconds ms) {
			std::unique_lock<std::mutex> lk(mMutex);

			if (mState) {
				return true;
			}

			mCond.wait_for(lk, ms);
			return mState;
		}

		/**
		 * Unblocks all waiting threads of the semaphore. Note that threads reaching
		 * the `wait()` on this semaphore after `signal()` has been called won't
		 * block but continue immediately.
		 */
		void signal() {
			std::unique_lock<std::mutex> lk(mMutex);
			mState = true;
			mCond.notify_all();
		}

	private:
		std::mutex mMutex;
		std::condition_variable mCond;
		bool mState;
	};

	class SdkIpc : public fwRefCountable
	{
	public:
		SdkIpc(fwRefContainer<fx::ServerInstanceBase> instance, const std::string& pipeAppendix);
		~SdkIpc();

		void WaitForInitialized();
		void NotifyInitialized();
		void NotifyStarted();
		void NotifyReady();

	private:
		std::string m_pipeName;

	public:
		inline fwRefContainer<fx::ResourceManager> GetResourceManager() {
			return m_instance->GetComponent<fx::ResourceManager>();
		}

		inline fwRefContainer<console::Context> GetConsole()
		{
			return m_instance->GetComponent<console::Context>();
		}

	public:
		bool StartResource(const std::string& resourceName, std::string& error);
		bool StopResource(const std::string& resourceName, std::string& error);
		bool RestartResource(const std::string& resourceName, std::string& error);

	private:
		std::shared_ptr<uvw::TimerHandle> m_timer;

		fwRefContainer<fx::ServerInstanceBase> m_instance;

		SingleshotSemaphore m_inited;

		std::unique_ptr<fx::ResourceMonitor> m_resourceMonitor;

		void Quit(const std::string& error);
	};
}
