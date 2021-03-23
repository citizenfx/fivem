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

#include <SDKFileWatcher.h>

namespace fxdk::ioUtils
{
#pragma region EventQueue
	void EventQueue::clear() {
		std::lock_guard<std::mutex> lock(mutex);
		queue.clear();
	}

	std::size_t EventQueue::count() {
		std::lock_guard<std::mutex> lock(mutex);
		return queue.size();
	}

	std::unique_ptr<Event> EventQueue::dequeue() {
		std::lock_guard<std::mutex> lock(mutex);
		if (queue.empty()) {
			return nullptr;
		}

		auto& front = queue.front();
		auto retVal = std::move(front);
		queue.pop_front();

		return retVal;
	}

	std::unique_ptr<std::vector<std::unique_ptr<Event>>> EventQueue::dequeueAll() {
		std::lock_guard<std::mutex> lock(mutex);
		if (queue.empty()) {
			return nullptr;
		}

		const auto queueSize = queue.size();
		std::unique_ptr<std::vector<std::unique_ptr<Event>>> events(new std::vector<std::unique_ptr<Event>>);
		for (size_t i = 0; i < queueSize; ++i) {
			auto& front = queue.front();
			events->emplace_back(std::move(front));
			queue.pop_front();
		}

		return events;
	}

	void EventQueue::enqueue(
		const EventType type,
		const std::string& fromDirectory,
		const std::string& fromFile,
		const std::string& toDirectory,
		const std::string& toFile
	) {
		if (mPaused) {
			return;
		}
		std::lock_guard<std::mutex> lock(mutex);
		queue.emplace_back(std::unique_ptr<Event>(new Event(type, fromDirectory, fromFile, toDirectory, toFile)));
	}

	void EventQueue::pause() {
		mPaused = true;
		clear();
	}

	void EventQueue::resume() {
		mPaused = false;
	}

	EventQueue::EventQueue() :
		mPaused(false)
	{}
#pragma endregion

#pragma region Helpers
	static void stripNTPrefix(std::wstring& path)
	{
		if (path.rfind(L"\\\\?\\UNC\\", 0) != std::wstring::npos)
		{
			path.replace(0, 7, L"\\");
		}
		else if (path.rfind(L"\\\\?\\", 0) != std::wstring::npos)
		{
			path.erase(0, 4);
		}
	}

	static std::wstring getWStringFileName(LPWSTR cFileName, DWORD length)
	{
		LPWSTR nullTerminatedFileName = new WCHAR[length + 1]();
		memcpy(nullTerminatedFileName, cFileName, length);
		std::wstring fileName = nullTerminatedFileName;
		delete[] nullTerminatedFileName;
		return fileName;
	}

	static std::string getUTF8FileName(std::wstring path)
	{
		std::wstring::size_type found = path.rfind('\\');
		if (found != std::wstring::npos) {
			path = path.substr(found + 1);
		}

		return ToNarrow(path);
	}

	static bool isNtPath(const std::wstring& path)
	{
		return path.rfind(L"\\\\?\\", 0) == 0 || path.rfind(L"\\??\\", 0) == 0;
	}

	static std::wstring prefixWithNtPath(const std::wstring& path)
	{
		const ULONG widePathLength = GetFullPathNameW(path.c_str(), 0, nullptr, nullptr);
		if (widePathLength == 0) {
			return path;
		}

		std::wstring ntPathString;
		ntPathString.resize(widePathLength - 1);
		if (GetFullPathNameW(path.c_str(), widePathLength, &(ntPathString[0]), nullptr) != widePathLength - 1)
		{
			return path;
		}

		return ntPathString.rfind(L"\\\\", 0) == 0
			? ntPathString.replace(0, 2, L"\\\\?\\UNC\\")
			: ntPathString.replace(0, 0, L"\\\\?\\");
	}

	static std::wstring toWidePath(const std::string& path)
	{
		auto widePath = ToWide(path);
		if (!isNtPath(widePath)) {
			// We convert to an NT Path to support paths > MAX_PATH
			widePath = prefixWithNtPath(widePath);
		}

		return widePath;
	}
#pragma endregion

	FXWatcher::FXWatcher(const std::string& path, const _EventCallback eventCallback, const _ErrorCallback errorCallback)
		: mDirectoryHandle(INVALID_HANDLE_VALUE), mQueue(std::make_shared<EventQueue>()),
		mDebounceMS(0), mRunning(true), mEventCallback(eventCallback), mErrorCallback(errorCallback)
	{
		auto widePath = ToWide(path);
		const bool ntPath = isNtPath(widePath);

		if (!ntPath) {
			// We convert to an NT Path to support paths > MAX_PATH
			widePath = prefixWithNtPath(widePath);
		}

		mPath = widePath;
		mPathWasNtPrefixed = ntPath;
		mDirectoryHandle = CreateFileW(
			mPath.data(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ
			| FILE_SHARE_WRITE
			| FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS
			| FILE_FLAG_OVERLAPPED,
			NULL
		);

		if (mDirectoryHandle == INVALID_HANDLE_VALUE) {
			return;
		}

		ZeroMemory(&mOverlapped, sizeof(OVERLAPPED));
		mOverlapped.hEvent = this;
		resizeBuffers(1024 * 1024);
		start();
	}

	FXWatcher::~FXWatcher()
	{
		Stop();
	}

	void FXWatcher::Stop()
	{
		mRunning = false;

		// schedule a NOOP APC to force the running loop in `Watcher::run()` to wake
		// up, notice the changed `mRunning` and properly terminate the running loop
		QueueUserAPC([](ULONG_PTR) {}, mUserAPCRunner.native_handle(), (ULONG_PTR)this);

		if (mUserAPCRunner.joinable())
		{
			mUserAPCRunner.join();
		}
		if (mPollThread.joinable())
		{
			mPollThread.join();
		}
	}

	void FXWatcher::start()
	{
		mUserAPCRunner = std::thread([this]
			{
				while (mRunning)
				{
					SleepEx(INFINITE, true);
				}
			});

		if (!mUserAPCRunner.joinable()) {
			mRunning = false;
		}

		QueueUserAPC([](__in ULONG_PTR self) {
			auto watcher = reinterpret_cast<FXWatcher*>(self);
			watcher->pollDirectoryChanges();
			watcher->mHasStartedSemaphore.signal();
		}, mUserAPCRunner.native_handle(), (ULONG_PTR)this);

		if (!mHasStartedSemaphore.waitFor(std::chrono::seconds(10))) {
			setError("Watcher has failed to started");
		}

		mPollThread = std::thread([this]()
		{
			pollForEvents();
		});
	}

	void FXWatcher::pollForEvents() {
		while (mRunning) {
			uint32_t sleepDuration = 50;
			{
				std::lock_guard<std::mutex> lock(mPollMutex);

				if (!getError().empty()) {
					mErrorCallback(getError());
					mRunning = false;
					break;
				}

				if (mQueue->count() != 0) {
					auto events = mQueue->dequeueAll();
					if (events != nullptr) {
						sleepDuration = mDebounceMS;

						mEventCallback(events);
					}
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration));
		}
	}

	bool FXWatcher::pollDirectoryChanges()
	{
		DWORD bytes = 0;

		if (!mRunning)
		{
			return false;
		}

		if (!ReadDirectoryChangesW(
			mDirectoryHandle,
			mWriteBuffer.data(),
			static_cast<DWORD>(mWriteBuffer.size()),
			TRUE,                           // recursive watching
			FILE_NOTIFY_CHANGE_FILE_NAME
			| FILE_NOTIFY_CHANGE_DIR_NAME
			| FILE_NOTIFY_CHANGE_ATTRIBUTES
			| FILE_NOTIFY_CHANGE_SIZE
			| FILE_NOTIFY_CHANGE_LAST_WRITE
			| FILE_NOTIFY_CHANGE_LAST_ACCESS
			| FILE_NOTIFY_CHANGE_CREATION
			| FILE_NOTIFY_CHANGE_SECURITY,
			&bytes,                         // num bytes written
			&mOverlapped,
			[](DWORD errorCode, DWORD numBytes, LPOVERLAPPED overlapped) {
				auto watcher = reinterpret_cast<FXWatcher*>(overlapped->hEvent);
				watcher->eventCallback(errorCode);
			}))
		{
			setError("Service shutdown unexpectedly");
			return false;
		}

		return true;
	}

	void FXWatcher::eventCallback(DWORD errorCode) {
		if (errorCode != ERROR_SUCCESS)
		{
			if (errorCode == ERROR_NOTIFY_ENUM_DIR)
			{
				setError("Buffer filled up and service needs a restart");
			}
			else if (errorCode == ERROR_INVALID_PARAMETER)
			{
				// resize the buffers because we're over the network, 64kb is the max buffer size for networked transmission
				resizeBuffers(64 * 1024);

				if (!pollDirectoryChanges())
				{
					setError("failed resizing buffers for network traffic");
				}
			}
			else
			{
				setError("Service shutdown unexpectedly");
			}
			return;
		}

		std::swap(mWriteBuffer, mReadBuffer);
		pollDirectoryChanges();
		handleEvents();
	}

	void FXWatcher::handleEvents()
	{
		BYTE* base = mReadBuffer.data();
		while (true)
		{
			PFILE_NOTIFY_INFORMATION info = (PFILE_NOTIFY_INFORMATION)base;
			std::wstring fileName = getWStringFileName(info->FileName, info->FileNameLength);

			switch (info->Action) {
			case (FILE_ACTION_RENAMED_OLD_NAME):
				if (info->NextEntryOffset != 0)
				{
					base += info->NextEntryOffset;
					info = (PFILE_NOTIFY_INFORMATION)base;
					if (info->Action == FILE_ACTION_RENAMED_NEW_NAME)
					{
						std::wstring fileNameNew = getWStringFileName(info->FileName, info->FileNameLength);

						mQueue->enqueue(
							RENAMED,
							getUTF8Directory(fileName),
							getUTF8FileName(fileName),
							getUTF8Directory(fileName),
							getUTF8FileName(fileNameNew)
						);
					}
					else
					{
						mQueue->enqueue(DELETED, getUTF8Directory(fileName), getUTF8FileName(fileName));
					}
				}
				else
				{
					mQueue->enqueue(DELETED, getUTF8Directory(fileName), getUTF8FileName(fileName));
				}
				break;
			case FILE_ACTION_ADDED:
			case FILE_ACTION_RENAMED_NEW_NAME: // in the case we just receive a new name and no old name in the buffer
				mQueue->enqueue(CREATED, getUTF8Directory(fileName), getUTF8FileName(fileName));
				break;
			case FILE_ACTION_REMOVED:
				mQueue->enqueue(DELETED, getUTF8Directory(fileName), getUTF8FileName(fileName));
				break;
			case FILE_ACTION_MODIFIED:
			default:
				mQueue->enqueue(MODIFIED, getUTF8Directory(fileName), getUTF8FileName(fileName));
			};

			if (info->NextEntryOffset == 0)
			{
				break;
			}
			base += info->NextEntryOffset;
		}
	}

	void FXWatcher::setError(const std::string& error)
	{
		std::lock_guard<std::mutex> lock(mErrorMutex);
		mError = error;
	}

	const std::string FXWatcher::getError()
	{
		if (mDirectoryHandle == INVALID_HANDLE_VALUE)
		{
			return "Invalid directory handle";
		}
		if (!mRunning)
		{
			return "FXWatcher not running";
		}

		std::lock_guard<std::mutex> lock(mErrorMutex);
		return mError;
	}

	void FXWatcher::resizeBuffers(std::size_t size)
	{
		mReadBuffer.resize(size);
		mWriteBuffer.resize(size);
	}

	std::string FXWatcher::getUTF8Directory(std::wstring path)
	{
		std::wstring::size_type found = path.rfind('\\');
		std::wstringstream utf16DirectoryStream;

		utf16DirectoryStream << mPath;

		if (found != std::wstring::npos) {
			utf16DirectoryStream
				<< "\\"
				<< path.substr(0, found);
		}

		std::wstring uft16DirectoryString = utf16DirectoryStream.str();
		if (!mPathWasNtPrefixed) {
			// If we were the ones that prefixed the path, we should strip it
			// before returning it to the user
			stripNTPrefix(uft16DirectoryString);
		}

		return ToNarrow(uft16DirectoryString);
	}
}
