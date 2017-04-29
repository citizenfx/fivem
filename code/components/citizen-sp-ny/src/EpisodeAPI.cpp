/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "InternalRPCHandler.h"

#include "EpisodeManager.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <sstream>

static InitFunction initFunction([] ()
{
	nui::RPCHandlerManager* rpcHandlerManager = Instance<nui::RPCHandlerManager>::Get();

	rpcHandlerManager->RegisterEndpoint("episodes", [] (std::string functionName, std::string arguments, std::map<std::string, std::string> postMap, nui::RPCHandlerManager::TCallbackFn cb)
	{
		EpisodeManager* episodeManager = Instance<EpisodeManager>::Get();

		if (functionName == "list")
		{
			rapidjson::Document outDocument;
			outDocument.SetObject();

			rapidjson::Value episodeArray;
			episodeArray.SetArray();

			// list the episodes
			auto& episodes = episodeManager->GetEpisodes();

			for (auto& episode : episodes)
			{
				rapidjson::Value episodeObject;
				episodeObject.SetObject();

				episodeObject.AddMember("name", rapidjson::Value(episode->GetName().c_str(), outDocument.GetAllocator()).Move(), outDocument.GetAllocator());
				episodeObject.AddMember("identifier", rapidjson::Value(episode->GetIdentifier().c_str(), outDocument.GetAllocator()).Move(), outDocument.GetAllocator());

				episodeArray.PushBack(episodeObject, outDocument.GetAllocator());
			}

			outDocument.AddMember("episodes", episodeArray, outDocument.GetAllocator());

			// write the document
			rapidjson::StringBuffer s;
			rapidjson::Writer<rapidjson::StringBuffer> writer(s);

			outDocument.Accept(writer);

			// call the callback
			cb(s.GetString());
		}
		else if (functionName == "run")
		{
			// decode the episode identifier
			std::string argumentsDec;
			UrlDecode(arguments, argumentsDec);

			// find an episode with this identifier
			auto& episodes = episodeManager->GetEpisodes();
			bool result = false;

			for (auto& episode : episodes)
			{
				if (episode->GetIdentifier() == argumentsDec)
				{
					result = episode->RunEpisode();

					break;
				}
			}

			cb(std::string(va("{ \"success\": %s }", (result) ? "true" : "false")));
		}
	});
});