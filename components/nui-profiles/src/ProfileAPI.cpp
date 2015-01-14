/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "InternalRPCHandler.h"
#include "ProfileManager.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <sstream>

static InitFunction initFunction([] ()
{
	nui::RPCHandlerManager* rpcHandlerManager = Instance<nui::RPCHandlerManager>::Get();
	
	rpcHandlerManager->RegisterEndpoint("profiles", [] (std::string functionName, std::string arguments, std::map<std::string, std::string> postMap, nui::RPCHandlerManager::TCallbackFn cb)
	{
		ProfileManager* profileManager = Instance<ProfileManager>::Get();

		if (functionName == "list")
		{
			rapidjson::Document outDocument;
			outDocument.SetObject();

			rapidjson::Value profilesArray;
			profilesArray.SetArray();

			int numProfiles = profileManager->GetNumProfiles();

			for (int i = 0; i < numProfiles; i++)
			{
				fwRefContainer<Profile> profile = profileManager->GetProfile(i);
				rapidjson::Value profileObject;
				profileObject.SetObject();

				profileObject.AddMember("name", rapidjson::Value(profile->GetDisplayName(), outDocument.GetAllocator()).Move(), outDocument.GetAllocator());
				profileObject.AddMember("tile", rapidjson::Value(profile->GetTileURI(), outDocument.GetAllocator()).Move(), outDocument.GetAllocator());
				profileObject.AddMember("type", rapidjson::Value("steam").Move(), outDocument.GetAllocator());
				profileObject.AddMember("identifier", rapidjson::Value(profile->GetInternalIdentifier()).Move(), outDocument.GetAllocator());
				
				profilesArray.PushBack(profileObject, outDocument.GetAllocator());
			}

			outDocument.AddMember("profiles", profilesArray, outDocument.GetAllocator());

			// write the document
			rapidjson::StringBuffer s;
			rapidjson::Writer<rapidjson::StringBuffer> writer(s);
			
			outDocument.Accept(writer);

			// call the callback
			cb(s.GetString());
		}
		else if (functionName == "signin")
		{
			std::istringstream argumentStream(arguments);

			size_t profileId = 0;
			argumentStream >> profileId;

			if (profileId != 0)
			{
				int numProfiles = profileManager->GetNumProfiles();

				for (int i = 0; i < numProfiles; i++)
				{
					fwRefContainer<Profile> profile = profileManager->GetProfile(i);

					if (profile->GetInternalIdentifier() == profileId)
					{
						profileManager->SignIn(profile, postMap).then([=] (ProfileTaskResult& result)
						{
							cb(va("{ \"error\": %s }", (result.success) ? "null" : va("\"%s\"", result.error.c_str())));
						});
					}
					else
					{
						cb("{ \"error\": \"Invalid profile specified.\" }");
					}
				}
			}
		}
	});
});