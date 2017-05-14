#pragma once

#include "Console.Base.h"
#include "Console.Commands.h"
#include "ProgramArguments.h"

namespace detail
{
	//plain function pointers
	template <typename... Args, typename ReturnType>
	inline auto make_function(ReturnType(*p)(Args...)) -> std::function<ReturnType(Args...)>
	{
		return { p };
	}

	//nonconst member function pointers
	template <typename... Args, typename ReturnType, typename ClassType>
	inline auto make_function(ReturnType(ClassType::*p)(Args...)) -> std::function<ReturnType(Args...)>
	{
		return { p };
	}

	//const member function pointers
	template <typename... Args, typename ReturnType, typename ClassType>
	inline auto make_function(ReturnType(ClassType::*p)(Args...) const) -> std::function<ReturnType(Args...)>
	{
		return { p };
	}

	//qualified functionoids
	template <typename FirstArg, typename... Args, class T>
	inline auto make_function(T&& t) -> std::function<decltype(t(std::declval<FirstArg>(), std::declval<Args>()...))(FirstArg, Args...)>
	{
		return { std::forward<T>(t) };
	}

	//unqualified functionoids try to deduce the signature of `T::operator()` and use that.
	template <class T>
	inline auto make_function(T&& t) -> decltype(make_function(&std::remove_reference<T>::type::operator()))
	{
		return { std::forward<T>(t) };
	}
}

class ConsoleManagersBase
{
public:
	virtual ~ConsoleManagersBase() = default;
};

class ConsoleVariableManager;

namespace console
{
class Context
{
public:
	Context();

	Context(Context* fallbackContext);

	virtual void ExecuteSingleCommand(const std::string& command);

	virtual void ExecuteSingleCommand(const ProgramArguments& arguments);

	virtual void AddToBuffer(const std::string& text);

	virtual void ExecuteBuffer();

	virtual void SaveConfigurationIfNeeded(const std::string& path);

	virtual void SetVariableModifiedFlags(int flags);

	virtual ConsoleCommandManager* GetCommandManager();

	virtual ConsoleVariableManager* GetVariableManager();

	inline Context* GetFallbackContext()
	{
		return m_fallbackContext;
	}

private:
	Context* m_fallbackContext;

	int m_variableModifiedFlags;

	std::unique_ptr<ConsoleManagersBase> m_managers;

	std::string m_commandBuffer;

	std::mutex m_commandBufferMutex;
};

Context* GetDefaultContext();

void ExecuteSingleCommand(const std::string& command);

void ExecuteSingleCommand(const ProgramArguments& arguments);

ProgramArguments Tokenize(const std::string& line);

void AddToBuffer(const std::string& text);

void ExecuteBuffer();

void SaveConfigurationIfNeeded(const std::string& path);

void SetVariableModifiedFlags(int flags);
}

DECLARE_INSTANCE_TYPE(console::Context);
