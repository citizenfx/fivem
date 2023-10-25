#pragma once

#include <StdInc.h>
#include <ServerInstanceBase.h>
#include <ResourceManager.h>

#include <json.hpp>

namespace fx
{
class NoticeLogicProcessor
{
private:
	enum RuleOp
	{
		And,
		Or,
		Not,
		NullOrEmpty,
		Contains,
		Equals
	};

	enum RuleType
	{
		ConVar,
		StartedResourceList
	};

public:
	NoticeLogicProcessor(fx::ServerInstanceBase* server);
	bool ProcessNoticeRule(const nlohmann::json& ruleRef, uint32_t nestingLevel) const;

	static void BeginProcessingNotices(fx::ServerInstanceBase* server, const nlohmann::json& noticesBlob);

private:
	ConsoleVariableManager* m_cvManager;
	fx::ResourceManager* m_resManager;

	// Macro'd map entries to prevent accidental typos or bad mapping
#define StrEnumMapEntry(EnumName, ValueName) \
	{                                        \
		#ValueName, EnumName::ValueName      \
	}

	const std::map<std::string, RuleOp> m_ruleOpStringToEnum{
		StrEnumMapEntry(RuleOp, And),
		StrEnumMapEntry(RuleOp, Or),
		StrEnumMapEntry(RuleOp, Not),
		StrEnumMapEntry(RuleOp, NullOrEmpty),
		StrEnumMapEntry(RuleOp, Contains),
		StrEnumMapEntry(RuleOp, Equals)
	};

	const std::map<std::string, RuleType> m_ruleTypeStringToEnum{
		StrEnumMapEntry(RuleType, ConVar),
		StrEnumMapEntry(RuleType, StartedResourceList),
	};
};
}
