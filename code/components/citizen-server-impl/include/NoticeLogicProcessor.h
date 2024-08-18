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
		Equals,
		LessThan,
		LessThanOrEquals,
		GreaterThan,
		GreaterThanOrEquals
	};

	enum RuleType
	{
		ConVar,
		StartedResourceList
	};

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
		StrEnumMapEntry(RuleOp, Equals),
		StrEnumMapEntry(RuleOp, LessThan),
		StrEnumMapEntry(RuleOp, LessThanOrEquals),
		StrEnumMapEntry(RuleOp, GreaterThan),
		StrEnumMapEntry(RuleOp, GreaterThanOrEquals)
	};

	const std::map<std::string, RuleType> m_ruleTypeStringToEnum{
		StrEnumMapEntry(RuleType, ConVar),
		StrEnumMapEntry(RuleType, StartedResourceList),
	};

	ConsoleVariableManager* m_cvManager;
	fx::ResourceManager* m_resManager;

public:
	NoticeLogicProcessor(fx::ServerInstanceBase* server);

	bool ProcessNoticeRule(const nlohmann::json& ruleRef, uint32_t nestingLevel) const;
	static void BeginProcessingNotices(fx::ServerInstanceBase* server, const nlohmann::json& noticesBlob);

private:
	static bool PerformNumberComparison(RuleOp operation, int currentValue, int comparand);
};
}
