/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <sstream>

#include <ResourceManager.h>

using rage::fiDevice;

class ContentSetup
{
private:
	struct SetupData
	{
		Resource* resource;

		boost::optional<std::string> audioXmlFile;
		boost::optional<std::string> contentDatFile;
	};

	std::vector<SetupData> m_setupDataFiles;

public:
	ContentSetup();

	void InitializeHook();

	void AttachData();
};

static ContentSetup setup;

ContentSetup::ContentSetup()
{
	ResourceManager::OnScriptReset.Connect([=] ()
	{
		m_setupDataFiles.clear();
	});

	Resource::OnCreateScriptEnvironments.Connect([=] (fwRefContainer<Resource> resource)
	{
		// create a path and get the device
		auto setupPath = resource->GetPath() + "setup.xml";
		fiDevice* device = fiDevice::GetDevice(setupPath.c_str(), true);

		// if the device is usable
		if (device)
		{
			// open the file
			uint32_t handle = device->Open(setupPath.c_str(), true);

			// if it's opened
			if (handle != 0xFFFFFFFF)
			{
				// allocate a buffer
				uint32_t length = device->GetFileLength(handle);
				std::vector<char> buffer(length + 1);

				// read to the end
				device->Read(handle, &buffer[0], length);

				// close the file
				device->Close(handle);

				// parse the setup xml
				std::istringstream stream(std::string(&buffer[0], buffer.size()));

				try
				{
					boost::property_tree::ptree pt;
					boost::property_tree::read_xml(stream, pt);

					// create the data struct
					SetupData data;

					data.contentDatFile = pt.get_optional<std::string>("ini.datfile");
					data.audioXmlFile = pt.get_optional<std::string>("ini.audiometadata");
					data.resource = resource.GetRef();

					// append to the list
					m_setupDataFiles.push_back(data);
				}
				catch (...)
				{
					trace("Error reading setup.xml from resource %s.\n", resource->GetName().c_str());
				}
			}
		}
	});
}

void ContentSetup::AttachData()
{
	for (auto& data : m_setupDataFiles)
	{
		if (data.audioXmlFile.is_initialized())
		{
			// attach audio file
			((void(*)(const char*, bool))0x794370)(va("%s%s", data.resource->GetPath().c_str(), data.audioXmlFile->c_str()), false);
		}

		if (data.contentDatFile.is_initialized())
		{
			// attach content file
			((void(*)(const char*, int))0x8D79A0)(va("%s%s", data.resource->GetPath().c_str(), data.contentDatFile->c_str()), 0);
		}
	}
}

static void EpisodeLoadTail()
{
	setup.AttachData();
}

void ContentSetup::InitializeHook()
{
	// tail of episode content adding func
	hook::jump(0x8152D9, EpisodeLoadTail);
}

static HookFunction hookFunction([] ()
{
	setup.InitializeHook();
});