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
class Context : public fwRefCountable
{
public:
	Context();

	Context(Context* fallbackContext);

	virtual void ExecuteSingleCommand(const std::string& command);

	virtual void ExecuteSingleCommandDirect(const ProgramArguments& arguments);

	virtual void AddToBuffer(const std::string& text);

	virtual void ExecuteBuffer();

	virtual void SaveConfigurationIfNeeded(const std::string& path);

	virtual void SetVariableModifiedFlags(int flags);

	virtual bool GIsPrinting();

	virtual ConsoleCommandManager* GetCommandManager();

	virtual ConsoleVariableManager* GetVariableManager();

	inline Context* GetFallbackContext()
	{
		return m_fallbackContext;
	}

	inline bool IsBufferEmpty()
	{
		std::lock_guard<std::mutex> guard(m_commandBufferMutex);
		return m_commandBuffer.empty() && !m_executing;
	}

public:
	fwEvent<const std::function<void(const std::string&)>&> OnSaveConfiguration;

private:
	Context* m_fallbackContext;

	int m_variableModifiedFlags;

	std::unique_ptr<ConsoleManagersBase> m_managers;

	std::string m_commandBuffer;

	std::mutex m_commandBufferMutex;

	volatile bool m_executing;
};

#ifdef COMPILING_CORE
extern "C" DLL_EXPORT Context* GetDefaultContext();

extern "C" DLL_EXPORT void CreateContext(Context* parentContext, fwRefContainer<Context>* outContext);
#elif defined(_WIN32)
inline Context* GetDefaultContext()
{
	using TCoreFunc = decltype(&GetDefaultContext);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "GetDefaultContext");
	}

	return (func) ? func() : 0;
}

inline void CreateContext(Context* parentContext, fwRefContainer<Context>* outContext)
{
	using TCoreFunc = decltype(&CreateContext);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CreateContext");
	}

	return (func) ? func(parentContext, outContext) : (void)nullptr;
}
#else
extern "C" Context* GetDefaultContext();
extern "C" void CreateContext(Context* parentContext, fwRefContainer<Context>* outContext);
#endif

void ExecuteSingleCommand(const std::string& command);

void ExecuteSingleCommandDirect(const ProgramArguments& arguments);

ProgramArguments Tokenize(const std::string& line);

void AddToBuffer(const std::string& text);

void ExecuteBuffer();

void SaveConfigurationIfNeeded(const std::string& path);

void SetVariableModifiedFlags(int flags);
}

DECLARE_INSTANCE_TYPE(console::Context);
