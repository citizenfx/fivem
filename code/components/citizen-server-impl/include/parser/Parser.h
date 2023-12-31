#pragma once

#include "StdInc.h"
#include <tinyxml2.h>

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
		inline void LoadFile(fwRefContainer<fx::Resource> resource, std::string_view file)
		{
			const std::string& rootPath = resource->GetPath();

			if (rootPath.empty())
			{
				return;
			}

			fwRefContainer<vfs::Stream> stream = vfs::OpenRead(rootPath + "/" + std::string{ file });

			if (!stream.GetRef())
			{
				return;
			}

			auto data = stream->ReadToEnd();
			tinyxml2::XMLDocument document;
			tinyxml2::XMLError error = document.Parse(reinterpret_cast<const char*>(data.data()), data.size());

			if (error == tinyxml2::XML_SUCCESS)
			{
				ParseFile(document);
			}
		};

		virtual void ParseFile(tinyxml2::XMLDocument&)
		{

		}

		T GetData()
		{
			return m_data;
		};
	};
}
