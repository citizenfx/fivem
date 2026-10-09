/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIClient.h"
#include "CefOverlay.h"

#include <shlobj.h>
#include <shobjidl.h>
#include <atlbase.h>
#include <knownfolders.h>

#include <algorithm>
#include <atomic>
#include <deque>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <json.hpp>

#include <CfxSubProcess.h>
#include <FileDialogIPC.h>

#include "include/wrapper/cef_closure_task.h"
#include "include/base/cef_callback_helpers.h"

#include "memdbgon.h"

extern nui::GameInterface* g_nuiGi;

// ext -> mime, defined in NUISchemeHandler.cpp
extern const std::map<std::string_view, std::string_view, std::less<>> g_mimeTypeMap;

namespace
{
#define FDTRACE(seq, fmtStr, ...) trace("[filedialog][s%d] " fmtStr "\n", (seq), ##__VA_ARGS__)

// If the helper never creates a dialog window within this budget, Show() is wedged inside shell
// init and never coming back; kill the helper and resolve as cancel. A window that exists means the
// user is browsing - left alone however long that takes.
constexpr int kHelperNoWindowMs = 6000;

struct ScopedHandle
{
	explicit ScopedHandle(HANDLE handle = nullptr)
		: handle(handle)
	{
	}

	~ScopedHandle()
	{
		if (handle)
		{
			CloseHandle(handle);
		}
	}

	ScopedHandle(const ScopedHandle&) = delete;
	ScopedHandle& operator=(const ScopedHandle&) = delete;

	explicit operator bool() const
	{
		return handle != nullptr;
	}

	HANDLE handle;
};

// Tells the originating NUI frame how its file dialog resolved.
//
// This CEF is Chromium 103; the <input type="file"> 'cancel' event only exists from
// Chrome 113, so without this a page cannot distinguish "picker cancelled" from "picker
// still open". PostFrameMessage marshals onto its own worker thread and tolerates
// not-yet-ready browsers, so this is safe to call from any thread. The page receives it
// as a regular window 'message' event.
//
// NOTE: this event and the input's own 'change' event ride different queues - pages must
// tolerate them arriving in either order.
void EmitFileDialogResult(const std::string& frameName, int seq, int mode, bool cancelled, const std::vector<CefString>& paths)
{
	if (frameName.empty())
	{
		return;
	}

	auto files = nlohmann::json::array();

	for (const auto& path : paths)
	{
		files.push_back(std::filesystem::path(path.ToWString()).u8string());
	}

	nui::PostFrameMessage(frameName, nlohmann::json::object({
		{ "type", "nuiFileDialogResult" },
		{ "seq", seq },
		{ "mode", mode },
		{ "cancelled", cancelled },
		{ "files", std::move(files) },
	}).dump());
}

// Resolves a CefFileDialogCallback exactly once, cancelling on destruction if nothing else did.
//
// The renderer keeps one file chooser per frame. While a request is outstanding Blink silently
// drops every new <input type="file"> activation - no dialog, no callback, not even a cancel - so
// an unresolved callback wedges the picker for the rest of the session. Making the resolve
// unconditional is the entire point of this class.
//
// (Note: this is a renderer-side limit. CEF's own "only one pending dialog" wording in
// cef_browser.h applies to the CefBrowserHost::RunFileDialog API, not to <input type="file">.)
class ScopedFileDialogCallback
{
public:
	ScopedFileDialogCallback(CefRefPtr<CefFileDialogCallback> callback, std::string frameName, int seq, int mode)
		: m_callback(std::move(callback)), m_frameName(std::move(frameName)), m_seq(seq), m_mode(mode)
	{
	}

	~ScopedFileDialogCallback()
	{
		Cancel();
	}

	ScopedFileDialogCallback(const ScopedFileDialogCallback&) = delete;
	ScopedFileDialogCallback& operator=(const ScopedFileDialogCallback&) = delete;

	void Continue(const std::vector<CefString>& paths)
	{
		auto callback = Take();

		if (!callback)
		{
			return;
		}

		if (paths.empty())
		{
			CefPostTask(TID_UI, base::BindOnce(&CefFileDialogCallback::Cancel, callback));
		}
		else
		{
			CefPostTask(TID_UI, base::BindOnce(&CefFileDialogCallback::Continue, callback, paths));
		}

		// exactly-once by construction: emission only happens on the winning Take()
		EmitFileDialogResult(m_frameName, m_seq, m_mode, paths.empty(), paths);
	}

	void Cancel()
	{
		if (auto callback = Take())
		{
			CefPostTask(TID_UI, base::BindOnce(&CefFileDialogCallback::Cancel, callback));

			EmitFileDialogResult(m_frameName, m_seq, m_mode, true, {});
		}
	}

private:
	// Held so the handover is atomic - a request finishing normally and an escape path both racing to
	// resolve must not double-resolve the CEF callback.
	CefRefPtr<CefFileDialogCallback> Take()
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		CefRefPtr<CefFileDialogCallback> callback;
		std::swap(callback, m_callback);

		return callback;
	}

	std::mutex m_mutex;

	CefRefPtr<CefFileDialogCallback> m_callback;

	// for the nuiFileDialogResult page event - resolved eagerly on the CEF UI thread at
	// request time, because the browser may be gone by resolution time
	std::string m_frameName;
	int m_seq;
	int m_mode;
};

std::atomic<int> g_fileDialogSeq{ 0 };

// Guards the request queue and busy flag.
std::mutex g_dialogMutex;
bool g_dialogBusy = false;

// COMDLG_FILTERSPEC holds raw pointers, so the backing strings have to outlive it. All strings are
// filled before any spec is built - otherwise vector reallocation dangles them.
struct FilterSpecStorage
{
	std::vector<std::wstring> strings;
	std::vector<COMDLG_FILTERSPEC> specs;
};

std::vector<std::string> SplitString(std::string_view value, char delimiter)
{
	std::vector<std::string> parts;

	size_t start = 0;

	while (start <= value.size())
	{
		auto end = value.find(delimiter, start);

		if (end == std::string_view::npos)
		{
			end = value.size();
		}

		auto part = value.substr(start, end - start);

		if (!part.empty())
		{
			parts.emplace_back(part);
		}

		start = end + 1;
	}

	return parts;
}

// ".png" or "png" -> "png"
std::string NormalizeExtension(std::string_view extension)
{
	while (!extension.empty() && (extension.front() == '.' || extension.front() == '*'))
	{
		extension.remove_prefix(1);
	}

	std::string normalized{ extension };
	std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);

	return normalized;
}

// g_mimeTypeMap is ext -> mime, so every MIME lookup is a linear reverse scan. It runs once per
// dialog over a few hundred entries, which is not worth a reverse index.
void CollectExtensionsForMimeType(const std::string& mimeType, std::vector<std::string>& extensions)
{
	const bool isWildcard = mimeType.size() > 2 && mimeType.compare(mimeType.size() - 2, 2, "/*") == 0;
	const std::string prefix = isWildcard ? mimeType.substr(0, mimeType.size() - 1) : std::string{};

	for (const auto& [extension, entryMime] : g_mimeTypeMap)
	{
		// the map has junk keys like "*mp3"/"*wav" that aren't real extensions
		if (extension.empty() || extension.front() == '*')
		{
			continue;
		}

		const bool matches = isWildcard
			? (entryMime.size() >= prefix.size() && entryMime.compare(0, prefix.size(), prefix) == 0)
			: (entryMime == mimeType);

		if (matches)
		{
			extensions.emplace_back(extension);
		}
	}
}

// accept_filters entries come in three forms (cef_dialog_handler.h):
//   "Image Types|.png;.gif"  - description + extension list
//   ".png,.jpg"              - extension list
//   "image/*" / "image/png"  - MIME type
void BuildFilterSpecs(const std::vector<CefString>& acceptFilters, FilterSpecStorage& storage)
{
	std::vector<std::pair<size_t, size_t>> pending;

	for (const auto& acceptFilter : acceptFilters)
	{
		auto filter = acceptFilter.ToString();

		if (filter.empty())
		{
			continue;
		}

		std::string description;
		std::vector<std::string> extensions;

		if (auto pipe = filter.find('|'); pipe != std::string::npos)
		{
			description = filter.substr(0, pipe);

			for (const auto& part : SplitString(std::string_view{ filter }.substr(pipe + 1), ';'))
			{
				extensions.emplace_back(NormalizeExtension(part));
			}
		}
		else if (filter.find('/') != std::string::npos)
		{
			CollectExtensionsForMimeType(filter, extensions);
		}
		else
		{
			for (const auto& part : SplitString(filter, ','))
			{
				extensions.emplace_back(NormalizeExtension(part));
			}
		}

		extensions.erase(std::remove_if(extensions.begin(), extensions.end(), [](const std::string& extension)
		{
			return extension.empty();
		}), extensions.end());

		std::sort(extensions.begin(), extensions.end());
		extensions.erase(std::unique(extensions.begin(), extensions.end()), extensions.end());

		if (extensions.empty())
		{
			continue;
		}

		std::string pattern;

		for (const auto& extension : extensions)
		{
			if (!pattern.empty())
			{
				pattern += ";";
			}

			pattern += "*." + extension;
		}

		if (description.empty())
		{
			description = filter + " (" + pattern + ")";
		}

		storage.strings.push_back(ToWide(description));
		storage.strings.push_back(ToWide(pattern));

		pending.emplace_back(storage.strings.size() - 2, storage.strings.size() - 1);
	}

	if (pending.empty())
	{
		return;
	}

	// never trap the user in a filter they can't escape
	storage.strings.push_back(L"All Files");
	storage.strings.push_back(L"*.*");

	pending.emplace_back(storage.strings.size() - 2, storage.strings.size() - 1);

	storage.specs.reserve(pending.size());

	for (const auto& [descriptionIndex, patternIndex] : pending)
	{
		storage.specs.push_back({ storage.strings[descriptionIndex].c_str(), storage.strings[patternIndex].c_str() });
	}
}

// Shows the file dialog OUT OF PROCESS and resolves the CEF callback with the result.
//
// Showing IFileDialog inside the game process deadlocks: shell-extension creation
// (AssocCreateForClasses -> SHExtCoCreateInstanceString) parks shell's own worker-pool STAs in
// non-pumping waits inside this hooked/hand-mapped process, so Show() blocks forever inside shell
// init without ever creating a window, and the stuck shell workers accumulate. A clean copy of the
// main launcher exe (see launcher/GameSelect.cpp, FileDialogIPC.h) shows the dialog instead; if THAT
// hangs before creating a window it is simply killed - the whole point of the out-of-process design.
//
// Runs on the persistent dialog thread (see StartDialogThread): CEF uses
// multi_threaded_message_loop, so OnFileDialog arrives on the CEF UI thread and blocking there would
// stall all of CEF.
void ShowFileDialog(int seq,
	CefDialogHandler::FileDialogMode mode,
	std::wstring title,
	std::wstring defaultPath,
	std::vector<CefString> acceptFilters,
	std::string frameName,
	HWND parentWindow,
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFileDialogCallback> rawCallback)
{
	auto callbackPtr = std::make_shared<ScopedFileDialogCallback>(rawCallback, std::move(frameName), seq, (int)mode);
	auto& callback = *callbackPtr;

	try
	{
		// package the request into an inheritable anonymous mapping + completion event
		SECURITY_ATTRIBUTES inheritable = { sizeof(inheritable), nullptr, TRUE };

		ScopedHandle mapping(CreateFileMappingW(INVALID_HANDLE_VALUE, &inheritable, PAGE_READWRITE,
			0, (DWORD)sizeof(fdipc::SharedBlock), nullptr));

		if (!mapping)
		{
			FDTRACE(seq, "CreateFileMapping failed err=%u", (unsigned)GetLastError());
			return;
		}

		auto block = reinterpret_cast<fdipc::SharedBlock*>(MapViewOfFile(mapping.handle, FILE_MAP_ALL_ACCESS, 0, 0, 0));

		if (!block)
		{
			FDTRACE(seq, "MapViewOfFile failed err=%u", (unsigned)GetLastError());
			return;
		}

		std::unique_ptr<fdipc::SharedBlock, decltype(&UnmapViewOfFile)> blockGuard(block, &UnmapViewOfFile);

		memset(block, 0, sizeof(*block));

		auto& request = block->request;
		request.version = fdipc::kVersion;
		request.mode = (uint32_t)mode;
		wcsncpy_s(request.title, title.c_str(), _TRUNCATE);
		wcsncpy_s(request.defaultPath, defaultPath.c_str(), _TRUNCATE);

		if (mode != FILE_DIALOG_OPEN_FOLDER)
		{
			// filter derivation (MIME map etc.) stays on this side - the helper just passes
			// the finished COMDLG pairs through
			FilterSpecStorage filters;
			BuildFilterSpecs(acceptFilters, filters);

			uint32_t numFilters = 0;

			for (const auto& spec : filters.specs)
			{
				if (numFilters >= fdipc::kMaxFilters)
				{
					break;
				}

				wcsncpy_s(request.filterNames[numFilters], spec.pszName, _TRUNCATE);
				wcsncpy_s(request.filterSpecs[numFilters], spec.pszSpec, _TRUNCATE);
				numFilters++;
			}

			request.numFilters = numFilters;
		}

		ScopedHandle doneEvent(CreateEventW(&inheritable, TRUE, FALSE, nullptr));

		if (!doneEvent)
		{
			FDTRACE(seq, "CreateEvent failed err=%u", (unsigned)GetLastError());
			return;
		}

		auto helperPath = MakeCfxSubProcess(L"FileDialog.exe");

		wchar_t commandLine[1024];
		swprintf_s(commandLine, L"\"%s\" -filedialog:%llu:%llu", helperPath,
			(unsigned long long)(uintptr_t)mapping.handle, (unsigned long long)(uintptr_t)doneEvent.handle);

		FDTRACE(seq, "spawning dialog helper mode=%d", (int)mode);

		STARTUPINFOW startupInfo = { sizeof(startupInfo) };
		PROCESS_INFORMATION processInfo = {};

		if (!CreateProcessW(helperPath, commandLine, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startupInfo, &processInfo))
		{
			FDTRACE(seq, "CreateProcess failed err=%u", (unsigned)GetLastError());
			return;
		}

		ScopedHandle process(processInfo.hProcess);
		ScopedHandle processThread(processInfo.hThread);

		// let the helper raise its dialog over the game window
		AllowSetForegroundWindow(processInfo.dwProcessId);

		// Wait for the result while keeping this STA pumping: a helper that creates no window within
		// the timeout is killed; once a window exists, the user can browse for as long as they like.
		const HANDLE waitHandles[] = { doneEvent.handle, process.handle };

		const uint64_t started = GetTickCount64();
		bool completed = false;
		bool killed = false;

		for (;;)
		{
			const DWORD wait = MsgWaitForMultipleObjects(2, waitHandles, FALSE, 1000, QS_ALLINPUT);

			if (wait == WAIT_OBJECT_0)
			{
				completed = true;
				break;
			}

			if (wait == WAIT_OBJECT_0 + 1 || wait == WAIT_FAILED)
			{
				// helper exited (or the wait broke) without signalling a result
				break;
			}

			if (wait == WAIT_OBJECT_0 + 2)
			{
				// service the apartment: inbound COM calls arrive as window messages
				MSG msg;

				while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
			}

			const uint64_t elapsed = GetTickCount64() - started;

			if (!killed && !block->response.windowCreated && elapsed >= (uint64_t)kHelperNoWindowMs)
			{
				FDTRACE(seq, "helper made no window in %ums - killing it", (unsigned)elapsed);
				TerminateProcess(process.handle, 1);
				killed = true;
				// loop continues; the process handle signals on the next pass
			}
		}

		std::vector<CefString> paths;

		if (completed && block->response.succeeded && block->response.numFiles > 0)
		{
			const uint32_t resultChars = (std::min)(block->response.resultChars, (uint32_t)fdipc::kMaxResultChars);
			const wchar_t* cursor = block->response.results;
			const wchar_t* end = block->response.results + resultChars;

			for (uint32_t i = 0; i < block->response.numFiles && cursor < end; i++)
			{
				const size_t length = wcsnlen(cursor, end - cursor);
				paths.emplace_back(ToNarrow(std::wstring(cursor, length)));
				cursor += length + 1;
			}
		}

		FDTRACE(seq, "helper result: succeeded=%u files=%u%s",
			(unsigned)(completed ? block->response.succeeded : 0), (unsigned)paths.size(),
			completed ? "" : (killed ? " (killed)" : " (no result)"));

		// empty resolves as cancel inside the guard
		callback.Continue(paths);
	}
	catch (const std::exception& e)
	{
		// guard cancels on unwind
		FDTRACE(seq, "C++ EXCEPTION escaped: %s", e.what());
	}
	catch (...)
	{
		// guard cancels on unwind. note /EHsc means SEH (access violations, delay-load failures) is
		// NOT caught here - that would reach breakpad instead
		FDTRACE(seq, "UNKNOWN C++ EXCEPTION escaped");
	}

	// The modal took Win32 focus, but nui::HasFocus() is a script-driven flag that never changed, so
	// the edge-triggered re-assert in CefInput.cpp won't fire and CEF stays internally unfocused -
	// leaving the page unable to take keyboard input. Push focus back explicitly.
	if (parentWindow)
	{
		SetForegroundWindow(parentWindow);
	}

	if (browser)
	{
		// unconditional: HasFocus() is the script-driven flag that never changed across the dialog,
		// so gating on it would skip the re-assert in exactly the cases that need it
		CefPostTask(TID_UI, base::BindOnce([](CefRefPtr<CefBrowser> browser)
		{
			browser->GetHost()->SetFocus(true);
		},
		browser));
	}
}

// One request for the persistent dialog thread.
struct DialogRequest
{
	int seq;
	CefDialogHandler::FileDialogMode mode;
	std::wstring title;
	std::wstring defaultPath;
	std::vector<CefString> acceptFilters;
	std::string frameName;
	HWND parentWindow;
	CefRefPtr<CefBrowser> browser;
	CefRefPtr<CefFileDialogCallback> callback;
};

std::deque<DialogRequest> g_dialogQueue;

// Wakes the dialog thread when a request is queued. A Win32 auto-reset event rather than a
// condition_variable, because the thread waits in a way that still dispatches messages - see
// StartDialogThread.
HANDLE g_dialogWakeEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

// false until the first request starts the dialog thread
std::atomic<bool> g_dialogThreadStarted{ false };

// A single thread serves every file dialog request, for the life of the process.
//
// The dialog itself runs out of process (see ShowFileDialog), so this thread's job is just to
// serialize requests off the CEF UI thread and host the bounded wait on the helper. It stays an STA
// with a pumping wait because the apartment exists for the thread's lifetime (combase!CoUninitialize
// is no-op-hooked process-wide - see citicore/FileMapping.Win32.cpp) and an STA that stops
// dispatching would block anything that ever marshals into it.
void StartDialogThread()
{
	std::thread([]()
	{
		// deliberately never uninitialized - this thread outlives every request
		CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		// a thread's message queue is created lazily on the first message call - force it into
		// existence now, so the apartment is dispatchable before anyone can marshal into it
		MSG primer;
		PeekMessageW(&primer, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

		while (true)
		{
			MsgWaitForMultipleObjects(1, &g_dialogWakeEvent, FALSE, INFINITE, QS_ALLINPUT);

			// service the apartment: inbound COM calls arrive here as window messages
			MSG msg;

			while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}

			// the wake event is auto-reset, so drain rather than handling a single request - two
			// could have landed between waits
			while (true)
			{
				DialogRequest request;

				{
					std::unique_lock<std::mutex> lock(g_dialogMutex);

					if (g_dialogQueue.empty())
					{
						break;
					}

					request = std::move(g_dialogQueue.front());
					g_dialogQueue.pop_front();
				}

				try
				{
					ShowFileDialog(request.seq, request.mode, std::move(request.title), std::move(request.defaultPath),
						std::move(request.acceptFilters), std::move(request.frameName), request.parentWindow,
						request.browser, request.callback);
				}
				catch (...)
				{
					// ShowFileDialog handles its own exceptions, but never let one escape and leave
					// g_dialogBusy stuck - that would lock the picker out for the session
					FDTRACE(request.seq, "EXCEPTION escaped ShowFileDialog");
				}

				{
					std::unique_lock<std::mutex> lock(g_dialogMutex);
					g_dialogBusy = false;
				}
			}
		}
	}).detach();
}

// Returns false if a dialog is already up, in which case the caller must resolve the callback.
bool QueueDialogRequest(DialogRequest&& request)
{
	{
		std::unique_lock<std::mutex> lock(g_dialogMutex);

		// Don't queue behind an open dialog - popping a second picker the instant the first closes
		// is not what someone who clicked minutes ago expects. Cancelling instead frees the
		// renderer's chooser right away, so the next click works normally.
		if (g_dialogBusy)
		{
			return false;
		}

		g_dialogBusy = true;
		g_dialogQueue.push_back(std::move(request));

		if (!g_dialogThreadStarted.exchange(true))
		{
			StartDialogThread();
		}
	}

	SetEvent(g_dialogWakeEvent);

	return true;
}
}

bool NUIClient::OnFileDialog(CefRefPtr<CefBrowser> browser,
	FileDialogMode mode,
	const CefString& title,
	const CefString& default_file_path,
	const std::vector<CefString>& accept_filters,
	CefRefPtr<CefFileDialogCallback> callback)
{
	const int seq = ++g_fileDialogSeq;

	FDTRACE(seq, "request mode=%d filters=%d", (int)mode, (int)accept_filters.size());

	// The browser is windowless with a null parent (NUIWindow.cpp), so parent focus restoration
	// targets the real game window instead.
	HWND parentWindow = (g_nuiGi) ? g_nuiGi->GetHWND() : nullptr;

	// Resolve the frame name for the nuiFileDialogResult event NOW, on the CEF UI thread,
	// while the browser is known-live - never lazily from another thread (OnBeforeClose /
	// ClearWindow can race a late resolution).
	std::string frameName;

	if (auto window = GetWindow())
	{
		frameName = window->GetName();

		if (frameName.find("nui_") == 0)
		{
			frameName = frameName.substr(4);
		}
	}

	DialogRequest request{
		seq,
		mode,
		title.ToWString(),
		default_file_path.ToWString(),
		accept_filters,
		frameName,
		parentWindow,
		browser,
		callback
	};

	try
	{
		if (!QueueDialogRequest(std::move(request)))
		{
			// resolve rather than drop it, or the renderer's chooser stays outstanding and every
			// later <input type="file"> is silently ignored
			FDTRACE(seq, "a dialog is already open - cancelling this request");
			callback->Cancel();

			// this path bypasses ScopedFileDialogCallback, so tell the page directly
			EmitFileDialogResult(frameName, seq, (int)mode, true, {});
		}
	}
	catch (const std::system_error& e)
	{
		FDTRACE(seq, "QUEUE FAILED: %s (code=%d)", e.what(), (int)e.code().value());
		callback->Cancel();

		EmitFileDialogResult(frameName, seq, (int)mode, true, {});
	}

	return true;
}
