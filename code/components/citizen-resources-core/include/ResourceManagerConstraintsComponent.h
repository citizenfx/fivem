#pragma once

#ifdef COMPILING_CITIZEN_RESOURCES_CORE
#define RESOURCES_CORE_EXPORT DLL_EXPORT
#else
#define RESOURCES_CORE_EXPORT DLL_IMPORT
#endif

namespace fx
{
enum class ConstraintMatchResult
{
	// Passed, continue execution
	Pass,

	// Failed, error out
	Fail,

	// Not a constraint, use standard evaluation rules
	NoConstraint
};

class RESOURCES_CORE_EXPORT ResourceManagerConstraintsComponent : public fwRefCountable
{
public:
	// Enables constraint enforcement if `true`.
	virtual void SetEnforcingConstraints(bool enabled) = 0;

	// Registers a string constraint.
	virtual void RegisterConstraint(const std::string& key, const std::string& value) = 0;

	// Registers an integer constraint.
	virtual void RegisterConstraint(const std::string& key, int value) = 0;

	// Registers a flag constraint.
	virtual void RegisterConstraintBool(const std::string& key, bool value) = 0;

	// Registers a callback constraint.
	virtual void RegisterConstraint(const std::string& key, const std::function<bool(std::string_view, std::string*)>& value) = 0;

	// Tests a constraint string and returns a result.
	virtual ConstraintMatchResult MatchConstraint(const std::string& constraintExpression, std::string* error) = 0;
};
}

DECLARE_INSTANCE_TYPE(fx::ResourceManagerConstraintsComponent);
