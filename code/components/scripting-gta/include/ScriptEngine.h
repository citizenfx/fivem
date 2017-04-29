/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_SCRIPTING_GTA
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

	private:
		uint8_t m_functionData[MaxArguments][ArgumentSize];

		int m_numArguments;
		int m_numResults;
		
	public:
		inline ScriptContext()
		{
			m_numArguments = 0;
			m_numResults = 0;
		}

		template<typename T>
		inline const T& GetArgument(int index)
		{
			return *reinterpret_cast<T*>(&m_functionData[index][0]);
		}

		inline int GetArgumentCount()
		{
			return m_numArguments;
		}

		template<typename T>
		inline void Push(const T& value)
		{
			assert(sizeof(T) <= ArgumentSize);

			if (sizeof(T) < ArgumentSize)
			{
				*reinterpret_cast<uintptr_t*>(&m_functionData[m_numArguments][0]) = 0;
			}

			*reinterpret_cast<T*>(&m_functionData[m_numArguments][0]) = value;
			m_numArguments++;
		}

		template<typename T>
		inline void SetResult(const T& value)
		{
			if (sizeof(T) < ArgumentSize)
			{
				*reinterpret_cast<uintptr_t*>(&m_functionData[0][0]) = 0;
			}

			*reinterpret_cast<T*>(&m_functionData[0][0]) = value;

			m_numResults = 1;
			m_numArguments = 0;
		}

		template<typename T>
		inline T GetResult()
		{
			return *reinterpret_cast<T*>(m_functionData);
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