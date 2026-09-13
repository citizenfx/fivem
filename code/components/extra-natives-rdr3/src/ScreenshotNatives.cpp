/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <ScriptEngine.h>
#include <CrossBuildRuntime.h>
#include <Utils.h>

#include <Resource.h>
#include <ResourceManager.h>
#include <ResourceCallbackComponent.h>
#include <fxScripting.h>

#include <botan/base64.h>
#include <toojpeg.h>

#include <json.hpp>

#include <windows.h>
#include <winhttp.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winhttp.lib")

namespace
{
constexpr int kDefaultQuality = 90;
constexpr int kMaxDimensionLimit = 16384;

// JPEG stores dimensions as 16-bit values, so this is a hard format limit.
constexpr int kMaxImageDimension = 65500;

// Responses are surfaced to scripts as a string, so cap what we are willing to buffer.
constexpr size_t kMaxUploadResponseBytes = 64 * 1024;

constexpr DWORD kHttpResolveTimeoutMs = 10000;
constexpr DWORD kHttpConnectTimeoutMs = 10000;
constexpr DWORD kHttpSendTimeoutMs = 30000;
constexpr DWORD kHttpReceiveTimeoutMs = 30000;

constexpr std::string_view kBoundaryPlaceholder = "%%BOUNDARY%%";

struct BgraBuffer
{
	std::vector<uint8_t> pixels;
	int width = 0;
	int height = 0;
};

struct RgbBuffer
{
	std::vector<uint8_t> pixels;
	int width = 0;
	int height = 0;
};

//
// A request that is waiting for the worker to finish.
//
// Only ever touched on the script thread: fx::FunctionRef duplicates the script reference in
// its constructor and deletes it in its destructor, and both go through natives, so the ref
// must never be created or destroyed on the worker.
//
struct PendingJob
{
	fx::FunctionRef callback;
	std::string resourceName;
};

//
// A finished request handed back from the worker to the script thread.
//
struct JobCompletion
{
	uint64_t id = 0;
	bool isUpload = false;

	std::string dataUrl;

	int status = 0;
	std::string body;
};

//
// Shared state for the capture/upload worker.
//
// This is intentionally leaked (see GetState) so that the worker thread can never observe a
// destroyed object during process teardown, which is otherwise easy to hit since the worker
// outlives static destruction.
//
struct ScreenshotState
{
	// script thread only
	uint64_t nextJobId = 1;
	std::map<uint64_t, PendingJob> jobs;

	std::mutex completionMutex;
	std::vector<JobCompletion> completions;

	std::mutex queueMutex;
	std::condition_variable queueCondition;
	std::deque<std::function<void()>> queue;
	bool workerStarted = false;
};

ScreenshotState* GetState()
{
	static ScreenshotState* state = new ScreenshotState();
	return state;
}

void QueueJob(std::function<void()>&& job)
{
	auto* state = GetState();

	std::unique_lock<std::mutex> lock(state->queueMutex);

	if (!state->workerStarted)
	{
		state->workerStarted = true;

		std::thread([state]()
		{
			SetThreadName(-1, "[Cfx] Screenshot");

			while (true)
			{
				std::function<void()> next;

				{
					std::unique_lock<std::mutex> innerLock(state->queueMutex);
					state->queueCondition.wait(innerLock, [state]
					{
						return !state->queue.empty();
					});

					next = std::move(state->queue.front());
					state->queue.pop_front();
				}

				next();
			}
		}).detach();
	}

	state->queue.push_back(std::move(job));
	state->queueCondition.notify_one();
}

//
// Window targeting
//

HWND GetGameWindow()
{
	HWND wnd = CoreGetGameWindow();

	if (wnd && IsWindow(wnd))
	{
		return wnd;
	}

	wnd = FindWindowW(L"CfxGameWindow", nullptr);

	if (wnd && IsWindow(wnd))
	{
		return wnd;
	}

	return nullptr;
}

//
// Returns whether a GDI capture of the game window would actually read game content.
//
// GDI reads from the composited desktop, so anything drawn on top of the game window ends up
// in the capture. To make sure a resource can never capture unrelated applications, or the
// desktop itself, we refuse to capture unless the game owns the foreground.
//
bool IsGameWindowCapturable(HWND wnd)
{
	if (!wnd || !IsWindow(wnd) || !IsWindowVisible(wnd) || IsIconic(wnd))
	{
		return false;
	}

	HWND foreground = GetForegroundWindow();

	if (!foreground)
	{
		return false;
	}

	return foreground == wnd || GetAncestor(foreground, GA_ROOT) == wnd;
}

//
// Scopes per-monitor-v2 DPI awareness onto the calling thread so that window metrics and the
// blit itself agree on physical pixels, and restores the previous context on exit.
//
class ScopedPerMonitorDpiAwareness
{
private:
	using PFN_SetThreadDpiAwarenessContext = void*(WINAPI*)(void*);

	static PFN_SetThreadDpiAwarenessContext GetSetter()
	{
		static auto setter = reinterpret_cast<PFN_SetThreadDpiAwarenessContext>(
			GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetThreadDpiAwarenessContext"));

		return setter;
	}

public:
	ScopedPerMonitorDpiAwareness()
	{
		if (auto setter = GetSetter())
		{
			// DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
			m_previous = setter(reinterpret_cast<void*>(-4));
			m_restore = m_previous != nullptr;
		}
	}

	~ScopedPerMonitorDpiAwareness()
	{
		if (m_restore)
		{
			if (auto setter = GetSetter())
			{
				setter(m_previous);
			}
		}
	}

	ScopedPerMonitorDpiAwareness(const ScopedPerMonitorDpiAwareness&) = delete;
	ScopedPerMonitorDpiAwareness& operator=(const ScopedPerMonitorDpiAwareness&) = delete;

private:
	void* m_previous = nullptr;
	bool m_restore = false;
};

//
// Captures the game window's client area.
//
// Both the window metrics and the blit run here, on a single thread, under a single DPI
// awareness context - measuring on the script thread and blitting on the worker would
// disagree about coordinates on scaled displays.
//
bool CaptureGameWindow(HWND wnd, BgraBuffer& out)
{
	ScopedPerMonitorDpiAwareness dpiAwareness;

	if (!IsGameWindowCapturable(wnd))
	{
		return false;
	}

	RECT clientRect = {};

	if (!GetClientRect(wnd, &clientRect))
	{
		return false;
	}

	POINT origin = { 0, 0 };

	if (!ClientToScreen(wnd, &origin))
	{
		return false;
	}

	const int width = clientRect.right - clientRect.left;
	const int height = clientRect.bottom - clientRect.top;

	if (width <= 0 || height <= 0 || width > kMaxImageDimension || height > kMaxImageDimension)
	{
		return false;
	}

	HDC screenDc = GetDC(nullptr);

	if (!screenDc)
	{
		return false;
	}

	bool success = false;

	if (HDC memoryDc = CreateCompatibleDC(screenDc))
	{
		if (HBITMAP bitmap = CreateCompatibleBitmap(screenDc, width, height))
		{
			HGDIOBJ previousBitmap = SelectObject(memoryDc, bitmap);

			// plain SRCCOPY: both CAPTUREBLT and PrintWindow force a DWM redraw, which stalls
			// the compositor and shows up as a hitch in-game
			if (BitBlt(memoryDc, 0, 0, width, height, screenDc, origin.x, origin.y, SRCCOPY))
			{
				BITMAPINFO info = {};
				info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				info.bmiHeader.biWidth = width;
				info.bmiHeader.biHeight = -height; // negative for a top-down DIB
				info.bmiHeader.biPlanes = 1;
				info.bmiHeader.biBitCount = 32;
				info.bmiHeader.biCompression = BI_RGB;

				out.pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);

				if (GetDIBits(memoryDc, bitmap, 0, height, out.pixels.data(), &info, DIB_RGB_COLORS) > 0)
				{
					out.width = width;
					out.height = height;
					success = true;
				}
				else
				{
					out.pixels.clear();
				}
			}

			SelectObject(memoryDc, previousBitmap);
			DeleteObject(bitmap);
		}

		DeleteDC(memoryDc);
	}

	ReleaseDC(nullptr, screenDc);

	return success;
}

//
// Converts BGRA to RGB, optionally downscaling so that neither side exceeds maxDimension.
// A maxDimension of 0 keeps the native resolution. Downscaling is bilinear to avoid the
// aliasing a nearest-neighbour pick would produce.
//
RgbBuffer DownscaleAndConvertToRgb(const BgraBuffer& src, int maxDimension)
{
	RgbBuffer dst;

	if (src.pixels.empty() || src.width <= 0 || src.height <= 0)
	{
		return dst;
	}

	float scale = 1.0f;

	if (maxDimension > 0 && (src.width > maxDimension || src.height > maxDimension))
	{
		scale = static_cast<float>(maxDimension) / static_cast<float>((std::max)(src.width, src.height));
	}

	const bool isNative = scale >= 0.999f;
	const int newWidth = isNative ? src.width : (std::max)(1, static_cast<int>(src.width * scale));
	const int newHeight = isNative ? src.height : (std::max)(1, static_cast<int>(src.height * scale));

	dst.width = newWidth;
	dst.height = newHeight;
	dst.pixels.resize(static_cast<size_t>(newWidth) * static_cast<size_t>(newHeight) * 3);

	const size_t srcStride = static_cast<size_t>(src.width) * 4;
	const size_t dstStride = static_cast<size_t>(newWidth) * 3;

	if (isNative)
	{
		for (int y = 0; y < src.height; ++y)
		{
			const uint8_t* srcRow = &src.pixels[static_cast<size_t>(y) * srcStride];
			uint8_t* dstRow = &dst.pixels[static_cast<size_t>(y) * dstStride];

			for (int x = 0; x < src.width; ++x)
			{
				const size_t s = static_cast<size_t>(x) * 4;
				const size_t d = static_cast<size_t>(x) * 3;

				dstRow[d] = srcRow[s + 2];     // R
				dstRow[d + 1] = srcRow[s + 1]; // G
				dstRow[d + 2] = srcRow[s];     // B
			}
		}

		return dst;
	}

	for (int y = 0; y < newHeight; ++y)
	{
		const float srcY = (y + 0.5f) / scale - 0.5f;
		const int y0 = std::clamp(static_cast<int>(std::floor(srcY)), 0, src.height - 1);
		const int y1 = std::clamp(y0 + 1, 0, src.height - 1);
		const float fy = std::clamp(srcY - y0, 0.0f, 1.0f);

		uint8_t* dstRow = &dst.pixels[static_cast<size_t>(y) * dstStride];

		for (int x = 0; x < newWidth; ++x)
		{
			const float srcX = (x + 0.5f) / scale - 0.5f;
			const int x0 = std::clamp(static_cast<int>(std::floor(srcX)), 0, src.width - 1);
			const int x1 = std::clamp(x0 + 1, 0, src.width - 1);
			const float fx = std::clamp(srcX - x0, 0.0f, 1.0f);

			const size_t topLeft = static_cast<size_t>(y0) * srcStride + static_cast<size_t>(x0) * 4;
			const size_t topRight = static_cast<size_t>(y0) * srcStride + static_cast<size_t>(x1) * 4;
			const size_t bottomLeft = static_cast<size_t>(y1) * srcStride + static_cast<size_t>(x0) * 4;
			const size_t bottomRight = static_cast<size_t>(y1) * srcStride + static_cast<size_t>(x1) * 4;

			const size_t d = static_cast<size_t>(x) * 3;

			for (int channel = 0; channel < 3; ++channel)
			{
				const size_t offset = 2 - static_cast<size_t>(channel); // R: +2, G: +1, B: +0

				const float top = src.pixels[topLeft + offset] * (1.0f - fx) + src.pixels[topRight + offset] * fx;
				const float bottom = src.pixels[bottomLeft + offset] * (1.0f - fx) + src.pixels[bottomRight + offset] * fx;

				dstRow[d + channel] = static_cast<uint8_t>(std::clamp(top * (1.0f - fy) + bottom * fy, 0.0f, 255.0f));
			}
		}
	}

	return dst;
}

//
// TooJpeg takes a plain function pointer, so the destination buffer is passed through a
// thread-local pointer rather than a shared global.
//
thread_local std::vector<uint8_t>* tl_jpegSink = nullptr;

void WriteJpegByte(unsigned char byte)
{
	if (tl_jpegSink)
	{
		tl_jpegSink->push_back(byte);
	}
}

bool EncodeToJpeg(const RgbBuffer& buffer, int quality, std::vector<uint8_t>& out)
{
	out.clear();

	if (buffer.pixels.empty() || buffer.width <= 0 || buffer.height <= 0)
	{
		return false;
	}

	if (buffer.width > kMaxImageDimension || buffer.height > kMaxImageDimension)
	{
		return false;
	}

	out.reserve((static_cast<size_t>(buffer.width) * static_cast<size_t>(buffer.height)) / 4);

	tl_jpegSink = &out;

	const bool success = TooJpeg::writeJpeg(WriteJpegByte, buffer.pixels.data(),
		static_cast<unsigned short>(buffer.width),
		static_cast<unsigned short>(buffer.height),
		true /* isRGB */,
		static_cast<unsigned char>(std::clamp(quality, 1, 100)),
		false /* downsample: keep 4:4:4 chroma */);

	tl_jpegSink = nullptr;

	if (!success)
	{
		out.clear();
	}

	return success;
}

bool CaptureAndEncode(HWND wnd, int quality, int maxDimension, std::vector<uint8_t>& out)
{
	BgraBuffer raw;

	if (!CaptureGameWindow(wnd, raw))
	{
		return false;
	}

	return EncodeToJpeg(DownscaleAndConvertToRgb(raw, maxDimension), quality, out);
}

std::string EncodeDataUrl(const std::vector<uint8_t>& jpeg)
{
	return "data:image/jpeg;base64," + Botan::base64_encode(jpeg.data(), jpeg.size());
}

int NormalizeQuality(int quality)
{
	return (quality <= 0) ? kDefaultQuality : std::clamp(quality, 1, 100);
}

int NormalizeMaxDimension(int maxDimension)
{
	if (maxDimension <= 0)
	{
		return 0;
	}

	return std::clamp(maxDimension, 16, kMaxDimensionLimit);
}

//
// Local file output
//

bool EnsureDirectoryExists(const std::wstring& path)
{
	if (path.empty() || path.back() == L':')
	{
		return true;
	}

	if (CreateDirectoryW(path.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return true;
	}

	const auto separator = path.find_last_of(L"\\/");

	if (separator == std::wstring::npos)
	{
		return false;
	}

	if (!EnsureDirectoryExists(path.substr(0, separator)))
	{
		return false;
	}

	return CreateDirectoryW(path.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
}

//
// Resolves a script-supplied file name to a path inside the client's screenshot directory.
//
// Anything that could escape that directory - drive letters, UNC prefixes, rooted paths and
// '..' segments - is rejected, so a resource can not write to arbitrary locations on disk.
//
bool ResolveScreenshotPath(std::string_view fileName, std::wstring& out)
{
	if (fileName.empty() || fileName.size() > 200)
	{
		return false;
	}

	if (fileName.find(':') != std::string_view::npos)
	{
		return false;
	}

	if (fileName.front() == '/' || fileName.front() == '\\')
	{
		return false;
	}

	std::string normalized;
	normalized.reserve(fileName.size());

	for (char character : fileName)
	{
		if (static_cast<unsigned char>(character) < 0x20)
		{
			return false;
		}

		normalized.push_back(character == '/' ? '\\' : character);
	}

	for (size_t segmentStart = 0; segmentStart <= normalized.size();)
	{
		const size_t separator = normalized.find('\\', segmentStart);
		const size_t segmentEnd = (separator == std::string::npos) ? normalized.size() : separator;
		const std::string_view segment = std::string_view(normalized).substr(segmentStart, segmentEnd - segmentStart);

		if (segment.empty() || segment == "." || segment == "..")
		{
			return false;
		}

		if (separator == std::string::npos)
		{
			break;
		}

		segmentStart = separator + 1;
	}

	const auto extensionAt = normalized.find_last_of('.');

	if (extensionAt == std::string::npos)
	{
		return false;
	}

	std::string extension = normalized.substr(extensionAt);
	std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c)
	{
		return static_cast<char>(std::tolower(c));
	});

	if (extension != ".jpg" && extension != ".jpeg")
	{
		return false;
	}

	const std::wstring base = MakeRelativeCitPath(L"data\\screenshots\\");
	out = base + ToWide(normalized);

	return true;
}

bool WriteFileBytes(const std::wstring& path, const std::vector<uint8_t>& bytes)
{
	const auto separator = path.find_last_of(L"\\/");

	if (separator != std::wstring::npos && !EnsureDirectoryExists(path.substr(0, separator)))
	{
		return false;
	}

	FILE* file = _wfopen(path.c_str(), L"wb");

	if (!file)
	{
		return false;
	}

	const size_t written = fwrite(bytes.data(), 1, bytes.size(), file);
	fclose(file);

	if (written != bytes.size())
	{
		DeleteFileW(path.c_str());
		return false;
	}

	return true;
}

//
// Upload
//

struct WinHttpHandle
{
	HINTERNET handle = nullptr;

	WinHttpHandle() = default;

	explicit WinHttpHandle(HINTERNET value)
		: handle(value)
	{
	}

	~WinHttpHandle()
	{
		if (handle)
		{
			WinHttpCloseHandle(handle);
		}
	}

	WinHttpHandle(const WinHttpHandle&) = delete;
	WinHttpHandle& operator=(const WinHttpHandle&) = delete;

	operator HINTERNET() const
	{
		return handle;
	}

	explicit operator bool() const
	{
		return handle != nullptr;
	}
};

//
// Strips anything that could break out of a multipart header - a script-supplied field name
// or file name containing CR/LF or a quote would otherwise let it rewrite the request body.
//
std::string SanitizeFormToken(std::string_view value, size_t maxLength)
{
	std::string out;
	out.reserve((std::min)(value.size(), maxLength));

	for (char character : value)
	{
		const unsigned char raw = static_cast<unsigned char>(character);

		if (raw < 0x20 || raw == 0x7f || character == '"' || character == '\\')
		{
			continue;
		}

		out.push_back(character);

		if (out.size() >= maxLength)
		{
			break;
		}
	}

	return out;
}

void UploadJpeg(const std::vector<uint8_t>& jpeg, const std::string& url, const std::string& field,
	const std::string& fileName, const std::string& extraParts, const std::string& boundary,
	JobCompletion& out)
{
	auto fail = [&out](std::string message)
	{
		out.status = 0;
		out.body = std::move(message);
	};

	const std::wstring wideUrl = ToWide(url);

	URL_COMPONENTS components = {};
	components.dwStructSize = sizeof(components);
	components.dwSchemeLength = static_cast<DWORD>(-1);
	components.dwHostNameLength = static_cast<DWORD>(-1);
	components.dwUrlPathLength = static_cast<DWORD>(-1);
	components.dwExtraInfoLength = static_cast<DWORD>(-1);

	if (!WinHttpCrackUrl(wideUrl.c_str(), static_cast<DWORD>(wideUrl.length()), 0, &components))
	{
		fail("invalid URL");
		return;
	}

	if (components.nScheme != INTERNET_SCHEME_HTTP && components.nScheme != INTERNET_SCHEME_HTTPS)
	{
		fail("only http and https URLs are supported");
		return;
	}

	const std::wstring host(components.lpszHostName, components.dwHostNameLength);
	const std::wstring path(components.lpszUrlPath, components.dwUrlPathLength + components.dwExtraInfoLength);

	WinHttpHandle session{ WinHttpOpen(L"CitizenFX/1.0 (RedM)",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0) };

	if (!session)
	{
		fail("WinHttpOpen failed: " + std::to_string(GetLastError()));
		return;
	}

	WinHttpSetTimeouts(session, kHttpResolveTimeoutMs, kHttpConnectTimeoutMs, kHttpSendTimeoutMs, kHttpReceiveTimeoutMs);

	WinHttpHandle connection{ WinHttpConnect(session, host.c_str(), components.nPort, 0) };

	if (!connection)
	{
		fail("WinHttpConnect failed: " + std::to_string(GetLastError()));
		return;
	}

	const DWORD flags = (components.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;

	WinHttpHandle request{ WinHttpOpenRequest(connection, L"POST", path.c_str(), nullptr,
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags) };

	if (!request)
	{
		fail("WinHttpOpenRequest failed: " + std::to_string(GetLastError()));
		return;
	}

	// non-file fields go first so that endpoints which stream the body see their metadata
	// before the attachment
	std::string head = extraParts;
	head += "--" + boundary + "\r\n";
	head += "Content-Disposition: form-data; name=\"" + field + "\"; filename=\"" + fileName + "\"\r\n";
	head += "Content-Type: image/jpeg\r\n\r\n";

	const std::string tail = "\r\n--" + boundary + "--\r\n";

	std::vector<uint8_t> body;
	body.reserve(head.size() + jpeg.size() + tail.size());
	body.insert(body.end(), head.begin(), head.end());
	body.insert(body.end(), jpeg.begin(), jpeg.end());
	body.insert(body.end(), tail.begin(), tail.end());

	const std::wstring headers = L"Content-Type: multipart/form-data; boundary=" + ToWide(boundary);

	if (!WinHttpSendRequest(request, headers.c_str(), static_cast<DWORD>(headers.length()),
			body.data(), static_cast<DWORD>(body.size()), static_cast<DWORD>(body.size()), 0))
	{
		fail("WinHttpSendRequest failed: " + std::to_string(GetLastError()));
		return;
	}

	if (!WinHttpReceiveResponse(request, nullptr))
	{
		fail("WinHttpReceiveResponse failed: " + std::to_string(GetLastError()));
		return;
	}

	DWORD statusCode = 0;
	DWORD statusSize = sizeof(statusCode);

	WinHttpQueryHeaders(request,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX,
		&statusCode, &statusSize, WINHTTP_NO_HEADER_INDEX);

	std::string response;
	DWORD available = 0;

	while (response.size() < kMaxUploadResponseBytes && WinHttpQueryDataAvailable(request, &available) && available > 0)
	{
		const DWORD toRead = static_cast<DWORD>((std::min)(static_cast<size_t>(available),
			kMaxUploadResponseBytes - response.size()));

		std::vector<char> chunk(toRead);
		DWORD read = 0;

		if (!WinHttpReadData(request, chunk.data(), toRead, &read) || read == 0)
		{
			break;
		}

		response.append(chunk.data(), read);
	}

	out.status = static_cast<int>(statusCode);
	out.body = std::move(response);
}

//
// Builds the non-file multipart sections from a JSON object supplied by the caller, against a
// placeholder boundary that the worker substitutes once it picks a real one. Returns false for
// malformed JSON so that the calling native can report it synchronously.
//
bool BuildExtraParts(std::string_view extraFieldsJson, std::string& out)
{
	out.clear();

	if (extraFieldsJson.empty() || extraFieldsJson == "null" || extraFieldsJson == "{}")
	{
		return true;
	}

	nlohmann::json parsed;

	try
	{
		parsed = nlohmann::json::parse(extraFieldsJson.begin(), extraFieldsJson.end());
	}
	catch (const std::exception&)
	{
		return false;
	}

	if (!parsed.is_object())
	{
		return false;
	}

	for (auto it = parsed.begin(); it != parsed.end(); ++it)
	{
		const std::string key = SanitizeFormToken(it.key(), 128);

		if (key.empty())
		{
			continue;
		}

		const std::string value = it.value().is_string() ? it.value().get<std::string>() : it.value().dump();

		out += "--";
		out += kBoundaryPlaceholder;
		out += "\r\n";
		out += "Content-Disposition: form-data; name=\"" + key + "\"\r\n\r\n";
		out += value + "\r\n";
	}

	return true;
}

std::string SubstituteBoundary(std::string parts, const std::string& boundary)
{
	for (size_t at = parts.find(kBoundaryPlaceholder); at != std::string::npos;
		 at = parts.find(kBoundaryPlaceholder, at))
	{
		parts.replace(at, kBoundaryPlaceholder.size(), boundary);
		at += boundary.size();
	}

	return parts;
}

const char* GetOptionalStringArgument(fx::ScriptContext& context, int index)
{
	if (context.GetArgumentCount() <= index)
	{
		return nullptr;
	}

	return context.GetArgument<const char*>(index);
}

//
// Job bookkeeping
//

void PostCompletion(JobCompletion&& completion)
{
	auto* state = GetState();

	std::lock_guard<std::mutex> lock(state->completionMutex);
	state->completions.push_back(std::move(completion));
}

//
// Runs on the script thread, off the resource manager tick, and is the only place a callback
// is invoked or a job's function reference is released.
//
void DrainCompletions(fx::ResourceManager* manager)
{
	auto* state = GetState();

	std::vector<JobCompletion> completions;

	{
		std::lock_guard<std::mutex> lock(state->completionMutex);

		if (state->completions.empty())
		{
			return;
		}

		completions.swap(state->completions);
	}

	for (auto& completion : completions)
	{
		auto job = state->jobs.find(completion.id);

		if (job == state->jobs.end())
		{
			continue;
		}

		// the owning resource may have stopped while the worker was busy - its reference is
		// gone with it, so there is nothing left to call
		auto resource = manager->GetResource(job->second.resourceName, false);

		if (resource.GetRef() && resource->GetState() == fx::ResourceState::Started)
		{
			if (completion.isUpload)
			{
				manager->CallReference<void>(job->second.callback.GetRef(), completion.status, completion.body);
			}
			else
			{
				manager->CallReference<void>(job->second.callback.GetRef(), completion.dataUrl);
			}
		}

		state->jobs.erase(job);
	}
}

//
// Registers a job for the calling resource and returns its id, or 0 when the native was not
// called from a resource or no callback was passed.
//
uint64_t BeginJob(fx::ScriptContext& context, int callbackArgument)
{
	fx::OMPtr<IScriptRuntime> runtime;

	if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		return 0;
	}

	auto* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

	if (!resource)
	{
		return 0;
	}

	const char* reference = GetOptionalStringArgument(context, callbackArgument);

	if (!reference || !*reference)
	{
		return 0;
	}

	auto* state = GetState();
	const uint64_t id = state->nextJobId++;

	state->jobs.emplace(id, PendingJob{ fx::FunctionRef{ reference }, resource->GetName() });

	return id;
}
}

static InitFunction initFunction([]()
{
	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		manager->OnTick.Connect([manager]()
		{
			DrainCompletions(manager);
		});
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_SCREENSHOT_AVAILABLE", [](fx::ScriptContext& context)
	{
		context.SetResult<bool>(IsGameWindowCapturable(GetGameWindow()));
	});

	fx::ScriptEngine::RegisterNativeHandler("REQUEST_SCREENSHOT", [](fx::ScriptContext& context)
	{
		const int quality = NormalizeQuality(context.GetArgumentCount() > 0 ? context.GetArgument<int>(0) : 0);
		const int maxDimension = NormalizeMaxDimension(context.GetArgumentCount() > 1 ? context.GetArgument<int>(1) : 0);

		const uint64_t id = BeginJob(context, 2);

		if (!id)
		{
			return;
		}

		HWND wnd = GetGameWindow();

		QueueJob([id, wnd, quality, maxDimension]()
		{
			JobCompletion completion;
			completion.id = id;

			std::vector<uint8_t> jpeg;

			if (CaptureAndEncode(wnd, quality, maxDimension, jpeg))
			{
				completion.dataUrl = EncodeDataUrl(jpeg);
			}

			PostCompletion(std::move(completion));
		});
	});

	fx::ScriptEngine::RegisterNativeHandler("REQUEST_SCREENSHOT_UPLOAD", [](fx::ScriptContext& context)
	{
		const char* urlArgument = GetOptionalStringArgument(context, 0);

		if (!urlArgument || !*urlArgument)
		{
			return;
		}

		const std::string url = urlArgument;

		const char* fieldArgument = GetOptionalStringArgument(context, 1);
		std::string field = SanitizeFormToken(fieldArgument ? fieldArgument : "", 128);

		if (field.empty())
		{
			field = "file";
		}

		const char* fileNameArgument = GetOptionalStringArgument(context, 2);
		std::string fileName = SanitizeFormToken(fileNameArgument ? fileNameArgument : "", 128);

		if (fileName.empty())
		{
			fileName = "screenshot.jpg";
		}

		const int quality = NormalizeQuality(context.GetArgumentCount() > 3 ? context.GetArgument<int>(3) : 0);
		const int maxDimension = NormalizeMaxDimension(context.GetArgumentCount() > 4 ? context.GetArgument<int>(4) : 0);

		const char* extraFieldsArgument = GetOptionalStringArgument(context, 5);

		std::string extraParts;

		if (!BuildExtraParts(extraFieldsArgument ? extraFieldsArgument : "", extraParts))
		{
			return;
		}

		const uint64_t id = BeginJob(context, 6);

		if (!id)
		{
			return;
		}

		HWND wnd = GetGameWindow();

		QueueJob([id, wnd, quality, maxDimension, url, field, fileName, extraParts]()
		{
			JobCompletion completion;
			completion.id = id;
			completion.isUpload = true;

			std::vector<uint8_t> jpeg;

			if (!CaptureAndEncode(wnd, quality, maxDimension, jpeg))
			{
				completion.status = 0;
				completion.body = "screen capture failed";
			}
			else
			{
				const std::string boundary = "----CitizenFXScreenshot" + std::to_string(GetTickCount64());

				UploadJpeg(jpeg, url, field, fileName, SubstituteBoundary(extraParts, boundary), boundary, completion);
			}

			PostCompletion(std::move(completion));
		});
	});

	fx::ScriptEngine::RegisterNativeHandler("SAVE_SCREENSHOT_TO_FILE", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 1)
		{
			context.SetResult<bool>(false);
			return;
		}

		const char* fileName = context.GetArgument<const char*>(0);
		const int quality = NormalizeQuality(context.GetArgumentCount() > 1 ? context.GetArgument<int>(1) : 0);

		std::wstring path;

		if (!fileName || !ResolveScreenshotPath(fileName, path))
		{
			context.SetResult<bool>(false);
			return;
		}

		HWND wnd = GetGameWindow();
		std::vector<uint8_t> jpeg;

		if (!wnd || !CaptureAndEncode(wnd, quality, 0, jpeg))
		{
			context.SetResult<bool>(false);
			return;
		}

		context.SetResult<bool>(WriteFileBytes(path, jpeg));
	});
});
