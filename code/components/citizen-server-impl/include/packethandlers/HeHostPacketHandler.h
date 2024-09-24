#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <ForceConsteval.h>

#include <Client.h>

#include "ByteReader.h"
#include "GameServer.h"
#include "HeHost.h"
#include "IHost.h"

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
			HeHostPacketHandler(fx::ServerInstanceBase* instance)
			{
			}

			static void Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& packet)
			{
				if (fx::IsOneSync())
				{
					return;
				}

				static size_t kClientMaxPacketSize = net::SerializableComponent::GetSize<net::packet::ClientHeHost>();

				if (packet.GetRemainingBytes() > kClientMaxPacketSize)
				{
					return;
				}

				net::packet::ClientHeHost clientHeHost;

				net::ByteReader reader{ packet.GetRemainingBytesPtr(), packet.GetRemainingBytes() };
				if (!clientHeHost.Process(reader))
				{
					// this only happens when a malicious client sends packets not created from our client code
					return;
				}

				auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
				auto gameServer = instance->GetComponent<fx::GameServer>();

				// check if the current host is being vouched for
				auto currentHost = clientRegistry->GetHost();

				if (currentHost && currentHost->GetNetId() == clientHeHost.allegedNewId)
				{
					trace("Got a late vouch for %s - they're the current arbitrator!\n", currentHost->GetName());
					return;
				}

				// get the new client
				auto newClient = clientRegistry->GetClientByNetID(clientHeHost.allegedNewId);

				if (!newClient)
				{
					trace("Got a late vouch for %d, who doesn't exist.\n", clientHeHost.allegedNewId);
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

				auto it = voteComponent->voteCounts.find(clientHeHost.allegedNewId);

				if (it == voteComponent->voteCounts.end())
				{
					it = voteComponent->voteCounts.insert({clientHeHost.allegedNewId, 1}).first;
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
					newClient->SetNetBase(clientHeHost.baseNum);

					// set as host and tell everyone
					clientRegistry->SetHost(newClient);

					net::Buffer hostBroadcast(net::SerializableComponent::GetSize<net::packet::ServerIHostPacket>());
					net::packet::ServerIHostPacket serverIHostPacket;
					serverIHostPacket.data.netId = newClient->GetNetId();
					serverIHostPacket.data.baseNum = newClient->GetNetBase();
					net::ByteWriter writer(hostBroadcast.GetBuffer(), hostBroadcast.GetLength());
					serverIHostPacket.Process(writer);
					hostBroadcast.Seek(writer.GetOffset());
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
