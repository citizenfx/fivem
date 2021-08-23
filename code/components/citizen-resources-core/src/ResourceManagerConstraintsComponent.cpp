#include <StdInc.h>

#include <ResourceManager.h>
#include <ResourceManagerConstraintsComponent.h>

#include <regex>
#include <variant>

namespace fx
{
class ResourceManagerConstraintsComponentImpl : public ResourceManagerConstraintsComponent
{
public:
	virtual void SetEnforcingConstraints(bool enabled) override
	{
		m_enforcingConstraints = enabled;
	}

	virtual void RegisterConstraint(const std::string& key, const std::string& value) override
	{
		m_constraints[key] = value;
	}

	virtual void RegisterConstraint(const std::string& key, int value) override
	{
		m_constraints[key] = value;
	}

	virtual void RegisterConstraint(const std::string& key, bool value) override
	{
		m_constraints[key] = value;
	}

	virtual void RegisterConstraint(const std::string& key, const std::function<bool(std::string_view, std::string*)>& value) override
	{
		m_constraints[key] = value;
	}

	virtual ConstraintMatchResult MatchConstraint(const std::string& constraintExpression, std::string* error) override
	{
		auto fail = [this, &error](const std::string& message)
		{
			if (error)
			{
				*error = message;
			}

			return (m_enforcingConstraints) ? ConstraintMatchResult::Fail : ConstraintMatchResult::Pass;
		};

		if (constraintExpression.empty() || constraintExpression[0] != '/')
		{
			return ConstraintMatchResult::NoConstraint;
		}

		static auto constraintRe = std::regex(R"(^/([a-z/0-9_-]+)(?::([a-z/0-9_-]+))?$)", std::regex_constants::ECMAScript | std::regex_constants::icase);
		std::smatch mr;
		if (!std::regex_match(constraintExpression, mr, constraintRe))
		{
			return fail("constraint syntax error");
		}

		// find the source constraint
		const auto constraintIt = m_constraints.find(mr[1]);
		if (constraintIt == m_constraints.end())
		{
			return fail(fmt::sprintf("constraint '%s' is unknown", mr[1]));
		}

		const auto& constraint = constraintIt->second;

		// get the constraint value
		std::string constraintValue;

		if (mr.size() >= 3)
		{
			constraintValue = mr[2];
		}

		// try matching the constraint
		std::string errorBit = "no details specified";

		// if no value, we pass for int/string constraints, where bool/callback constraints get polled anyway
		if (constraintValue.empty())
		{
			if (constraint.index() == 1)
			{
				if (!std::get<1>(constraint))
				{
					return fail(fmt::sprintf("%s isn't enabled", mr[1]));
				}
			}
			else if (constraint.index() == 3)
			{
				if (!std::get<3>(constraint)(constraintValue, &errorBit))
				{
					return fail(fmt::sprintf("%s: ^1%s^7", mr[1], errorBit));
				}
			}

			return ConstraintMatchResult::Pass;
		}
		else
		{
			switch (constraint.index())
			{
			case 0:
				if (std::get<0>(constraint) < atoi(constraintValue.c_str()))
				{
					return fail(fmt::sprintf("%s needs to be %d or higher", mr[1], atoi(constraintValue.c_str())));
				}

				break;
			case 1:
				if (std::get<1>(constraint) && constraintValue == "false")
				{
					return fail(fmt::sprintf("%s needs to be disabled", mr[1]));
				}
				else if (!std::get<1>(constraint) && constraintValue == "true")
				{
					return fail(fmt::sprintf("%s needs to be enabled", mr[1]));
				}
				else if (constraintValue != "true" && constraintValue != "false")
				{
					return fail(fmt::sprintf("%s isn't a boolean", constraintValue));
				}

				break;
			case 2:
				if (std::get<2>(constraint) != constraintValue)
				{
					return fail(fmt::sprintf("%s needs to be %s", mr[1], constraintValue));
				}

				break;
			case 3:
				if (!std::get<3>(constraint)(constraintValue, &errorBit))
				{
					return fail(fmt::sprintf("%s(%s): ^1%s^7", mr[1], constraintValue, errorBit));
				}

				break;
			}

			return ConstraintMatchResult::Pass;
		}
	}

private:
	std::unordered_map<std::string, std::variant<int, bool, std::string, std::function<bool(std::string_view, std::string*)>>> m_constraints;

	bool m_enforcingConstraints = false;
};
}

static InitFunction initFunction([]()
{
	static std::multimap<std::string, std::string> resourceDependencies;
	static std::multimap<std::string, std::string> resourceDependants;

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* resman)
	{
		resman->SetComponent<fx::ResourceManagerConstraintsComponent>(new fx::ResourceManagerConstraintsComponentImpl());
	});
});
