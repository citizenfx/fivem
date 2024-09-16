#pragma once
#include "ConVars.h"
#include "PacketHandler.h"

#include <msgpack.hpp>

#include <se/Security.h>

namespace fx
{
class ConVarsPacketHandler : public net::PacketHandler<net::packet::ServerConVars, HashRageString("msgConVars")>
{
	std::vector<std::string> m_convarsCreatedByServer;
	std::vector<std::string> m_convarsModifiedByServer;
public:
	ConVarsPacketHandler()
	{
		OnKillNetworkDone.Connect([this]
		{
			// Remove ConVars created by the server
			for (const std::string& convarName : m_convarsCreatedByServer)
			{
				console::GetDefaultContext()->GetVariableManager()->Unregister(convarName);
			}

			se::ScopedPrincipal principalScopeInternal(se::Principal{ "system.internal" });
			auto varManager = console::GetDefaultContext()->GetVariableManager();

			// Revert values modified by server back to their old values
			for (const std::string& convarName : m_convarsModifiedByServer)
			{
				if (auto convar = varManager->FindEntryRaw(convarName))
				{
					// Restore flags to default state
					auto defaultFlags = varManager->GetEntryDefaultFlags(convarName);
					if ((defaultFlags & ConVar_Replicated) == 0)
					{
						varManager->RemoveEntryFlags(convarName, ConVar_Replicated);
					}

					convar->SetValue(convar->GetOfflineValue());

					// Replicated-by-default convars go back to unmodified
					if ((defaultFlags & ConVar_Replicated) != 0)
					{
						varManager->RemoveEntryFlags(convarName, ConVar_Modified);
					}
				}
			}

			m_convarsCreatedByServer.clear();
			m_convarsModifiedByServer.clear();
		}, 99999);
	}

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerConVars& serverConVars, ConVarsPacketHandler* handler)
		{
			auto unpacked = msgpack::unpack(reinterpret_cast<const char*>(serverConVars.data.GetValue().data()), serverConVars.data.GetValue().size());
			auto conVars = unpacked.get().as<std::map<std::string, std::string>>();

			se::ScopedPrincipal principalScope(se::Principal{ "system.console" });
			se::ScopedPrincipal principalScopeInternal(se::Principal{ "system.internal" });

			for (const auto& conVar : conVars)
			{
				auto convar = console::GetDefaultContext()->GetVariableManager()->FindEntryRaw(conVar.first);
				if (convar)
				{
					auto flags = console::GetDefaultContext()->GetVariableManager()->GetEntryFlags(conVar.first);

					if (flags & ConVar_UserPref)
					{
						console::DPrintf("net", "Blocked convar %s from being set by server.\n", conVar.first);
						continue;
					}

					// If the value is not already from the server
					if (!(flags & ConVar_Replicated))
					{
						convar->SaveOfflineValue();
					}

					handler->m_convarsModifiedByServer.push_back(conVar.first);
				}
				else
				{
					handler->m_convarsCreatedByServer.push_back(conVar.first);
				}
				console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ "setr", conVar.first, conVar.second });
			}
		}, this);
	}
};
}
