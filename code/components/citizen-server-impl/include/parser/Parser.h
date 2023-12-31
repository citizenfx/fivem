#pragma once

#include "StdInc.h"
#include <tinyxml2.h>

#include <ScriptWarnings.h>
#include <ClientRegistry.h>
#include <ResourceManager.h>
#include <VFSManager.h>

namespace fx::data {
	template<typename T>
	class Parser
	{
	protected:
		T m_data;
	public:
		inline bool LoadFile(fwRefContainer<fx::Resource> resource, std::string_view file)
		{
			const std::string& rootPath = resource->GetPath();

			if (rootPath.empty())
			{
				return false;
			}

			fwRefContainer<vfs::Stream> stream = vfs::OpenRead(rootPath + "/" + std::string{ file });

			if (!stream.GetRef())
			{
				fx::scripting::Warningf("", "Unable to locate file at @%s/%s", resource->GetName(), file);
				return false;
			}

			auto data = stream->ReadToEnd();
			tinyxml2::XMLDocument document;
			tinyxml2::XMLError error = document.Parse(reinterpret_cast<const char*>(data.data()), data.size());

			if (error == tinyxml2::XML_SUCCESS)
			{
				return ParseFile(document);
			}
		};

	   virtual bool ParseFile(tinyxml2::XMLDocument&) = 0;

		T GetData()
		{
			return m_data;
		};
	};
}
