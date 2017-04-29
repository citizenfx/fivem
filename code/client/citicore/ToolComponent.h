/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <boost/program_options.hpp>

class ToolCommand : public fwRefCountable
{
public:
	virtual void SetupCommandLineParser(boost::program_options::wcommand_line_parser& parser, std::function<void(boost::program_options::wcommand_line_parser&)> cb) = 0;

	virtual void InvokeCommand(const boost::program_options::variables_map& variables) = 0;
};

class ToolComponent
{
public:
	virtual std::vector<std::string> GetCommandNames() = 0;

	virtual fwRefContainer<ToolCommand> GetCommand(const std::string& commandName) = 0;
};

#ifdef COMPILING_CORE
extern "C" CORE_EXPORT void ToolMode_RunPostLaunchRoutine();
extern "C" CORE_EXPORT void ToolMode_SetPostLaunchRoutine(void(*routine)());
extern "C" CORE_EXPORT void ToolMode_LaunchGame(const wchar_t* argument);
#else
inline void ToolMode_RunPostLaunchRoutine()
{
    static void(*func)();

    if (!func)
    {
        func = (void(*)())GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "ToolMode_RunPostLaunchRoutine");
    }

    return func();
}

inline void ToolMode_SetPostLaunchRoutine(void(*routine)())
{
    static void(*func)(void(*)());

    if (!func)
    {
        func = (void(*)(void(*)()))GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "ToolMode_SetPostLaunchRoutine");
    }

    return func(routine);
}

inline void ToolMode_LaunchGame(const wchar_t* argument)
{
    static void(*func)(const wchar_t*);

    if (!func)
    {
        func = (void(*)(const wchar_t*))GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "ToolMode_LaunchGame");
    }

    return func(argument);
}
#endif