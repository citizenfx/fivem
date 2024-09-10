#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "se/Security.h"

TEST_CASE("Principal test")
{
	WHEN("A access control entry for everyone is added")
	{
		seGetCurrentContext()->AddAccessControlEntry(
			se::Principal{"builtin.everyone"},
			se::Object{"forall"},
			se::AccessType::Allow
		);
		THEN("It does not require any principal inside the scope")
		{
			REQUIRE(
				seGetCurrentContext()->CheckPrivilege(
					se::Principal{ "builtin.everyone" },
					se::Object{ "forall" }
				) == true);
		}
		AND_WHEN("The principal that contains the privilege does not matter")
		{
			THEN("The shortcut can be used that does not require specifying the principal")
			{
				REQUIRE(seCheckPrivilege("forall") == true);
			}
		}
	}

	WHEN("The AddAccessControlEntry adds the command.test privilege to the system.extConsole principal")
	{
		seGetCurrentContext()->AddAccessControlEntry(
			se::Principal{"system.extConsole"},
			se::Object{"command.test"},
			se::AccessType::Allow
		);
		AND_WHEN("The system.extConsole principal is existing on the scope")
		{
			se::ScopedPrincipal principal{
				se::Principal{
					"system.extConsole"
				}
			};
			THEN("The principal privilege is available")
			{
				REQUIRE(
					seGetCurrentContext()->CheckPrivilege(
						se::Principal{ "system.extConsole" },
						se::Object{ "command.test" }
					) == true);
				THEN("Also available without explicitly knowning the Principal")
				{
					REQUIRE(seCheckPrivilege("command.test") == true);
				}
			}
		}
	}
}

class Player : public se::PrincipalSource
{
public:
	Player()
	{
	}

	virtual ~Player()
	{
	}

	void AddPrincipal(std::string principal)
	{
		m_principals.emplace_back(principal);
	}

	void ClearPrincipals()
	{
		m_principals.clear();
	}

	void GetPrincipals(const std::function<bool(const se::Principal&)>& iterator) override
	{
		for (auto& principal : m_principals)
		{
			if (iterator(principal))
			{
				break;
			}
		}
	}

	inline auto EnterPrincipalScope()
	{
		auto principal = std::make_unique<se::ScopedPrincipal>(this);

		return std::move(principal);
	}

private:
	std::list<se::Principal> m_principals;
};

TEST_CASE("PrincipalSource test")
{
	GIVEN("A admin principal with access to the command.test privilege")
	{
		seGetCurrentContext()->AddAccessControlEntry(
			se::Principal{"admin"},
			se::Object{"command.test"},
			se::AccessType::Allow
		);
		GIVEN("A player that has the admin principal")
		{
			Player player;
			player.AddPrincipal("admin");
			WHEN("We have no access to the privilege")
			{
				REQUIRE(seCheckPrivilege("command.test") == false);
				THEN("After creating the scope we have access")
				{
					auto principalScope = player.EnterPrincipalScope();
					REQUIRE(seCheckPrivilege("command.test") == true);
					AND_WHEN("The pricipal got revoked")
					{
						player.ClearPrincipals();
						THEN("The access is no longer available")
						{
							REQUIRE(seCheckPrivilege("command.test") == false);
						}
					}
				}
			}
		}
	}
}

TEST_CASE("ScopedPrincipalReset test")
{
	GIVEN("A admin principal with access to the command.test privilege and builtin.everyone has access to command.chat")
	{
		seGetCurrentContext()->AddAccessControlEntry(
			se::Principal{"admin"},
			se::Object{"command.banall"},
			se::AccessType::Allow
		);
		seGetCurrentContext()->AddAccessControlEntry(
			se::Principal{"builtin.everyone"},
			se::Object{"command.chat"},
			se::AccessType::Allow
		);
		se::ScopedPrincipal adminPrincipal{
			se::Principal{
				"admin"
			}
		};
		REQUIRE(seCheckPrivilege("command.banall") == true);
		REQUIRE(seCheckPrivilege("command.chat") == true);
		GIVEN("A ScopedPrincipalReset")
		{
			se::ScopedPrincipalReset reset;
			THEN("The admin principal is no longer available")
			{
				REQUIRE(seCheckPrivilege("command.banall") == false);
			}
			THEN("The builtin.everyone is still available, because it can not be taken away")
			{
				REQUIRE(seCheckPrivilege("command.chat") == true);
			}
		}
	}
}

TEST_CASE("Object parents")
{
	GIVEN("A admin principal with access to the command privilege")
	{
		seGetCurrentContext()->AddAccessControlEntry(
			se::Principal{"admin"},
			se::Object{"command"},
			se::AccessType::Allow
		);
		se::ScopedPrincipal adminPrincipal{
			se::Principal{
				"admin"
			}
		};
		THEN("It also has access to any child objects from command")
		{
			REQUIRE(seCheckPrivilege("command.banall") == true);
			REQUIRE(seCheckPrivilege("command.chat") == true);	
		}
	}
	GIVEN("There is no limit of levels")
	{
		seGetCurrentContext()->AddAccessControlEntry(
			se::Principal{"owner"},
			se::Object{"command.admins"},
			se::AccessType::Allow
		);
		seGetCurrentContext()->AddAccessControlEntry(
			se::Principal{"mod"},
			se::Object{"command.mods"},
			se::AccessType::Allow
		);
		se::ScopedPrincipal ownerPrincipal{
			se::Principal{
				"owner"
			}
		};
		se::ScopedPrincipal modPrincipal{
			se::Principal{
				"mod"
			}
		};
		THEN("It also has access to any child objects from command")
		{
			REQUIRE(seCheckPrivilege("command.mods") == true);
			REQUIRE(seCheckPrivilege("command.admins") == true);
			REQUIRE(seCheckPrivilege("command.admins.ban") == true);
			REQUIRE(seCheckPrivilege("command.mods.kick") == true);
			REQUIRE(seCheckPrivilege("command.mods.kick.all") == true);
			REQUIRE(seCheckPrivilege("command") == false);	
		}
	}
}
