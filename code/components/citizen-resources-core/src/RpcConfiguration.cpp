#include <StdInc.h>
#include <RpcConfiguration.h>

#include <Error.h>

#include <VFSManager.h>

RpcConfiguration::RpcConfiguration()
{

}

RpcConfiguration::~RpcConfiguration()
{

}

inline RpcConfiguration::RpcType ParseRpcType(std::string_view str)
{
	if (str == "entity")
	{
		return RpcConfiguration::RpcType::EntityCreate;
	}
	else if (str == "ctx")
	{
		return RpcConfiguration::RpcType::EntityContext;
	}
	else if (str == "object")
	{
		return RpcConfiguration::RpcType::ObjectCreate;
	}
	else
	{
		FatalError("Unknown RPC type %s", std::string(str));
	}
}

inline RpcConfiguration::ArgumentType ParseContextType(std::string_view str)
{
	if (str == "BOOL")
	{
		return RpcConfiguration::ArgumentType::Bool;
	}
	else if (str == "Entity")
	{
		return RpcConfiguration::ArgumentType::Entity;
	}
	else if (str == "Hash")
	{
		return RpcConfiguration::ArgumentType::Hash;
	}
	else if (str == "Player")
	{
		return RpcConfiguration::ArgumentType::Player;
	}
	else if (str == "int" || str == "Any")
	{
		return RpcConfiguration::ArgumentType::Int;
	}
	else if (str == "charPtr")
	{
		return RpcConfiguration::ArgumentType::String;
	}
	else if (str == "float")
	{
		return RpcConfiguration::ArgumentType::Float;
	}
	else if (str == "ObjRef")
	{
		return RpcConfiguration::ArgumentType::ObjRef;
	}
	else if (str == "ObjDel")
	{
		return RpcConfiguration::ArgumentType::ObjDel;
	}
	else if (str == "Vector3")
	{
		return RpcConfiguration::ArgumentType::Vector3;
	}
	else
	{
		FatalError("Unknown RPC argument type %s", std::string(str));
	}
}

RpcConfiguration::Argument::Argument()
	: m_translate(false)
{

}

void RpcConfiguration::Argument::Initialize(rapidjson::Value& value)
{
	m_type = ParseContextType(value["type"].GetString());

	if (value.HasMember("translate"))
	{
		m_translate = value["translate"].GetBool();
	}
}

RpcConfiguration::Getter::Getter()
	: m_returnArgStart(0), m_returnType(ArgumentType::Int)
{

}

void RpcConfiguration::Getter::Initialize(rapidjson::Value& value)
{
	m_name = value["name"].GetString();
	m_returnType = ParseContextType(value["returnType"].GetString());
	m_returnArgStart = value["returnArgStart"].GetInt();
}

RpcConfiguration::Native::Native()
{

}

void RpcConfiguration::Native::Initialize(rapidjson::Value& value)
{
	m_name = value["name"].GetString();
	m_gameHash = strtoull(value["hash"].GetString() + 2, nullptr, 16);

	m_rpcType = ParseRpcType(value["type"].GetString());

	if (m_rpcType == RpcType::EntityContext)
	{
		m_contextType = ParseContextType(value["ctx"]["type"].GetString());
		m_contextArgument = value["ctx"]["idx"].GetInt();
	}

	auto& args = value["args"];

	for (auto it = args.Begin(); it != args.End(); ++it)
	{
		Argument arg;
		arg.Initialize(*it);

		m_arguments.push_back(arg);
	}

	if (value.HasMember("getter"))
	{
		auto& getter = value["getter"];

		if (getter.IsObject())
		{
			Getter getterObj;
			getterObj.Initialize(getter);

			m_getter = std::move(getterObj);
		}
	}
}

std::shared_ptr<RpcConfiguration> RpcConfiguration::Load(std::string_view path)
{
	auto fileStream = vfs::OpenRead(std::string(path));

	if (!fileStream.GetRef())
	{
		return {};
	}

	auto data = fileStream->ReadToEnd();

	rapidjson::Document document;
	document.Parse(reinterpret_cast<char*>(data.data()), data.size());

	if (document.HasParseError())
	{
		return {};
	}

	auto config = std::make_shared<RpcConfiguration>();

	if (document.IsArray())
	{
		for (auto it = document.Begin(); it != document.End(); ++it)
		{
			if (it->IsObject())
			{
				auto native = std::make_shared<Native>();
				native->Initialize(*it);

				config->m_natives.push_back(native);
			}
		}
	}

	return config;
}
