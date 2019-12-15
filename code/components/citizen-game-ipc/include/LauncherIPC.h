#pragma once

#include <nngpp/nngpp.h>
#include <nngpp/protocol/push0.h>
#include <nngpp/protocol/pull0.h>

#include <msgpack.hpp>

#include <CoreConsole.h>

namespace ipc
{
	namespace internal
	{
		template <class TFunc>
		struct HandlerFunction
		{
		};

		template <typename TArgument, int Iterator>
		constexpr int AddOneArg = Iterator + 1;

		template <int Iterator>
		int AddOneArg<ExternalContext, Iterator> = Iterator;

		template <typename TRet, typename... Args>
		struct HandlerFunction<std::function<TRet(Args...)>>
		{
			using TFunc = std::function<TRet(Args...)>;

			using ArgTuple = std::tuple<Args...>;

			static bool Call(TFunc func, const std::vector<msgpack::object>& list, msgpack::sbuffer& retBuf)
			{
				// check if the argument count matches
				if (sizeof...(Args) != list.size())
				{
					return false;
				}

				// invoke the recursive template argument tree for parsing our arguments
				return CallInternal<0, 0, std::tuple<>>(func, list, std::tuple<>(), retBuf);
			}

			// non-terminator iterator
			template <size_t Iterator, size_t ArgIterator, typename TupleType>
			static std::enable_if_t<(Iterator < sizeof...(Args)), bool> CallInternal(TFunc func, const std::vector<msgpack::object>& list, TupleType tuple, msgpack::sbuffer& retBuf)
			{
				// the type of the current argument
				using ArgType = std::tuple_element_t<Iterator, ArgTuple>;

				std::decay_t<ArgType> argument;
				if (list[ArgIterator].convert(&argument))
				{
					return CallInternal<Iterator + 1, AddOneArg<ArgType, ArgIterator>>(
						func,
						list,
						std::tuple_cat(std::move(tuple), std::forward_as_tuple(std::forward<ArgType>(argument))),
						retBuf);
				}

				return false;
			}

			// terminator
			template <size_t Iterator, size_t ArgIterator, typename TupleType>
			static std::enable_if_t<(Iterator == sizeof...(Args)), bool> CallInternal(TFunc func, const std::vector<msgpack::object>& list, TupleType tuple, msgpack::sbuffer& retBuf)
			{
				if constexpr (!std::is_same_v<TRet, void>)
				{
					auto rv = std::apply(func, std::move(tuple));
					msgpack::pack(retBuf, rv);
				}
				else
				{
					std::apply(func, std::move(tuple));
					msgpack::pack(retBuf, std::vector<int>{});
				}

				return true;
			}
		};
	}

	class
#ifdef COMPILING_CITIZEN_GAME_IPC
		DLL_EXPORT
#else
		DLL_IMPORT
#endif
		Endpoint
	{
	public:
		Endpoint(const std::string& bindingPrefix, bool server);

		void RunFrame();

		template<typename TFunc>
		inline void Bind(const std::string& name, const TFunc& func)
		{
			auto function = detail::make_function(func);

			handlers[name] = [function](const std::vector<msgpack::object>& list, msgpack::sbuffer& retBuf)
			{
				return internal::HandlerFunction<decltype(function)>::Call(function, list, retBuf);
			};
		}

		template<typename... TArg>
		inline void Call(const std::string& name, const TArg&... args)
		{
			msgpack::sbuffer buf;
			msgpack::packer<msgpack::sbuffer> packer(buf);

			// pack the argument pack as array
			packer.pack_array(2);
			packer.pack(name);

			packer.pack_array(sizeof...(args));
			pass{ (packer.pack(args), 0)... };

			Call(buf.data(), buf.size());

			//return msgpack::unpack((const char*)response.data(), response.size());
		}

		void Call(const void* data, size_t size);

	private:
		std::map<std::string, std::function<bool(const std::vector<msgpack::object>&, msgpack::sbuffer&)>, std::less<>> handlers;

		nng::socket reqSock;
		nng::socket repSock;

		struct pass
		{
			template<typename ...T> pass(T...) {}
		};
	};
}
