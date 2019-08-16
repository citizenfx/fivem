#pragma once

#include <atomic>
#include <functional>
#include <list>

#include <sstream>

#include "ProgramArguments.h"

#include <shared_mutex>

#include <any>

namespace console
{
class Context;

struct IgnoreCaseLess
{
	inline bool operator()(const std::string& left, const std::string& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) < 0;
	}
};
}

// console execution context
struct ConsoleExecutionContext
{
	const ProgramArguments arguments;
	std::stringstream errorBuffer;
	std::string contextRef;

	inline ConsoleExecutionContext(const ProgramArguments&& arguments, const std::string& contextRef)
	    : arguments(arguments), contextRef(contextRef)
	{
	}
};

class ConsoleCommandManager
{
private:
	using THandler = std::function<bool(ConsoleExecutionContext& context)>;

public:
	ConsoleCommandManager(console::Context* context);

	virtual ~ConsoleCommandManager();

	virtual int Register(const std::string& name, const THandler& handler);

	virtual void Unregister(int token);

	virtual void InvokeDirect(const std::string& commandName, const ProgramArguments& arguments, const std::string& executionContext = std::string());

	virtual void Invoke(const std::string& commandString, const std::string& executionContext = std::string());

	virtual void ForAllCommands(const std::function<void(const std::string&)>& callback);

	virtual const std::string& GetRawCommand();

public:
	fwEvent<const std::string&, const ProgramArguments&, const std::string&> FallbackEvent;

private:
	struct Entry
	{
		std::string name;
		THandler function;

		int token;

		inline Entry(const std::string& name, const THandler& function, int token)
		    : name(name), function(function), token(token)
		{
		}
	};

private:
	console::Context* m_parentContext;

	std::multimap<std::string, Entry, console::IgnoreCaseLess> m_entries;

	std::shared_mutex m_mutex;

	std::string m_rawCommand;

	std::atomic<int> m_curToken;

public:
	inline static ConsoleCommandManager* GetDefaultInstance()
	{
		return Instance<ConsoleCommandManager>::Get();
	}
};

template <typename TArgument, typename TConstraint = void>
struct ConsoleArgumentTraits
{
	using Less    = std::less<TArgument>;
	using Greater = std::greater<TArgument>;
	using Equal   = std::equal_to<TArgument>;
};

template <typename TArgument, typename TConstraint = void>
struct ConsoleArgumentName
{
	inline static const char* Get()
	{
		return typeid(TArgument).name();
	}
};

template <typename TArgument, typename TConstraint = void>
struct ConsoleArgumentType
{
	static std::string Unparse(const TArgument& argument)
	{
		//static_assert(false, "Unknown ConsoleArgumentType unparse handler (try defining one?)");
		TArgument::__fatal();
	}

	static bool Parse(const std::string& input, TArgument* out)
	{
		//static_assert(false, "Unknown ConsoleArgumentType parse handler (try defining one?)");
		TArgument::__fatal();
	}
};

template <>
struct ConsoleArgumentType<std::string>
{
	static std::string Unparse(const std::string& input)
	{
		return input;
	}

	static bool Parse(const std::string& input, std::string* out)
	{
		*out = input;
		return true;
	}
};

template <>
struct ConsoleArgumentName<std::string>
{
	inline static const char* Get()
	{
		return "string";
	}
};

template <typename TArgument>
struct ConsoleArgumentType<TArgument, typename std::enable_if<std::is_same<TArgument, bool>::value>::type>
{
	static std::string Unparse(const TArgument& input)
	{
		if (input == true)
		{
			return "true";
		}
		else
		{
			return "false";
		}
	}

	static bool Parse(const std::string& input, TArgument* out)
	{
		const char* inputPtr = input.c_str();

		bool retBool = false;

		if (_stricmp(inputPtr, "TRUE") == 0)
		{
			retBool = true;
		}
		else if (_stricmp(inputPtr, "FALSE") == 0)
		{
			retBool = false;
		}
		else
		{
			// If the boolean declaration is not a recognized string, then just check whether its integral.
			try
			{
				retBool = (std::stoull(input) != 0);
			}
			catch (...)
			{
				// Just do nothing.
				// Assume that the best default is already written to retBool.
			}
		}

		*out = retBool;
		return true;
	}
};

template <typename TArgument>
struct ConsoleArgumentType<TArgument, std::enable_if_t<std::is_integral<TArgument>::value && !std::is_same<TArgument, bool>::value>>
{
	static std::string Unparse(const TArgument& input)
	{
		return std::to_string(input);
	}

	static bool Parse(const std::string& input, TArgument* out)
	{
		try
		{
			*out = (TArgument)std::stoull(input);

			// no way to know if an integer is valid this lazy way, sadly :(
			return true;
		}
		catch (...)
		{
			return false;
		}
	}
};

template <typename TArgument>
struct ConsoleArgumentType<TArgument, std::enable_if_t<std::is_floating_point<TArgument>::value>>
{
	static std::string Unparse(const TArgument& input)
	{
		return std::to_string(input);
	}

	static bool Parse(const std::string& input, TArgument* out)
	{
		try
		{
			*out = (TArgument)std::stod(input);

			return true;
		}
		catch (...)
		{
			return false;
		}
	}
};

class ExternalContext : public std::any
{
public:
	ExternalContext(const std::any& any)
		: std::any(any)
	{
		
	}
};

namespace internal
{
template <typename TArgument>
inline bool ParseArgument(const std::string& input, TArgument* out)
{
	return ConsoleArgumentType<TArgument>::Parse(input, out);
}

template <typename TArgument>
inline bool ParseArgument(const ConsoleExecutionContext& context, int argInput, TArgument* out)
{
	return ConsoleArgumentType<TArgument>::Parse(context.arguments[argInput], out);
}

template<>
inline bool ParseArgument<ProgramArguments>(const ConsoleExecutionContext& context, int argInput, ProgramArguments* out)
{
	size_t numRemaining = context.arguments.Count() - argInput;
	std::vector<std::string> argList(numRemaining);

	for (int arg = 0; arg < numRemaining; arg++)
	{
		argList[arg] = context.arguments[arg + argInput];
	}

	*out = ProgramArguments{ argList };

	return true;
}

template<>
inline bool ParseArgument<ExternalContext>(const ConsoleExecutionContext& context, int argInput, ExternalContext* out)
{
	*out = ExternalContext{ context.contextRef };
	return true;
}

template <typename TArgument, int Iterator>
constexpr int AddOneArg = Iterator + 1;

template <int Iterator>
int AddOneArg<ExternalContext, Iterator> = Iterator;

template <typename TArgument>
std::string UnparseArgument(const TArgument& input)
{
	return ConsoleArgumentType<TArgument>::Unparse(input);
}

template <class TFunc>
struct ConsoleCommandFunction
{
};

template <typename... Args>
struct ConsoleCommandFunction<std::function<void(Args...)>>
{
	using TFunc = std::function<void(Args...)>;

	using ArgTuple = std::tuple<Args...>;

	static bool Call(TFunc func, ConsoleExecutionContext& context)
	{
		// check if the argument count matches
		if constexpr (NeedsArgumentCheck())
		{
			if (sizeof...(Args) != context.arguments.Count())
			{
				context.errorBuffer << "Argument count mismatch (passed " << std::to_string(context.arguments.Count()) << ", wanted " << std::to_string(sizeof...(Args)) << ")" << std::endl;
				return false;
			}
		}

		// invoke the recursive template argument tree for parsing our arguments
		return CallInternal<0, 0, std::tuple<>>(func, context, std::tuple<>());
	}

private:
	inline constexpr static bool NeedsArgumentCheck()
	{
		if constexpr (sizeof...(Args) > 0)
		{
			if constexpr (std::is_same_v<std::decay_t<std::tuple_element_t<sizeof...(Args) - 1, ArgTuple>>, ProgramArguments>)
			{
				return false;
			}
		}

		return true;
	}

public:

	// non-terminator iterator
	template <size_t Iterator, size_t ArgIterator, typename TupleType>
	static std::enable_if_t<(Iterator < sizeof...(Args)), bool> CallInternal(TFunc func, ConsoleExecutionContext& context, TupleType tuple)
	{
		// the type of the current argument
		using ArgType = std::tuple_element_t<Iterator, ArgTuple>;

		std::decay_t<ArgType> argument;
		if (ParseArgument(context, ArgIterator, &argument))
		{
			return CallInternal<Iterator + 1, AddOneArg<ArgType, ArgIterator>>(
			    func,
			    context,
			    std::tuple_cat(std::move(tuple), std::forward_as_tuple(std::forward<ArgType>(argument))));
		}

		context.errorBuffer << "Could not convert argument " << std::to_string(Iterator) << " (" << context.arguments[Iterator] << ") to " << typeid(ArgType).name() << std::endl;

		return false;
	}

	// terminator
	template <size_t Iterator, size_t ArgIterator, typename TupleType>
	static std::enable_if_t<(Iterator == sizeof...(Args)), bool> CallInternal(TFunc func, ConsoleExecutionContext& context, TupleType tuple)
	{
		apply(func, std::move(tuple));

		return true;
	}
};
}

DECLARE_INSTANCE_TYPE(ConsoleCommandManager);
