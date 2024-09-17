#include <StdInc.h>
#include "NoticeLogicProcessor.h"

fx::NoticeLogicProcessor::NoticeLogicProcessor(fx::ServerInstanceBase* server)
{
	m_cvManager = server->GetComponent<console::Context>()->GetVariableManager();
	m_resManager = server->GetComponent<fx::ResourceManager>()->GetCurrent();
}

bool fx::NoticeLogicProcessor::ProcessNoticeRule(const nlohmann::json& ruleRef, uint32_t nestingLevel) const
{
	if (nestingLevel >= 10)
	{
		throw std::invalid_argument("Maximum nesting level for notice rules was exceeded");
	}
	if (!ruleRef.is_object())
	{
		return false;
	}
	auto ruleObject = ruleRef.get<nlohmann::json::object_t>();

	auto& opObject = ruleObject["op"];
	if (!opObject.is_string())
	{
		return false;
	}

	auto mappedOp = m_ruleOpStringToEnum.find(opObject.get<std::string>());
	if (mappedOp == m_ruleOpStringToEnum.end())
	{
		return false;
	}

	auto opNum = mappedOp->second;
	switch (opNum)
	{
		case RuleOp::And:
		case RuleOp::Or:
		{
			auto& nestedRules = ruleObject["rules"];
			if (!nestedRules.is_array())
			{
				return false;
			}

			for (auto& subRule : nestedRules)
			{
				auto isSubRuleTrue = ProcessNoticeRule(subRule, nestingLevel + 1);
				if (opNum == RuleOp::And)
				{
					if (!isSubRuleTrue)
					{
						return false;
					}
				}
				else // ::Or
				{
					if (isSubRuleTrue)
					{
						return true;
					}
				}
			}

			// Final catch when all ::Ands were true or all ::Ors were false
			return opNum == RuleOp::And ? true : false;
		}
		case RuleOp::Not:
		{
			auto& ruleToInvert = ruleObject["rule"];
			return ruleToInvert.is_object() ? !ProcessNoticeRule(ruleToInvert, nestingLevel + 1) : false;
		}
		case RuleOp::NullOrEmpty:
		case RuleOp::Contains:
		case RuleOp::Equals:
		case RuleOp::GreaterThan:
		case RuleOp::GreaterThanOrEquals:
		case RuleOp::LessThan:
		case RuleOp::LessThanOrEquals:
		{
			auto& typeObject = ruleObject["type"];
			if (!typeObject.is_string())
			{
				return false;
			}

			auto mappedType = m_ruleTypeStringToEnum.find(typeObject.get<std::string>());
			if (mappedType == m_ruleTypeStringToEnum.end())
			{
				return false;
			}

			auto typeNum = mappedType->second;
			if (typeNum == RuleType::ConVar)
			{
				auto& keyObject = ruleObject["key"];
				if (!keyObject.is_string())
				{
					return false;
				}

				auto cvEntry = m_cvManager->FindEntryRaw(keyObject.get<std::string>());
				if (!cvEntry)
				{
					return opNum == RuleOp::NullOrEmpty;
				}

				auto currentValue = cvEntry->GetValue();
				if (opNum == RuleOp::NullOrEmpty)
				{
					return currentValue == "";
				}

				auto& dataObject = ruleObject["data"];
				auto treatDataAsNumber = dataObject.is_number_integer();
				// Only string and number comparands are supported
				if (!dataObject.is_string() && !treatDataAsNumber)
				{
					return false;
				}

				if (treatDataAsNumber)
				{
					auto currentValueNumber = strtol(currentValue.c_str(), nullptr, 10);
					auto comparandNumber = dataObject.get<int>();
					return PerformNumberComparison(opNum, currentValueNumber, comparandNumber);
				}
				else
				{
					switch (opNum)
					{
						case RuleOp::Contains:
							return currentValue.find(dataObject.get<std::string>()) != std::string::npos;
						case RuleOp::Equals:
							return currentValue == dataObject.get<std::string>();
						default:
							break;
					}
				}
			}
			else if (typeNum == RuleType::StartedResourceList && opNum == RuleOp::Contains)
			{
				auto& dataObject = ruleObject["data"];
				if (!dataObject.is_string())
				{
					return false;
				}

				auto resource = m_resManager->GetResource(dataObject.get<std::string>(), false);
				return (resource.GetRef() && resource->GetState() == fx::ResourceState::Started);
			}
			return false;
		}
	}
	return false;
}

void fx::NoticeLogicProcessor::BeginProcessingNotices(fx::ServerInstanceBase* server, const nlohmann::json& noticesBlob)
{
	NoticeLogicProcessor nlp(server);

	for (auto& [noticeType, data] : noticesBlob.get<nlohmann::json::object_t>())
	{
		auto& enabled = data["enabled"];
		if (!enabled.get<bool>())
		{
			continue;
		}

		auto& rootRule = data["rule"];
		auto ruleIsTrue = nlp.ProcessNoticeRule(rootRule, 0);

		if (ruleIsTrue)
		{
			auto& lines = data["notice_lines"];
			if (lines.is_array())
			{
				trace("^1-- [server notice: %s]^7\n", noticeType);
				for (auto& noticeLine : lines)
				{
					trace("%s\n", noticeLine.get<std::string>());
				}
			}
		}
	}
}

bool fx::NoticeLogicProcessor::PerformNumberComparison(RuleOp operation, int currentValue, int comparand)
{
	switch (operation)
	{
	case RuleOp::Equals:
		return currentValue == comparand;
	case RuleOp::GreaterThan:
		return currentValue > comparand;
	case RuleOp::GreaterThanOrEquals:
		return currentValue >= comparand;
	case RuleOp::LessThan:
		return currentValue < comparand;
	case RuleOp::LessThanOrEquals:
		return currentValue <= comparand;
	default:
		return false;
	}
}
