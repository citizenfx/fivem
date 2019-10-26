#include <StdInc.h>
#include <replxx.hxx>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <CoreConsole.h>
#include <ServerInstanceBase.h>

#include <ResourceManager.h>

#include <boost/algorithm/string/trim.hpp>

#include <utf8.h>
#include <regex>

using namespace std::chrono_literals;

static int utf8str_codepoint_len(char const* s, int utf8len)
{
	int codepointLen = 0;
	unsigned char m4 = 128 + 64 + 32 + 16;
	unsigned char m3 = 128 + 64 + 32;
	unsigned char m2 = 128 + 64;
	for (int i = 0; i < utf8len; ++i, ++codepointLen)
	{
		char c = s[i];
		if ((c & m4) == m4) {
			i += 3;
		}
		else if ((c & m3) == m3) {
			i += 2;
		}
		else if ((c & m2) == m2) {
			i += 1;
		}
	}
	return (codepointLen);
};

void hook_color(std::string const& context, replxx::Replxx::colors_t& colors, std::vector<std::pair<std::string, replxx::Replxx::Color>> const& regex_color)
{
	// highlight matching regex sequences
	for (auto const& e : regex_color)
	{
		size_t pos{ 0 };
		std::string str = context;
		std::smatch match;

		while (std::regex_search(str, match, std::regex(e.first)))
		{
			std::string c{ match[0] };
			std::string prefix(match.prefix().str());
			pos += utf8str_codepoint_len(prefix.c_str(), static_cast<int>(prefix.length()));
			int len(utf8str_codepoint_len(c.c_str(), static_cast<int>(c.length())));

			for (int i = 0; i < len; ++i) {
				colors.at(pos + i) = e.second;
			}

			pos += len;
			str = match.suffix();
		}
	}
}

static InitFunction initFunction([]()
{
	static replxx::Replxx rxx;
	rxx.install_window_change_handler();

	// replxx printing does *not* like incremental writes that don't involve \n as it will instantly overwrite them with its prompt
#if 0
	console::CoreSetPrintFunction([](const char* str)
	{
		rxx.print("%s", str);
	});
#endif

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->OnInitialConfiguration.Connect([=]()
		{
			static fwRefContainer<console::Context> con = instance->GetComponent<console::Context>();
			static fwRefContainer<fx::ResourceManager> rm = instance->GetComponent<fx::ResourceManager>();

			static auto disableTTYVariable = instance->AddVariable<bool>("con_disableNonTTYReads", ConVar_None, false);

			auto getCmdsForContext = [](const std::string& input, int& contextLen)
			{
				static std::set<std::string> cmds;

				// make a substring of the input of [contextLen] codepoints
				auto it = input.end();

				for (int i = 0; i < contextLen; i++)
				{
					utf8::prior(it, input.begin());
				}

				auto inputCopy = std::string(it, input.end());
				boost::algorithm::trim_left(inputCopy);

				// count new (trimmed) context length
				int len = 0;

				for (auto it = inputCopy.begin(); it != inputCopy.end(); utf8::next(it, inputCopy.end()))
				{
					len++;
				}

				contextLen = len;

				// try to find a command start
				auto trackStart = input.length() - inputCopy.length();

				while (trackStart > 0)
				{
					if (input[trackStart] == ';')
					{
						break;
					}

					trackStart--;
				}

				auto track = input.substr(trackStart);

				// get the context type
				static std::regex curCmdRe{ "(?:^|;\\s*)([^\\s;]+)(\\s+(.+)?)?(?:$|;)" };
				enum class ContextType
				{
					None,
					Command,
					Resource
				};

				ContextType cxt = ContextType::Command;
				
				std::smatch matches;

				if (std::regex_search(track, matches, curCmdRe))
				{
					// has spacing for args?
					if (matches[2].matched)
					{
						auto cmd = matches[1].str();
						auto argStr = (matches[3].matched) ? matches[3].str() : "";

						// do we need a resource completion list?
						bool isResourceCommand = false;

						for (auto test : { "ensure", "start", "stop", "restart" })
						{
							if (cmd == test)
							{
								isResourceCommand = true;
							}
						}

						// gather resource completion
						if (isResourceCommand)
						{
							cxt = ContextType::Resource;
						}
						else
						{
							cxt = ContextType::None;
						}
					}
				}

				// get commands
				cmds.clear();

				switch (cxt)
				{
				case ContextType::Command:
					con->GetCommandManager()->ForAllCommands([&inputCopy](const std::string& cmd)
					{
						if (cmd == "_crash")
						{
							return;
						}

						if (strncmp(cmd.c_str(), inputCopy.c_str(), inputCopy.length()) == 0)
						{
							cmds.insert(cmd);
						}
					});
					break;

				case ContextType::Resource:
					rm->ForAllResources([&inputCopy](const fwRefContainer<fx::Resource>& resource)
					{
						auto cmd = resource->GetName();

						if (cmd == "_cfx_internal")
						{
							return;
						}

						if (strncmp(cmd.c_str(), inputCopy.c_str(), inputCopy.length()) == 0)
						{
							cmds.insert(cmd);
						}
					});

					break;

				case ContextType::None:
				default:
					break;
				}

				return std::vector<std::string>{cmds.begin(), cmds.end()};
			};

			using cl = replxx::Replxx::Color;
			static std::vector<std::pair<std::string, cl>> regex_color
			{
				// commands
				{ "(^|;\\s*)[^\\s;]+", cl::BRIGHTMAGENTA },

				// special characters
				{"\\\"", cl::BRIGHTBLUE},
				{";", cl::BRIGHTMAGENTA},

				// numbers
				{"[\\-|+]{0,1}[0-9]+", cl::YELLOW}, // integers
				{"[\\-|+]{0,1}[0-9]*\\.[0-9]+", cl::YELLOW}, // decimals
				{"[\\-|+]{0,1}[0-9]+e[\\-|+]{0,1}[0-9]+", cl::YELLOW}, // scientific notation

				// booleans
				{"true", cl::BRIGHTBLUE},
				{"false", cl::BRIGHTBLUE},

				// strings
				{"\".*?(\"|$)", cl::BRIGHTGREEN}, // double quotes

			};

			rxx.set_completion_callback([=](const std::string& input, int& contextLen)
			{
				return getCmdsForContext(input, contextLen);
			});

			rxx.set_hint_callback([=](const std::string& input, int& contextLen, replxx::Replxx::Color& color) -> replxx::Replxx::hints_t
			{
				if (input.length() > 0)
				{
					auto cmds = getCmdsForContext(input, contextLen);

					if (!cmds.empty())
					{
						return { cmds[0] };
					}
				}

				return {};
			});

			rxx.set_highlighter_callback(std::bind(&hook_color, std::placeholders::_1, std::placeholders::_2, std::cref(regex_color)));

			rxx.set_word_break_characters("; ");

			rxx.set_double_tab_completion(false);
			rxx.set_complete_on_empty(true);
			rxx.set_beep_on_ambiguous_completion(true);

			std::thread([=]()
			{
				while (true)
				{
					if (disableTTYVariable->GetValue() && !isatty(fileno(stdin)))
					{
						std::this_thread::sleep_for(1000ms);
						continue;
					}

					// wait until console buffer was processed
					while (!con->IsBufferEmpty() || con->GIsPrinting())
					{
						std::this_thread::sleep_for(25ms);
					}

					const char* result = rxx.input("cfx> ");

					// null result?
					if (result == nullptr)
					{
						con->AddToBuffer(fmt::sprintf("quit \"Ctrl-C pressed in server console.\"\n"));
						break;
					}

					// make a string and free
					std::string resultStr = result;

					// handle input
					con->AddToBuffer(resultStr);
					con->AddToBuffer("\n");

					rxx.history_add(resultStr);
				}
			}).detach();
		}, 9999);
	});
});
