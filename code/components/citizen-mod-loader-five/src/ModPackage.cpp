#include <StdInc.h>
#include <ModPackage.h>

#include <ManifestVersion.h> // for ParseGuid
#include <VFSManager.h>

#include <tinyxml2.h>

#include <fiDevice.h>
#include <VFSRagePackfile7.h>

#include <boost/algorithm/string.hpp>

namespace fx
{
ModPackage::ModPackage(const std::string& path)
	: m_rootPath(path)
{
	ParsePackage(path);
}

static ModPackage::Metadata ParseMetadata(const tinyxml2::XMLElement* element)
{
	ModPackage::Metadata md;

	md.name = element->FirstChildElement("name")->GetText();
	md.version = fmt::sprintf(
		"%d.%d",
		element->FirstChildElement("version")->FirstChildElement("major")->IntText(),
		element->FirstChildElement("version")->FirstChildElement("minor")->IntText()
	);
	md.authorName = element->FirstChildElement("author")->FirstChildElement("displayName")->GetText();
	md.description = element->FirstChildElement("description")->GetText();

	return md;
}

static void ParseContent(ModPackage::Content& content, const tinyxml2::XMLElement* element, const std::vector<std::string>& archiveRoots)
{
	for (auto child = element->FirstChildElement(); child; child = child->NextSiblingElement())
	{
		std::string_view elemType = child->Name();

		if (elemType == "archive")
		{
			auto newRoots = archiveRoots;
			newRoots.emplace_back(child->Attribute("path"));

			ParseContent(content, child, newRoots);
		}
		else if (elemType == "add")
		{
			ModPackage::Content::Entry entry;
			entry.archiveRoots = archiveRoots;
			entry.type = ModPackage::Content::Entry::Type::Add;
			entry.sourceFile = child->Attribute("source");
			entry.targetFile = child->GetText();
				
			content.entries.push_back(std::move(entry));
		}
	}
}

static ModPackage::Content ParseContent(const tinyxml2::XMLElement* element)
{
	ModPackage::Content content;
	ParseContent(content, element, {});

	return content;
}

void ModPackage::ParsePackage(const std::string& path)
{
	auto stream = vfs::OpenRead(path + "/assembly.xml");

	if (!stream.GetRef())
	{
		return;
	}

	auto data = stream->ReadToEnd();

	tinyxml2::XMLDocument doc;
	if (doc.Parse(reinterpret_cast<const char*>(data.data()), data.size()) == tinyxml2::XML_SUCCESS)
	{
		if (strcmp(doc.RootElement()->Name(), "package") != 0)
		{
			return;
		}

		if (!doc.RootElement()->Attribute("target", "Five"))
		{
			trace("Failed to parse mod package - target != Five");
			return;
		}

		m_guid = ParseGuid(doc.RootElement()->Attribute("id") + 1);

		m_metadata = ParseMetadata(doc.RootElement()->FirstChildElement("metadata"));

		m_content = ParseContent(doc.RootElement()->FirstChildElement("content"));
	}
}

void MountModDevice(const std::shared_ptr<fx::ModPackage>& modPackage);
void MountModStream(const std::shared_ptr<fx::ModPackage>& modPackage);
bool ModsNeedEncryption();
extern bool loadedUnencryptedMod;
}

struct IgnoreCaseLess
{
	inline bool operator()(const std::string& left, const std::string& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) < 0;
	}
};

static HookFunction hookFunction([]()
{
	rage::fiDevice::OnInitialMount.Connect([]()
	{
		auto cfxDevice = rage::fiDevice::GetDevice("cfx:/mods/", true);

		std::set<std::string, IgnoreCaseLess> modList;

		rage::fiFindData findData;
		auto handle = cfxDevice->FindFirst("cfx:/mods/", &findData);

		if (handle != -1)
		{
			do
			{
				if ((findData.fileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					std::string fn = findData.fileName;

					if (boost::algorithm::ends_with(fn, ".rpf"))
					{
						modList.insert(fn);
					}
				}
			} while (cfxDevice->FindNext(handle, &findData));

			cfxDevice->FindClose(handle);
		}

		std::list<std::shared_ptr<fx::ModPackage>> packages;

		for (auto& fn : modList)
		{
			std::string fullFn = "cfx:/mods/" + fn;
			std::string addonRoot = "addons:/" + fn.substr(0, fn.find_last_of('.')) + "/";

			fwRefContainer<vfs::RagePackfile7> addonPack = new vfs::RagePackfile7();
			if (addonPack->OpenArchive(fullFn, fx::ModsNeedEncryption()))
			{
				if (!fx::ModsNeedEncryption())
				{
					fx::loadedUnencryptedMod = true;
				}

				vfs::Mount(addonPack, addonRoot);

				auto modPackage = std::make_shared<fx::ModPackage>(addonRoot);
				fx::MountModDevice(modPackage);

				packages.push_front(modPackage);
			}
		}

		for (auto& package : packages)
		{
			fx::MountModStream(package);
		}
	}, 10);
});
