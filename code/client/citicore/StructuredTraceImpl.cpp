#include <StdInc.h>
#include <StructuredTrace.h>

#ifdef IS_FXSERVER
#include <condition_variable>
#include <thread>
#include <mutex>

#include <tbb/concurrent_queue.h>

#ifdef _WIN32
#include <io.h>
#else
#include <sys/io.h>
#endif

extern "C" bool DLL_EXPORT StructuredTraceEnabled()
{
	static bool hasStructuredFd = ([]
	{
#ifdef _WIN32
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

		const char* str = R"({"test":"test"})";

		int retval = write(3, str, strlen(str));
		return (retval >= 0);
	})();

	return hasStructuredFd;
}

static std::once_flag g_initConsoleFlag;
static std::condition_variable g_consoleCondVar;
static std::mutex g_consoleMutex;

static tbb::concurrent_queue<std::string> g_consolePrintQueue;
static bool g_isPrinting;

extern "C" void DLL_EXPORT StructuredTraceReal(const char* channel, const char* func, const char* file, int line, const nlohmann::json & j)
{
	g_consolePrintQueue.push(nlohmann::json::object({
		{ "channel", channel },
		{ "func", func },
		{ "file", file },
		{ "line", line },
		{ "data", j }
	}).dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace));

	g_consoleCondVar.notify_all();
}

static InitFunction initFunction([]()
{
	std::call_once(g_initConsoleFlag, []()
	{
		std::thread([]()
		{
			SetThreadName(-1, "[Cfx] StConThread");

			while (true)
			{
				{
					std::unique_lock<std::mutex> lock(g_consoleMutex);
					g_consoleCondVar.wait(lock);
				}

				std::string str;

				while (g_consolePrintQueue.try_pop(str))
				{
					g_isPrinting = true;

					write(3, str.c_str(), str.length());

					g_isPrinting = false;
				}
			}
		}).detach();
	});
});
#endif
