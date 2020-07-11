#include <StdInc.h>
#include "Console.Commands.h"
#include "Console.Variables.h"
#include "Console.h"

#include <se/Security.h>

#include <chrono>
#include <sstream>

#include <boost/algorithm/string.hpp>

#include <console/ConsoleWriter.h>

#if __has_include(<json.hpp>)
#define CONSOLE_JSON_PARSE

#include <json.hpp>

using json = nlohmann::json;

/*!
Input adapter for a (caching) istream. Ignores a UFT Byte Order Mark at
beginning of input. Does not support changing the underlying std::streambuf
in mid-input. Maintains underlying std::istream and std::streambuf to support
subsequent use of standard std::istream operations to process any input
characters following those used in parsing the JSON input.  Clears the
std::istream flags; any input errors (e.g., EOF) will be detected by the first
subsequent call for input from the std::istream.
*/
template<typename TChar>
class input_stream_adapter_any
{
public:
	using char_type = TChar;

	~input_stream_adapter_any()
	{
		// clear stream flags; we use underlying streambuf I/O, do not
		// maintain ifstream flags, except eof
		if (is)
		{
			is->clear(is->rdstate() & std::ios::eofbit);
		}
	}

	explicit input_stream_adapter_any(std::basic_istream<TChar>& i)
		: is(&i), sb(i.rdbuf())
	{
	}

	// delete because of pointer members
	input_stream_adapter_any(const input_stream_adapter_any&) = delete;
	input_stream_adapter_any& operator=(input_stream_adapter_any&) = delete;
	input_stream_adapter_any& operator=(input_stream_adapter_any&& rhs) = delete;

	input_stream_adapter_any(input_stream_adapter_any&& rhs)
		: is(rhs.is), sb(rhs.sb)
	{
		rhs.is = nullptr;
		rhs.sb = nullptr;
	}

	// std::istream/std::streambuf use std::char_traits<char>::to_int_type, to
	// ensure that std::char_traits<char>::eof() and the character 0xFF do not
	// end up as the same value, eg. 0xFFFFFFFF.
	auto get_character()
	{
		auto res = sb->sbumpc();
		// set eof manually, as we don't use the istream interface.
		if (res == EOF)
		{
			is->clear(is->rdstate() | std::ios::eofbit);
		}
		return res;
	}

private:
	/// the associated input stream
	std::basic_istream<TChar>* is = nullptr;
	std::basic_streambuf<TChar>* sb = nullptr;
};

namespace nlohmann::detail
{
template<typename TChar>
inline input_stream_adapter_any<TChar> input_adapter(std::basic_istream<TChar>& stream)
{
	return input_stream_adapter_any<TChar>(stream);
}

template<typename TChar>
inline input_stream_adapter_any<TChar> input_adapter(std::basic_istream<TChar>&& stream)
{
	return input_stream_adapter_any<TChar>(stream);
}
}
#endif

static console::IWriter* g_writer;
extern bool GIsPrinting();

namespace console
{
struct ConsoleManagers : public ConsoleManagersBase
{
	std::unique_ptr<ConsoleCommandManager> commandManager;

	std::unique_ptr<ConsoleVariableManager> variableManager;

	std::shared_ptr<ConsoleCommand> helpCommand;

	std::chrono::milliseconds waitUntil;
	std::shared_ptr<ConsoleCommand> waitCommand;

	ConsoleManagers()
		: waitUntil(0)
	{

	}
};

Context::Context()
    : Context(GetDefaultContext())
{
}

Context::Context(Context* fallbackContext)
    : m_fallbackContext(fallbackContext), m_executing(false)
{
	m_managers = std::make_unique<ConsoleManagers>();

	ConsoleManagers* managers = static_cast<ConsoleManagers*>(m_managers.get());
	managers->commandManager  = std::make_unique<ConsoleCommandManager>(this);
	managers->variableManager = std::make_unique<ConsoleVariableManager>(this);

	managers->waitCommand = std::make_shared<ConsoleCommand>(managers->commandManager.get(), "wait", [managers](int msec)
	{
		if (managers->waitUntil.count() == 0)
		{
			managers->waitUntil = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
		}

		managers->waitUntil += std::chrono::milliseconds{ msec };
	});

	managers->helpCommand = std::make_shared<ConsoleCommand>(managers->commandManager.get(), "cmdlist", [=]()
	{
		std::set<std::string, IgnoreCaseLess> commands;

		managers->commandManager->ForAllCommands([&](const std::string& cmdName)
		{
			commands.insert(cmdName);
		});

		if (m_fallbackContext)
		{
			m_fallbackContext->GetCommandManager()->ForAllCommands([&](const std::string& cmdName)
			{
				commands.insert(cmdName);
			});
		}

		for (auto& commandName : commands)
		{
			if (!seCheckPrivilege(fmt::sprintf("command.%s", commandName)))
			{
				continue;
			}

			auto cvar = managers->variableManager->FindEntryRaw(commandName);

			if (!cvar)
			{
				if (m_fallbackContext)
				{
					cvar = m_fallbackContext->GetVariableManager()->FindEntryRaw(commandName);
				}
			}

			if (cvar)
			{
				console::Printf("CmdSystem", "%s = %s^7\n", commandName, cvar->GetValue());
			}
			else
			{
				console::Printf("CmdSystem", "%s\n", commandName);
			}
		}
	});

	m_variableModifiedFlags = 0;
}

ConsoleVariableManager* Context::GetVariableManager()
{
	return static_cast<ConsoleManagers*>(m_managers.get())->variableManager.get();
}

ConsoleCommandManager* Context::GetCommandManager()
{
	return static_cast<ConsoleManagers*>(m_managers.get())->commandManager.get();
}

void Context::ExecuteSingleCommand(const std::string& command)
{
	GetCommandManager()->Invoke(command);
}

void Context::ExecuteSingleCommandDirect(const ProgramArguments& arguments)
{
	// early out if no command nor arguments were passed
	if (arguments.Count() == 0)
	{
		return;
	}

	// make a copy of the arguments to shift off the command name
	ProgramArguments localArgs(arguments);
	std::string command = localArgs.Shift();

	// run the command through the command manager
	GetCommandManager()->InvokeDirect(command, localArgs);
}

void Context::AddToBuffer(const std::string& text)
{
	std::lock_guard<std::mutex> guard(m_commandBufferMutex);
	m_commandBuffer += text;
}

template<typename TFn>
struct defer
{
	inline defer(TFn&& fn)
		: fn(fn)
	{

	}

	inline ~defer()
	{
		fn();
	}

private:
	TFn fn;
};

void Context::ExecuteBuffer()
{
	ConsoleManagers* managers = static_cast<ConsoleManagers*>(m_managers.get());

	// if we have executed a `wait`, do that
	if (managers->waitUntil.count() != 0 && managers->waitUntil >= std::chrono::high_resolution_clock::now().time_since_epoch())
	{
		return;
	}

	managers->waitUntil = std::chrono::milliseconds{ 0 };

	std::vector<std::string> toExecute;

	{
		std::lock_guard<std::mutex> guard(m_commandBufferMutex);

		while (m_commandBuffer.length() > 0)
		{
			// parse the command up to the first occurrence of a newline/semicolon
			size_t i     = 0;
			bool inQuote = false;

			size_t cbufLength = m_commandBuffer.length();

			for (i = 0; i < cbufLength; i++)
			{
#ifdef CONSOLE_JSON_PARSE
				if (!inQuote && (m_commandBuffer[i] == '{' || m_commandBuffer[i] == '['))
				{
					std::stringstream oss;
					oss << m_commandBuffer.substr(i);
					oss.seekg(0);

					try
					{
						json j;
						oss >> j;

						i += oss.tellg();
						i -= 1;
						continue;
					}
					catch (json::exception&)
					{
					}
				}
#endif

				if (m_commandBuffer[i] == '"')
				{
					inQuote = !inQuote;
				}

				// break if a semicolon
				if (!inQuote && m_commandBuffer[i] == ';')
				{
					break;
				}

				// or a newline
				if (m_commandBuffer[i] == '\r' || m_commandBuffer[i] == '\n')
				{
					break;
				}
			}

			std::string command = m_commandBuffer.substr(0, i);

			if (cbufLength > i)
			{
				m_commandBuffer = m_commandBuffer.substr(i + 1);
			}
			else
			{
				m_commandBuffer.clear();
			}

			// and add the command for execution when the mutex is unlocked
			toExecute.push_back(command);

			// if the command is `wait`, break out of the execution loop
			boost::algorithm::trim_left(command);

			if (command.find("wait") == 0)
			{
				break;
			}
		}
	}

	auto d = defer{ [this] {m_executing = false; } };
	m_executing = true;

	for (const std::string& command : toExecute)
	{
		ExecuteSingleCommand(command);
	}
}

static void SaveConfiguration(const std::string& path, console::Context* context, ConsoleVariableManager* varMan)
{
	if (!g_writer)
	{
		return;
	}

	auto handle = g_writer->Create(path);

	if (handle != -1)
	{
		auto writeLine = [&](const std::string& line) {
			const char newLine[] = { '\r', '\n' };

			g_writer->Write(handle, line.c_str(), line.size());
			g_writer->Write(handle, newLine, sizeof(newLine));
		};

		// write a cutesy warning
		writeLine("// generated by CitizenFX");

		context->OnSaveConfiguration(writeLine);

		// save the actual configuration
		varMan->SaveConfiguration(writeLine);

		g_writer->Close(handle);
	}
}

void Context::SaveConfigurationIfNeeded(const std::string& path)
{
	// check if the configuration was saved already
	static bool wasSavedBefore = false;

	// mark a flag to see if any variables are modified (or if we haven't done our initial save)
	if (!wasSavedBefore || (m_variableModifiedFlags & ConVar_Archive))
	{
		console::DPrintf("cmd", "Saving configuration to %s...\n", path.c_str());

		SaveConfiguration(path, this, GetVariableManager());

		wasSavedBefore = true;

		m_variableModifiedFlags &= ~ConVar_Archive;
	}
}

void Context::SetVariableModifiedFlags(int flags)
{
	m_variableModifiedFlags |= flags;
}

bool Context::GIsPrinting()
{
	return ::GIsPrinting();
}

// default context functions
Context* GetDefaultContext()
{
	static std::unique_ptr<Context> defaultContext;
	static std::once_flag flag;

	std::call_once(flag, []() {
		// nullptr is important - we don't have ourselves to fall back on!
		defaultContext = std::make_unique<Context>(nullptr);
	});

	return defaultContext.get();
}

void CreateContext(Context* parentContext, fwRefContainer<Context>* outContext)
{
	*outContext = new Context(parentContext);
}

void ExecuteSingleCommand(const std::string& command)
{
	return GetDefaultContext()->ExecuteSingleCommand(command);
}

void ExecuteSingleCommandDirect(const ProgramArguments& arguments)
{
	return GetDefaultContext()->ExecuteSingleCommandDirect(arguments);
}

void AddToBuffer(const std::string& text)
{
	return GetDefaultContext()->AddToBuffer(text);
}

void ExecuteBuffer()
{
	return GetDefaultContext()->ExecuteBuffer();
}

void SaveConfigurationIfNeeded(const std::string& path)
{
	return GetDefaultContext()->SaveConfigurationIfNeeded(path);
}

void SetVariableModifiedFlags(int flags)
{
	return GetDefaultContext()->SetVariableModifiedFlags(flags);
}

static inline bool IsEscapeChar(typename ProgramArguments::TCharType c)
{
	return (c == U'"');
}

ProgramArguments Tokenize(const std::string& lineUtf8)
{
	int i = 0;
	int j = 0;
	std::vector<std::basic_string<typename ProgramArguments::TCharType>> args;

	std::basic_string<typename ProgramArguments::TCharType> line;

	// VC++ 14.0 libraries don't export this symbol, MSFT won't fix until next incompatible version
	// so we use uint32_t for VC++
#ifndef _MSC_VER
	static std::wstring_convert<std::codecvt_utf8<typename ProgramArguments::TCharType>, typename ProgramArguments::TCharType> converter;
#else
	static std::wstring_convert<std::codecvt_utf8<uint32_t>, uint32_t> converter;
#endif
	
	try
	{
		// once MSVC conforms this won't be needed anymore
		auto tempLine = converter.from_bytes(lineUtf8);

		line = std::basic_string<typename ProgramArguments::TCharType>(tempLine.begin(), tempLine.end());
	}
	catch (std::range_error&)
	{
		return ProgramArguments{};
	}

	size_t lineLength = line.length();

	// outer loop
	while (true)
	{
		// inner loop to skip whitespace
		while (true)
		{
			// skip whitespace and control characters
			while (i < lineLength && line[i] <= U' ') // ASCII only?
			{
				i++;
			}

			// return if needed
			if (i >= lineLength)
			{
				return ProgramArguments{args};
			}

			// allegedly fixes issues with parsing
			if (i == 0)
			{
				break;
			}

			// skip comments
			if ((line[i] == U'/' && line[i + 1] == U'/') || line[i] == U'#') // full line is a comment
			{
				return ProgramArguments{args};
			}

			// /* comments
			if (line[i] == U'/' && line[i + 1] == U'*')
			{
				while (i < (lineLength + 1) && (line[i] != U'*' && line[i + 1] != U'/'))
				{
					i++;
				}

				if (i >= lineLength)
				{
					return ProgramArguments{args};
				}

				i += 2;
			}
			else
			{
				break;
			}
		}

		// there's a new argument on the block
		std::basic_stringstream<typename ProgramArguments::TCharType> arg;

#ifdef CONSOLE_JSON_PARSE
		if ((line[i] == '{' || line[i] == '['))
		{
			std::basic_stringstream<typename ProgramArguments::TCharType> oss;
			oss << line.substr(i);
			oss.seekg(0);

			try
			{
				json j;
				oss >> j;

				int start = i;
				i += oss.tellg();

				args.push_back(line.substr(start, i - start));

				continue;
			}
			catch (json::exception&)
			{
			}
		}
#endif

		// quoted strings
		if (line[i] == U'"')
		{
			bool inEscape = false;

			while (true)
			{
				i++;

				if (i >= lineLength)
				{
					break;
				}

				if (line[i] == U'"' && !inEscape)
				{
					break;
				}

				if (line[i] == U'\\' && IsEscapeChar(line[i + 1]))
				{
					inEscape = true;
				}
				else
				{
					arg << line[i];
					inEscape = false;
				}
			}

			i++;

			args.push_back(arg.str());
			j++;

			if (i >= lineLength)
			{
				return ProgramArguments{args};
			}

			continue;
		}

		// non-quoted strings
		while (i < lineLength && line[i] > U' ')
		{
			if (line[i] == U'"')
			{
				break;
			}

			// # comments are one character long
			if (i < lineLength)
			{
				if (line[i] == U'#')
				{
					return ProgramArguments{args};
				}
			}

			if (i < (lineLength - 1))
			{
				if ((line[i] == U'/' && line[i + 1] == U'/'))
				{
					return ProgramArguments{args};
				}

				if (line[i] == U'/' && line[i + 1] == U'*')
				{
					return ProgramArguments{args};
				}
			}

			arg << line[i];

			i++;
		}

		auto argStr = arg.str();

		if (!argStr.empty())
		{
			args.push_back(argStr);
			j++;
		}

		if (i >= lineLength)
		{
			return ProgramArguments{args};
		}
	}
}
}

static thread_local bool inTracing;

void CoreTrace(const char* channel, const char* func, const char* file, int line, const char* string)
{
	inTracing = true;
	console::Printf(channel, "%s", string);
	inTracing = false;
}

void SetConsoleWriter(console::IWriter* writer)
{
	g_writer = writer;
}

#ifdef _WIN32
static void(*g_asyncTrace)(const char*);

extern "C" DLL_EXPORT void AsyncTrace(const char* string)
{
	if (g_asyncTrace)
	{
		g_asyncTrace(string);
	}
}

extern "C" void CoreAddPrintListener(void(*function)(ConsoleChannel, const char*));
#endif

static InitFunction initFunction([]()
{
#ifdef _WIN32
	g_asyncTrace = (decltype(g_asyncTrace))GetProcAddress(GetModuleHandle(NULL), "AsyncTrace");

	CoreAddPrintListener([](ConsoleChannel channel, const char* message)
	{
		if (!inTracing)
		{
			AsyncTrace(message);
		}
	});
#endif

	auto cxt = console::GetDefaultContext();
	Instance<ConsoleCommandManager>::Set(cxt->GetCommandManager());
	Instance<ConsoleVariableManager>::Set(cxt->GetVariableManager());
	Instance<console::Context>::Set(cxt);
});
