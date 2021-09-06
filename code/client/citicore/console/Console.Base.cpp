#include <StdInc.h>
#include "Console.Base.h"

#include "Console.VariableHelpers.h"

#include <stdarg.h>
#include <CL2LaunchMode.h>

#include <tbb/concurrent_queue.h>
#include <boost/algorithm/string/replace.hpp>

namespace console
{
static bool g_allowVt;

static inline bool _isdigit(char c)
{
	return (c >= '0' && c <= '9');
}

static const int g_colors[] = {
	97, // bright white, black
	91, // red
	32, // green
	93, // bright yellow
	94, // blue
	36, // cyan
	35, // magenta
	0,  // reset
	31, // dark red
	34  // dark blue
};

static void Print(const char* str)
{
	if (launch::IsSDK() || launch::IsSDKGuest())
	{
		fprintf(stderr, "%s", str);
	}
	else
	{
		printf("%s", str);
	}
}

static auto g_printf = &Print;
static thread_local std::string g_printChannel;
static bool g_startOfLine = true;
static int lastColor = 7;

template<typename Stream>
inline void WriteColor(Stream& buf, int color)
{
	buf << fmt::sprintf("\x1B[%dm", g_colors[color]);
}

template<typename Stream>
inline void WriteChannel(Stream& buf, const std::string& channelName)
{
	if (g_allowVt)
	{
		auto hash = HashString(channelName.c_str());
		buf << fmt::sprintf("\x1B[38;5;%dm", (hash % (231 - 17)) + 17);
	}

	std::string effectiveChannelName = channelName;

	if (channelName.length() > 20)
	{
		if (channelName.find("citizen-") == 0)
		{
			effectiveChannelName = "c-" + channelName.substr(8);
		}
	}

	buf << fmt::sprintf("[%20s] ", effectiveChannelName.substr(0, std::min(size_t(20), effectiveChannelName.length())));

	if (g_allowVt)
	{
		buf << fmt::sprintf("\x1B[%dm", 0);
	}
}

static void CfxPrintf(const std::string& str)
{
	std::stringstream buf;

	for (size_t i = 0; i < str.size(); i++)
	{
		if (g_startOfLine)
		{
			WriteChannel(buf, g_printChannel);
			WriteColor(buf, lastColor);
			g_startOfLine = false;
		}

		if (str[i] == '^' && _isdigit(str[i + 1]))
		{
			if (g_allowVt)
			{
				lastColor = str[i + 1] - '0';
				WriteColor(buf, lastColor);
			}

			i += 1;
		}
		else
		{
			buf << fmt::sprintf("%c", str[i]);
			if (str[i] == '\n')
			{
				g_startOfLine = true;
			}
		}
	}

	g_printf(buf.str().c_str());
}

static std::once_flag g_initConsoleFlag;
static std::condition_variable g_consoleCondVar;
static std::mutex g_consoleMutex;

static tbb::concurrent_queue<std::tuple<std::string, std::string>> g_consolePrintQueue;
static bool g_isPrinting;

static void PrintfTraceListener(ConsoleChannel channel, const char* out)
{
#ifdef _WIN32
	static std::once_flag g_vtOnceFlag;

	std::call_once(g_vtOnceFlag, []()
	{
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

			DWORD consoleMode;
			if (GetConsoleMode(hConsole, &consoleMode))
			{
				consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

				if (SetConsoleMode(hConsole, consoleMode))
				{
					g_allowVt = true;
				}

				if (IsWindows10OrGreater())
				{
					SetConsoleCP(65001);
					SetConsoleOutputCP(65001);
				}
			}
			else
			{
				g_allowVt = true;
			}
		}

		{
			HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);

			DWORD consoleMode;
			if (GetConsoleMode(hConsole, &consoleMode))
			{
				consoleMode |= ENABLE_EXTENDED_FLAGS;
				consoleMode &= ~ENABLE_QUICK_EDIT_MODE;

				SetConsoleMode(hConsole, consoleMode);
			}
		}
	});
#else
	g_allowVt = true;
#endif

	std::call_once(g_initConsoleFlag, []()
	{
		std::thread([]()
		{
			SetThreadName(-1, "[Cfx] Console Thread");

			while (true)
			{
				{
					std::unique_lock<std::mutex> lock(g_consoleMutex);
					g_consoleCondVar.wait(lock);
				}

				std::tuple<std::string, std::string> strRef;

				while (g_consolePrintQueue.try_pop(strRef))
				{
					g_isPrinting = true;
					g_printChannel = std::get<0>(strRef);
					CfxPrintf(std::get<1>(strRef));
					g_printChannel = "";
					g_isPrinting = false;
				}
			}
		}).detach();
	});

	g_consolePrintQueue.push({ channel, std::string{ out } });
	g_consoleCondVar.notify_all();
}
}

bool GIsPrinting()
{
	return !console::g_consolePrintQueue.empty() || console::g_isPrinting;
}

namespace console
{
static fwEvent<ConsoleChannel, const char*> g_printFilter;
static std::vector<void (*)(ConsoleChannel, const char*)> g_printListeners = { PrintfTraceListener };
static int g_useDeveloper;

void Printfv(ConsoleChannel channel, std::string_view format, fmt::printf_args argList)
{
	auto buffer = fmt::vsprintf(format, argList);

	// replace NUL characters, if any
	if (buffer.find('\0') != std::string::npos)
	{
		boost::algorithm::replace_all(buffer, std::string_view{ "\0", 1 }, "^5<\\0>^7");
	}

	// run print filter
	if (!g_printFilter(channel, buffer.data()))
	{
		return;
	}

	// print to all interested listeners
	for (auto& listener : g_printListeners)
	{
		listener(channel, buffer.data());
	}
}

void DPrintfv(ConsoleChannel channel, std::string_view format, fmt::printf_args argList)
{
	if (g_useDeveloper > 0)
	{
		Printfv(channel, format, argList);
	}
}

void PrintWarningv(ConsoleChannel channel, std::string_view format, fmt::printf_args argList)
{
	// print the string directly
	Printf(channel, "^3Warning: %s^7", fmt::vsprintf(format, argList));
}

void PrintErrorv(ConsoleChannel channel, std::string_view format, fmt::printf_args argList)
{
	// print the string directly
	Printf(channel, "^1Error: %s^7", fmt::vsprintf(format, argList));
}

static ConVar<int> developerVariable(GetDefaultContext(), "developer", ConVar_Archive, 0, &g_useDeveloper);
}

extern "C" DLL_EXPORT void CoreAddPrintListener(void(*function)(ConsoleChannel, const char*))
{
	console::g_printListeners.push_back(function);
}

extern "C" DLL_EXPORT fwEvent<ConsoleChannel, const char*>* CoreGetPrintFilterEvent()
{
	return &console::g_printFilter;
}

extern "C" DLL_EXPORT void CoreSetPrintFunction(void(*function)(const char*))
{
	console::g_printf = function;
}
