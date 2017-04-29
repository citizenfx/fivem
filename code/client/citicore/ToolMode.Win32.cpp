/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <boost/program_options.hpp>

#include "ComponentLoader.h"
#include "ToolComponent.h"

extern "C" DLL_EXPORT void ToolMode_Init()
{
	// initialize components to create a list of viable commands
	ComponentLoader* componentLoader = ComponentLoader::GetInstance();
	componentLoader->Initialize();

    componentLoader->ForAllComponents([&] (fwRefContainer<ComponentData> componentData)
    {
        for (auto& instance : componentData->GetInstances())
        {
            instance->Initialize();
        }
    });

	// instantiate all components, however do not initialize them - also save any components implementing tool functionality
	std::map<std::string, fwRefContainer<ComponentData>> toolCommands;

	componentLoader->ForAllComponents([&] (fwRefContainer<ComponentData> component)
	{
		fwRefContainer<Component> componentInstance = component->CreateManualInstance();
		Component* componentPtr = componentInstance.GetRef();

		// try it as tool
		ToolComponent* tool = dynamic_component_cast<ToolComponent*>(componentPtr);

		if (tool)
		{
			auto thisCommands = tool->GetCommandNames();

			for (auto&& command : thisCommands)
			{
				toolCommands[command] = component;
			}
		}
	});

	// parse program options
	namespace bpo = boost::program_options;

	try
	{
		const wchar_t* commandLine = GetCommandLine();

		// setup global options
		bpo::options_description options("Options");

		options.add_options()
			("command", bpo::value<std::string>(), "The command to run.")
			("args", bpo::value<std::vector<std::string>>(), "Arguments.");

		bpo::positional_options_description positional;
		positional.add("command", 1).add("args", -1);

		// parse global options
		bpo::variables_map globalVars;

		std::vector<std::wstring> mainArgs(bpo::split_winmain(commandLine));
		mainArgs.erase(mainArgs.begin());

		bpo::wparsed_options result = bpo::wcommand_line_parser(mainArgs).
			options(options).
			positional(positional).
			allow_unregistered().
			run();

		bpo::store(result, globalVars);

		if (globalVars.find("command") == globalVars.end())
		{
			printf("No command passed. Valid commands include: ");

			for (auto&& entry : toolCommands)
			{
				printf("%s ", entry.first.c_str());
			}

			printf("\n");

			return;
		}

		// get the command identifier
		std::string command = globalVars["command"].as<std::string>();

		// find the command in our command mapping
		auto it = toolCommands.find(command);

		if (it != toolCommands.end())
		{
			// initialize anything this component depends on
			fwRefContainer<ComponentData> componentData = it->second;

			std::function<bool(fwRefContainer<ComponentData>)> initDeps;
			std::set<std::string> initialized;

			initDeps = [&] (fwRefContainer<ComponentData> data)
			{
				for (auto&& dep : data->GetDependencyDataList())
				{
					if (!initDeps(dep))
					{
						return false;
					}
				}

				if (initialized.find(data->GetName()) == initialized.end())
				{
					if (!data->GetInstances()[0]->Initialize())
					{
						return false;
					}

					initialized.insert(data->GetName());
				}

				return true;
			};

			initDeps(componentData);
			
			// get the component
			fwRefContainer<Component> component = componentData->GetInstances()[0];

			// get the command implementation
			ToolComponent* tool = dynamic_component_cast<ToolComponent*>(component.GetRef());

			fwRefContainer<ToolCommand> commandImpl = tool->GetCommand(command);

			// get the remaining options, and remove the first entry (which is the command name)
			std::vector<std::wstring> remnantOptions = bpo::collect_unrecognized(result.options, bpo::include_positional);
			remnantOptions.erase(remnantOptions.begin());

			// parse and store in a new map
			bpo::variables_map vm;
			commandImpl->SetupCommandLineParser(bpo::wcommand_line_parser(remnantOptions), [&] (boost::program_options::wcommand_line_parser& parser)
			{
				bpo::store(parser.run(), vm);
			});

			// execute the command
			commandImpl->InvokeCommand(vm);
		}
		else
		{
			printf("Command [%s] is unknown to this tool.\n", command.c_str());
		}
	}
	catch (boost::program_options::required_option& e)
	{
		printf("Required option missing: %s\n", e.what());
	}
	catch (boost::program_options::error& e)
	{
		printf("error: %s\n", e.what());
	}
}

static void(*g_gameFunction)(const wchar_t*);
static void(*g_postLaunchRoutine)();

extern "C" DLL_EXPORT void ToolMode_RunPostLaunchRoutine()
{
	if (g_postLaunchRoutine)
	{
		g_postLaunchRoutine();
	}
}

extern "C" DLL_EXPORT void ToolMode_SetPostLaunchRoutine(void(*routine)())
{
	g_postLaunchRoutine = routine;
}

extern "C" DLL_EXPORT void ToolMode_LaunchGame(const wchar_t* argument)
{
	assert(g_gameFunction);

	g_gameFunction(argument);
}

extern "C" DLL_EXPORT void ToolMode_SetGameFunction(void(*gameFunction)(const wchar_t*))
{
	g_gameFunction = gameFunction;
}