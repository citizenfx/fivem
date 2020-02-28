#pragma once

#include <optional>

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
		Hash,
		ObjRef,
		ObjDel,
		Vector3
	};

	enum class RpcType
	{
		EntityContext,
		EntityCreate,
		ObjectCreate
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

	class Getter
	{
	public:
		Getter();

		void Initialize(rapidjson::Value& value);

		inline const std::string& GetName() const
		{
			return m_name;
		}

		inline ArgumentType GetReturnType() const
		{
			return m_returnType;
		}

		inline int GetReturnArgStart() const
		{
			return m_returnArgStart;
		}

	private:
		std::string m_name;
		ArgumentType m_returnType;
		int m_returnArgStart;
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

		inline const std::optional<Getter>& GetGetter() const
		{
			return m_getter;
		}

	private:
		std::string m_name;
		uint64_t m_gameHash;

		int m_contextArgument;
		ArgumentType m_contextType;

		RpcType m_rpcType;

		std::vector<Argument> m_arguments;

		std::optional<Getter> m_getter;
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
