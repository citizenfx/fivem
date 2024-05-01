#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <ForceConsteval.h>

#include <Client.h>

#include "GameServer.h"

namespace fx
{
	namespace ServerDecorators
	{
		struct HostVoteCount : public fwRefCountable
		{
			std::map<uint32_t, int> voteCounts;
		};
	}
}

DECLARE_INSTANCE_TYPE(fx::ServerDecorators::HostVoteCount);

namespace fx
{
	namespace ServerDecorators
	{
		// Used to vote for a new host which is initiated by client. The client that gets more then 50% of the votes will become the new host.
		class HeHostPacketHandler
		{
		public:
			static void Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& packet)
			{
				if (fx::IsOneSync())
				{
					return;
				}

				auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
				auto gameServer = instance->GetComponent<fx::GameServer>();

				auto allegedNewId = packet.Read<uint32_t>();
				auto baseNum = packet.Read<uint32_t>();

				// check if the current host is being vouched for
				auto currentHost = clientRegistry->GetHost();

				if (currentHost && currentHost->GetNetId() == allegedNewId)
				{
					trace("Got a late vouch for %s - they're the current arbitrator!\n", currentHost->GetName());
					return;
				}

				// get the new client
				auto newClient = clientRegistry->GetClientByNetID(allegedNewId);

				if (!newClient)
				{
					trace("Got a late vouch for %d, who doesn't exist.\n", allegedNewId);
					return;
				}

				// count the total amount of living (networked) clients
				int numClients = 0;

				clientRegistry->ForAllClients([&](const fx::ClientSharedPtr& client)
				{
					if (client->HasRouted())
					{
						++numClients;
					}
				});

				// get a count of needed votes
				int votesNeeded = (int)ceil(numClients * 0.6);

				if (votesNeeded <= 0)
				{
					votesNeeded = 1;
				}

				// count votes
				auto voteComponent = instance->GetComponent<HostVoteCount>();

				auto it = voteComponent->voteCounts.find(allegedNewId);

				if (it == voteComponent->voteCounts.end())
				{
					it = voteComponent->voteCounts.insert({allegedNewId, 1}).first;
				}

				++it->second;

				// log
				trace("Received a vouch for %s, they have %d vouches and need %d.\n", newClient->GetName(), it->second,
				      votesNeeded);

				// is the vote count exceeded?
				if (it->second >= votesNeeded)
				{
					// make new arbitrator
					trace("%s is the new arbitrator, with an overwhelming %d vote/s.\n", newClient->GetName(),
					      it->second);

					// clear vote list
					voteComponent->voteCounts.clear();

					// set base
					newClient->SetNetBase(baseNum);

					// set as host and tell everyone
					clientRegistry->SetHost(newClient);

					net::Buffer hostBroadcast;
					hostBroadcast.Write(fx::force_consteval<uint32_t, HashRageString("msgIHost")>);
					hostBroadcast.Write<uint16_t>(newClient->GetNetId());
					hostBroadcast.Write(newClient->GetNetBase());

					gameServer->Broadcast(hostBroadcast);
				}
			}

			static constexpr const char* GetPacketId()
			{
				return "msgHeHost";
			}
		};
	}
}
