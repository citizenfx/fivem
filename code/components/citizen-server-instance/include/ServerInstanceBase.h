/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ComponentHolder.h>

#include <console/Console.CommandHelpers.h>
#include <console/Console.VariableHelpers.h>

namespace fx
{
	class
#ifdef COMPILING_CITIZEN_SERVER_INSTANCE
		DLL_EXPORT
#else
		DLL_IMPORT
#endif
		ServerInstanceBase : public fwRefCountable, public ComponentHolderImpl<ServerInstanceBase>
	{
	public:
		virtual const std::string& GetRootPath() = 0;

	public:
		fwEvent<> OnInitialConfiguration;

		fwEvent<const std::string&> OnRequestQuit;

	public:
		static fwEvent<ServerInstanceBase*> OnServerCreate;

	public:
		template<typename TFunc>
		inline std::shared_ptr<ConsoleCommand> AddCommand(const std::string& name, const TFunc& func)
		{
			auto context = GetComponent<console::Context>();

			return std::make_shared<ConsoleCommand>(context.GetRef(), name, func);
		}

		template<typename TVar, typename... TArgs>
		inline std::shared_ptr<ConVar<TVar>> AddVariable(const TArgs&... args)
		{
			auto context = GetComponent<console::Context>();

			return std::make_shared<ConVar<TVar>>(context.GetRef(), args...);
		}
	};
}
