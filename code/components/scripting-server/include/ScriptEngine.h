/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_SCRIPTING_SERVER
#define SCRT_EXPORT DLL_EXPORT
#else
#define SCRT_EXPORT DLL_IMPORT
#endif

#include <boost/optional.hpp>

namespace fx
{
	class ScriptContext
	{
	public:
		enum
		{
			MaxArguments = 32,
			ArgumentSize = sizeof(void*)
		};

	protected:
		void* m_argumentBuffer;

		int m_numArguments;
		int m_numResults;

	protected:
		inline ScriptContext() = default;

	public:
		template<typename T>
		inline const T& GetArgument(int index)
		{
			auto functionData = (uintptr_t*)m_argumentBuffer;

			return *reinterpret_cast<T*>(&functionData[index]);
		}

		template<typename T>
		inline void SetArgument(int index, const T& value)
		{
			auto functionData = (uintptr_t*)m_argumentBuffer;

			static_assert(sizeof(T) <= ArgumentSize, "Argument size of T");

			if (sizeof(T) < ArgumentSize)
			{
				*reinterpret_cast<uintptr_t*>(&functionData[index]) = 0;
			}

			*reinterpret_cast<T*>(&functionData[index]) = value;
		}

		template<typename T>
		inline const T& CheckArgument(int index)
		{
			const auto& argument = GetArgument<T>(index);

			if (argument == T())
			{
				throw std::runtime_error(va("Argument at index %d was null.", index));
			}

			return argument;
		}

		inline int GetArgumentCount()
		{
			return m_numArguments;
		}

		template<typename T>
		inline void Push(const T& value)
		{
			auto functionData = (uintptr_t*)m_argumentBuffer;

			static_assert(sizeof(T) <= ArgumentSize, "Argument size of T");

			if (sizeof(T) < ArgumentSize)
			{
				*reinterpret_cast<uintptr_t*>(&functionData[m_numArguments]) = 0;
			}

			*reinterpret_cast<T*>(&functionData[m_numArguments]) = value;
			m_numArguments++;
		}

		template<typename T>
		inline void SetResult(const T& value)
		{
			auto functionData = (uintptr_t*)m_argumentBuffer;

			if (sizeof(T) < ArgumentSize)
			{
				*reinterpret_cast<uintptr_t*>(&functionData[0]) = 0;
			}

			*reinterpret_cast<T*>(&functionData[0]) = value;

			m_numResults = 1;
			m_numArguments = 0;
		}

		template<typename T>
		inline T GetResult()
		{
			auto functionData = (uintptr_t*)m_argumentBuffer;

			return *reinterpret_cast<T*>(functionData);
		}

		inline void* GetArgumentBuffer()
		{
			return m_argumentBuffer;
		}
	};

	class ScriptContextRaw : public ScriptContext
	{
	public:
		inline ScriptContextRaw(void* functionBuffer, int numArguments)
		{
			m_argumentBuffer = functionBuffer;

			m_numArguments = numArguments;
			m_numResults = 0;
		}
	};

	class ScriptContextBuffer : public ScriptContext
	{
	private:
		uint8_t m_functionData[MaxArguments][ArgumentSize];

	public:
		inline ScriptContextBuffer()
		{
			m_argumentBuffer = &m_functionData;

			m_numArguments = 0;
			m_numResults = 0;
		}
	};

	typedef std::function<void(ScriptContext&)> TNativeHandler;

	class SCRT_EXPORT ScriptEngine
	{
	public:
		static boost::optional<TNativeHandler> GetNativeHandler(uint64_t nativeIdentifier);

		static void RegisterNativeHandler(uint64_t nativeIdentifier, TNativeHandler function);

		static void RegisterNativeHandler(const std::string& nativeName, TNativeHandler function);
	};
}
