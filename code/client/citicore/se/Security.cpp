#include "StdInc.h"
#include "Security.h"

#include <IteratorView.h>

#include <unordered_set>

#include <shared_mutex>
#include <stack>

#include <CoreConsole.h>

static bool g_debugSecurity;

namespace se
{
struct AccessControlEntry
{
	Object object;
	Principal principal;
	AccessType type;
};

class ContextImpl
{
public:
	std::multimap<Object, AccessControlEntry> m_aces;
	std::multimap<Principal, Principal> m_principalInheritance;

	std::shared_mutex m_mutex;

public:
	template<typename TSet>
	void AddPrincipals(const Principal& principal, TSet& set)
	{
		for (const auto& parenting : fx::GetIteratorView(m_principalInheritance.equal_range(principal)))
		{
			if (set.find(parenting.second) == set.end())
			{
				set.insert(parenting.second);

				AddPrincipals(parenting.second, set);
			}
		}
	}
};

static thread_local std::deque<std::reference_wrapper<Principal>> g_principalStack;

Context::Context()
{
	m_impl = std::make_unique<ContextImpl>();
}

void Context::Reset()
{
	// clear all ACEs
	m_impl->m_aces.clear();

	// clear all inheritances
	m_impl->m_principalInheritance.clear();
}

void Context::LoadSnapshot(const std::string& snapshot)
{
	// TODO: load snapshot (used for server->client comms)
}

std::string Context::SaveSnapshot()
{
	// TODO: save snapshot
	return "";
}

void Context::AddPrincipalInheritance(const Principal& child, const Principal& parent)
{
	m_impl->m_principalInheritance.insert({ child, parent });
}

void Context::RemovePrincipalInheritance(const Principal& child, const Principal& parent)
{
	for (auto it = m_impl->m_principalInheritance.begin(); it != m_impl->m_principalInheritance.end(); )
	{
		if (it->first == child && it->second == parent)
		{
			it = m_impl->m_principalInheritance.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Context::AddAccessControlEntry(const Principal& principal, const Object& object, AccessType type)
{
	m_impl->m_aces.insert({ object, { object, principal, type } });
}

void Context::RemoveAccessControlEntry(const Principal& principal, const Object& object, AccessType type)
{
	for (auto it = m_impl->m_aces.begin(); it != m_impl->m_aces.end(); )
	{
		if (it->first == object && it->second.principal == principal && it->second.type == type)
		{
			it = m_impl->m_aces.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Context::ForAllPrincipalInheritances(const std::function<void(const Principal &, const Principal &)>& cb)
{
	for (auto& pce : m_impl->m_principalInheritance)
	{
		cb(pce.first, pce.second);
	}
}

void Context::ForAllAccessControlEntries(const std::function<void(const Principal &, const Object &, AccessType)>& cb)
{
	for (auto& ace : m_impl->m_aces)
	{
		cb(ace.second.principal, ace.second.object, ace.second.type);
	}
}

bool Context::CheckPrivilege(const Object& object)
{
	if (g_principalStack.empty())
	{
		return CheckPrivilege(Principal{ "builtin.everyone" }, object);
	}

	for (auto& entry : g_principalStack)
	{
		if (CheckPrivilege(entry, object))
		{
			return true;
		}
	}

	return false;
}

bool Context::CheckPrivilege(const Principal& principal, const Object& object)
{
	bool allowed = false;

	// get a set of principals this principal equates to
	std::set<Principal> principals = { principal, Principal{ "builtin.everyone" } };

	m_impl->AddPrincipals(principal, principals);

	// split the object by parents
	// this makes 'object.parent.child' turn into a set of ['object.parent.child', 'object.parent', 'object']
	std::vector<Object> objects = { object };

	{
		std::string tempObject = object.GetIdentifier();

		while (tempObject.find('.') != std::string::npos)
		{
			tempObject = tempObject.substr(0, tempObject.find_last_of('.'));

			objects.push_back(Object{ tempObject });
		}
	}

	// loop through each ACE for each object
	for (const auto& objectRef : objects)
	{
		for (const auto& ace : fx::GetIteratorView(m_impl->m_aces.equal_range(objectRef)))
		{
			// get entry
			const auto& entry = ace.second;

			// if this concerns one of our principals
			if (principals.find(entry.principal) != principals.end())
			{
				// if allow, assume we're allowed
				if (entry.type == AccessType::Allow)
				{
					if (g_debugSecurity)
					{
						console::DPrintf("security", "TEST ACL [%s -> %s] ACE [%s %s] -> ALLOW\n",
							principal.GetIdentifier(), object.GetIdentifier(), entry.principal.GetIdentifier(), entry.object.GetIdentifier());
					}

					allowed = true;
				}
				else if (entry.type == AccessType::Deny)
				{
					if (g_debugSecurity)
					{
						console::DPrintf("security", "TEST ACL [%s -> %s] ACE [%s %s] -> DENY-ALWAYS\n",
							principal.GetIdentifier(), object.GetIdentifier(), entry.principal.GetIdentifier(), entry.object.GetIdentifier());

						console::Printf("security", "TEST ACL [%s -> %s] -> DENY-ALWAYS\n", principal.GetIdentifier(), object.GetIdentifier());
					}

					// deny trumps all
					return false;
				}
			}
		}
	}

	// trace log
	if (g_debugSecurity)
	{
		console::Printf("security", "TEST ACL [%s -> %s] -> %s\n", principal.GetIdentifier(), object.GetIdentifier(), (allowed) ? "ALLOW" : "DENY");
	}

	return allowed;
}

void Context::PushPrincipal(Principal& principal)
{
	g_principalStack.push_front(principal);
}

void Context::PopPrincipal()
{
	g_principalStack.pop_front();
}

static std::stack<std::deque<std::reference_wrapper<Principal>>> g_principalStackStack;

void Context::PushPrincipalReset()
{
	g_principalStackStack.push(g_principalStack);
	g_principalStack.clear();
}

void Context::PopPrincipalReset()
{
	g_principalStack = g_principalStackStack.top();
	g_principalStackStack.pop();
}

static thread_local Context* g_currentContext;
static Context* g_globalContext;

void Context::MakeCurrent()
{
	g_currentContext = this;
}

Context::~Context()
{

}
}

extern "C" se::Context* seGetCurrentContext()
{
	{
		static std::once_flag initFlag;

		std::call_once(initFlag, []()
		{
			static ConVar<bool> seDebugConvar(console::GetDefaultContext(), "se_debug", ConVar_None, false, &g_debugSecurity);

#ifdef IS_FXSERVER
			static ConsoleCommand addAclCmd("add_ace", [](const std::string& principal, const std::string& object, const std::string& allow)
			{
				se::AccessType type;

				if (allow == "allow")
				{
					type = se::AccessType::Allow;
				}
				else if (allow == "deny")
				{
					type = se::AccessType::Deny;
				}
				else
				{
					console::Printf("security", "Access type needs to be 'allow' or 'deny'.");
					return;
				}

				seGetCurrentContext()->AddAccessControlEntry(se::Principal{ principal }, se::Object{ object }, type);
			});

			static ConsoleCommand addPrincipalCmd("add_principal", [](const std::string& principal, const std::string& parent)
			{
				seGetCurrentContext()->AddPrincipalInheritance(se::Principal{ principal }, se::Principal{ parent });
			});

			static ConsoleCommand removeAclCmd("remove_ace", [](const std::string& principal, const std::string& object, const std::string& allow)
			{
				se::AccessType type;

				if (allow == "allow")
				{
					type = se::AccessType::Allow;
				}
				else if (allow == "deny")
				{
					type = se::AccessType::Deny;
				}
				else
				{
					console::Printf("security", "Access type needs to be 'allow' or 'deny'.");
					return;
				}

				seGetCurrentContext()->RemoveAccessControlEntry(se::Principal{ principal }, se::Object{ object }, type);
			});

			static ConsoleCommand removePrincipalCmd("remove_principal", [](const std::string& principal, const std::string& parent)
			{
				seGetCurrentContext()->RemovePrincipalInheritance(se::Principal{ principal }, se::Principal{ parent });
			});
#endif

			static ConsoleCommand testAceCmd("test_ace", [](const std::string& principal, const std::string& object)
			{
				console::Printf("security", "%s -> %s = %s\n", 
					principal,
					object,
					seGetCurrentContext()->CheckPrivilege(se::Principal{ principal }, se::Object{ object }) ? "true" : "false");
			});

			static ConsoleCommand listPrincipalCmd("list_principals", []()
			{
				seGetCurrentContext()->ForAllPrincipalInheritances([](const se::Principal& child, const se::Principal& parent)
				{
					console::Printf("security", "%s <- %s\n", child.GetIdentifier(), parent.GetIdentifier());
				});
			});

			static ConsoleCommand listAceCmd("list_aces", []()
			{
				seGetCurrentContext()->ForAllAccessControlEntries([](const se::Principal& principal, const se::Object& object, se::AccessType accessType)
				{
					console::Printf("security", "%s -> %s = %s\n", principal.GetIdentifier(), object.GetIdentifier(), (accessType == se::AccessType::Allow) ? "ALLOW" : "DENY");
				});
			});
		});
	}

	if (!se::g_currentContext)
	{
		if (se::g_globalContext == nullptr)
		{
			se::g_globalContext = new se::Context();
			se::g_globalContext->AddAccessControlEntry(se::Principal{ "system.console" }, se::Object{ "command" }, se::AccessType::Allow);
		}

		return se::g_globalContext;
	}

	return se::g_currentContext;
}

extern "C" void seCreateContext(fwRefContainer<se::Context>* outContext)
{
	*outContext = new se::Context();
}
