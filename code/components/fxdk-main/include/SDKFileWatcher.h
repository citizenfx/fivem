/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Axosoft
 * Copyright (c) 2021 CitizenFX Collective
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <StdInc.h>
#include <SDK.h>

#include <iostream>
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <functional>
#include <sstream>

namespace fxdk::ioUtils
{
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

	class EventQueue {
	public:
		void clear();
		std::size_t count();
		std::unique_ptr<Event> dequeue();
		std::unique_ptr<std::vector<std::unique_ptr<Event>>> dequeueAll();
		void enqueue(
			const EventType type,
			const std::string& fromDirectory,
			const std::string& fromFile,
			const std::string& toDirectory = "",
			const std::string& toFile = ""
		);

		void pause();
		void resume();

		EventQueue();

	private:
		std::deque<std::unique_ptr<Event>> queue;
		std::mutex mutex;
		std::atomic<bool> mPaused;
	};

	class FXWatcher
	{
		typedef std::function<void(const FileEvents& events)> _EventCallback;
		typedef std::function<void(const std::string& error)> _ErrorCallback;

	public:
		FXWatcher(const std::string& path, const _EventCallback eventCallback, const _ErrorCallback errorCallback);
		~FXWatcher();

		void Stop();
		const std::string getError();

	private:
		void start();

		void pollForEvents();
		bool pollDirectoryChanges();
		void eventCallback(DWORD errorCode);
		void handleEvents();

		void setError(const std::string& error);
		void resizeBuffers(std::size_t size);

		std::string getUTF8Directory(std::wstring path);

	private:
		std::string mError;
		std::mutex mErrorMutex;

		std::wstring mPath;
		HANDLE mDirectoryHandle;
		bool mPathWasNtPrefixed;

		std::shared_ptr<EventQueue> mQueue;

		uint32_t mDebounceMS;

		std::atomic<bool> mRunning;
		SingleshotSemaphore mHasStartedSemaphore;

		_ErrorCallback mErrorCallback;
		_EventCallback mEventCallback;

		std::vector<BYTE> mReadBuffer, mWriteBuffer;
		OVERLAPPED mOverlapped;

		std::thread mUserAPCRunner;

		std::thread mPollThread;
		std::mutex mPollMutex;
	};
}
