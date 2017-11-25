#pragma once

#include <rapidjson/document.h>

#ifdef COMPILING_CITIZEN_RESOURCES_CORE
#define RESOURCES_CORE_EXPORT DLL_EXPORT
#else
#define RESOURCES_CORE_EXPORT DLL_IMPORT
#endif

class RESOURCES_CORE_EXPORT RpcConfiguration
{
public:
	enum class ArgumentType
	{
		Entity,
		Float,
		Int,
		Bool,
		String,
		Player,
		Hash
	};

	enum class RpcType
	{
		EntityContext,
		EntityCreate
	};

	class Argument
	{
	public:
		Argument();

		void Initialize(rapidjson::Value& value);

		inline ArgumentType GetType() const
		{
			return m_type;
		}

		inline bool GetTranslated() const
		{
			return m_translate;
		}

	private:
		ArgumentType m_type;
		bool m_translate;
	};

	class Native
	{
	public:
		Native();

		void Initialize(rapidjson::Value& value);

		inline const std::string& GetName() const
		{
			return m_name;
		}

		inline uint64_t GetGameHash() const
		{
			return m_gameHash;
		}

		inline int GetContextIndex() const
		{
			return m_contextArgument;
		}

		inline ArgumentType GetContextType() const
		{
			return m_contextType;
		}

		inline RpcType GetRpcType() const
		{
			return m_rpcType;
		}

		inline const std::vector<Argument>& GetArguments() const
		{
			return m_arguments;
		}

	private:
		std::string m_name;
		uint64_t m_gameHash;

		int m_contextArgument;
		ArgumentType m_contextType;

		RpcType m_rpcType;

		std::vector<Argument> m_arguments;
	};

public:
	RpcConfiguration();

	virtual ~RpcConfiguration();

	inline const std::vector<std::shared_ptr<Native>>& GetNatives() const
	{
		return m_natives;
	}

	static std::shared_ptr<RpcConfiguration> Load(std::string_view path);

private:
	std::vector<std::shared_ptr<Native>> m_natives;
};
