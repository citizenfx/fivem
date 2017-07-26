#include <StdInc.h>
#include <linenoise.h>

#include <CoreConsole.h>
#include <ServerInstanceBase.h>

using namespace std::chrono_literals;

static InitFunction initFunction([]()
{
	linenoiseInstallWindowChangeHandler();

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->OnInitialConfiguration.Connect([=]()
		{
			static fwRefContainer<console::Context> con = instance->GetComponent<console::Context>();

			linenoiseSetCompletionCallback([](const char* prefix, linenoiseCompletions* lc)
			{
				static std::set<std::string> cmds;

				cmds.clear();

				con->GetCommandManager()->ForAllCommands([&](const std::string& cmd)
				{
					if (strncmp(cmd.c_str(), prefix, strlen(prefix)) == 0)
					{
						cmds.insert(cmd);
					}
				});

				for (auto& cmd : cmds)
				{
					linenoiseAddCompletion(lc, cmd.c_str());
				}
			});

			std::thread([=]()
			{
				while (true)
				{
					char* result = linenoise("cfx> ");

					// null result?
					if (result == nullptr)
					{
						continue;
					}

					// make a string and free
					std::string resultStr = result;
					free(result);

					// handle input
					con->AddToBuffer(resultStr);
					con->AddToBuffer("\n");

					linenoiseHistoryAdd(resultStr.c_str());

					// wait until console buffer was processed
					while (!con->IsBufferEmpty())
					{
						std::this_thread::sleep_for(25ms);
					}
				}
			}).detach();
		}, 9999);
	});
});
