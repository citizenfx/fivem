#include <StdInc.h>
#include <ModPackage.h>

#include <fiDevice.h>

#include <VFSManager.h>
#include <VFSRagePackfile7.h>

#include <CoreConsole.h>
#include <Streaming.h>

#include <tinyxml2.h>

#include <FontRenderer.h>
#include <DrawCommands.h>

void DLL_IMPORT CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags);

namespace streaming
{
	void AddDataFileToLoadList(const std::string& type, const std::string& path);
}

namespace fx
{
static std::list<fwRefContainer<vfs::Device>> g_devices;

struct IgnoreCaseLess
{
	inline bool operator()(const std::string& left, const std::string& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) < 0;
	}
};

#define VFS_GET_RAGE_PAGE_FLAGS 0x20001

struct GetRagePageFlagsExtension
{
	const char* fileName; // in
	int version;
	rage::ResourceFlags flags; // out
};

class ModVFSDevice : public vfs::Device
{
public:
	ModVFSDevice(const std::shared_ptr<ModPackage>& package);

	virtual THandle OpenBulk(const std::string& fileName, uint64_t* ptr) override
	{
		auto fn = MapFileName(fileName);

		if (fn.empty())
		{
			return InvalidHandle;
		}
		
		return m_parentDevice->OpenBulk(fn, ptr);
	}

	virtual THandle Open(const std::string & fileName, bool readOnly) override
	{
		auto fn = MapFileName(fileName);

		if (fn.empty())
		{
			return InvalidHandle;
		}

		return m_parentDevice->Open(fn, readOnly);
	}

	virtual size_t ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size) override
	{
		return m_parentDevice->ReadBulk(handle, ptr, outBuffer, size);
	}

	virtual size_t Read(THandle handle, void * outBuffer, size_t size) override
	{
		return m_parentDevice->Read(handle, outBuffer, size);
	}

	virtual size_t Seek(THandle handle, intptr_t offset, int seekType) override
	{
		return m_parentDevice->Seek(handle, offset, seekType);
	}

	virtual bool Close(THandle handle) override
	{
		return m_parentDevice->Close(handle);
	}

	virtual bool CloseBulk(THandle handle) override
	{
		return m_parentDevice->CloseBulk(handle);
	}

	virtual THandle FindFirst(const std::string & folder, vfs::FindData* findData) override
	{
		return THandle();
	}

	virtual bool FindNext(THandle handle, vfs::FindData* findData) override
	{
		return false;
	}

	virtual void FindClose(THandle handle) override
	{
	}

	virtual void SetPathPrefix(const std::string& pathPrefix) override
	{
		m_pathPrefix = pathPrefix;
	}

	virtual bool ExtensionCtl(int controlIdx, void* controlData, size_t controlSize) override
	{
		if (controlIdx == VFS_GET_RAGE_PAGE_FLAGS)
		{
			GetRagePageFlagsExtension* data = (GetRagePageFlagsExtension*)controlData;

			auto entry = MapFileName(data->fileName);

			data->fileName = entry.c_str();

			return m_parentDevice->ExtensionCtl(controlIdx, controlData, controlSize);
		}

		return false;
	}

	bool ShouldMountCommon();

	bool ShouldMountPlatform();

private:
	std::string MapFileName(const std::string& fn);

private:
	std::map<std::string, std::string, IgnoreCaseLess> m_entries;

	std::shared_ptr<ModPackage> m_modPackage;

	std::string m_pathPrefix;

	fwRefContainer<vfs::Device> m_parentDevice;
};

ModVFSDevice::ModVFSDevice(const std::shared_ptr<ModPackage>& package)
	: m_modPackage(package)
{
	m_parentDevice = vfs::GetDevice(package->GetRootPath());

	for (auto& entry : package->GetContent().entries)
	{
		if (entry.type == ModPackage::Content::Entry::Type::Add)
		{
			if (entry.archiveRoots.empty())
			{
				// probably a DLC?
				continue;
			}

			auto lastArchive = entry.archiveRoots.back();
			auto srcFile = entry.targetFile;
			auto tgtFile = entry.targetFile;

			std::replace(srcFile.begin(), srcFile.end(), '\\', '/');
			std::replace(tgtFile.begin(), tgtFile.end(), '\\', '/');

			if (tgtFile[0] == '/')
			{
				tgtFile = tgtFile.substr(1);
			}

			if (tgtFile == "common/data/gameconfig.xml")
			{
				continue;
			}

			if (lastArchive == "update\\update.rpf")
			{
				// update.rpf already is _mostly_ /common/ and /x64/, only replace the latter
				if (tgtFile.find("x64/") == 0)
				{
					tgtFile = "platform/" + tgtFile.substr(4);
				}
				else if (tgtFile.find("dlc_patch/") == 0)
				{
					continue;
				}

				m_entries[tgtFile] = srcFile;
			}
			else if (lastArchive.length() == 8 /* x64N.rpf */ && lastArchive.find("x64") == 0 && lastArchive.find(".rpf") == 3)
			{
				m_entries["platform/" + tgtFile] = srcFile;
			}
			else if (lastArchive == "common.rpf")
			{
				m_entries["common/" + tgtFile] = srcFile;
			}
		}
	}
}

bool ModVFSDevice::ShouldMountCommon()
{
	return true;
}

bool ModVFSDevice::ShouldMountPlatform()
{
	return true;
}

std::string ModVFSDevice::MapFileName(const std::string& name)
{
	auto e = m_entries.find(name.substr(m_pathPrefix.length()));

	if (e != m_entries.end())
	{
		return m_modPackage->GetRootPath() + "content/" + e->second;
	}

	return {};
}

bool ModsNeedEncryption()
{
	static ConVar<bool> modDevMode("modDevMode", ConVar_None, false);

	return !modDevMode.GetValue();
}

static void MountFauxStreamingRpf(const std::string& fn)
{
	static int packIdx;

	fwRefContainer<vfs::RagePackfile7> packfile = new vfs::RagePackfile7();
	if (packfile->OpenArchive(fn, ModsNeedEncryption()))
	{
		std::string devName;

		std::string mount = fmt::sprintf("faux_pack%d:/", packIdx++);
		vfs::Mount(packfile, mount);

		vfs::FindData findData;
		auto findHandle = packfile->FindFirst(mount, &findData);

		if (findHandle != INVALID_DEVICE_HANDLE)
		{
			do 
			{
				if (!(findData.attributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					std::string tfn = mount + findData.name;

					GetRagePageFlagsExtension data;
					data.fileName = tfn.c_str();
					packfile->ExtensionCtl(VFS_GET_RAGE_PAGE_FLAGS, &data, sizeof(data));

					CfxCollection_AddStreamingFileByTag(mount, tfn, data.flags);
				}
			} while (packfile->FindNext(findHandle, &findData));

			packfile->FindClose(findHandle);
		}

		g_devices.push_back(packfile);
	}
}

void MountModDevice(const std::shared_ptr<fx::ModPackage>& modPackage)
{
	{
		auto modRoot = fmt::sprintf("modVfs_%s:/", modPackage->GetGuidString());

		fwRefContainer<ModVFSDevice> device = new ModVFSDevice(modPackage);
		vfs::Mount(device, modRoot);

		if (device->ShouldMountCommon())
		{
			rage::fiDeviceRelative* relativeDevice = new rage::fiDeviceRelative();
			relativeDevice->SetPath((modRoot + "common/").c_str(), nullptr, true);
			relativeDevice->Mount("common:/");

			rage::fiDeviceRelative* relativeDeviceCrc = new rage::fiDeviceRelative();
			relativeDeviceCrc->SetPath((modRoot + "common/").c_str(), nullptr, true);
			relativeDeviceCrc->Mount("commoncrc:/");
		}

		if (device->ShouldMountPlatform())
		{
			rage::fiDeviceRelative* relativeDevice = new rage::fiDeviceRelative();
			relativeDevice->SetPath((modRoot + "platform/").c_str(), nullptr, true);
			relativeDevice->Mount("platform:/");

			rage::fiDeviceRelative* relativeDeviceCrc = new rage::fiDeviceRelative();
			relativeDeviceCrc->SetPath((modRoot + "platform/").c_str(), nullptr, true);
			relativeDeviceCrc->Mount("platformcrc:/");
		}

		g_devices.push_back(device);
	}

	// add streaming assets
	auto parentDevice = vfs::GetDevice(modPackage->GetRootPath());

	for (auto& entry : modPackage->GetContent().entries)
	{
		if (entry.type != ModPackage::Content::Entry::Type::Add)
		{
			continue;
		}

		if (entry.archiveRoots.empty())
		{
			continue;
		}

		auto lastArchive = entry.archiveRoots.back();
		auto tgtFile = entry.targetFile;

		std::replace(tgtFile.begin(), tgtFile.end(), '\\', '/');

		if (entry.archiveRoots.size() >= 2)
		{
			// if only one path is there, as well
			if (std::count(tgtFile.begin(), tgtFile.end(), '/') == 0)
			{
				// probably a streaming file
				std::string fn = modPackage->GetRootPath() + "content/" + entry.sourceFile;
				std::replace(fn.begin(), fn.end(), '\\', '/');

				GetRagePageFlagsExtension data;
				data.fileName = fn.c_str();
				parentDevice->ExtensionCtl(VFS_GET_RAGE_PAGE_FLAGS, &data, sizeof(data));

				CfxCollection_AddStreamingFileByTag("mod_" + modPackage->GetGuidString(), fn, data.flags);
			}
		}
	}

	// add pseudo-DLCs
	for (auto& entry : modPackage->GetContent().entries)
	{
		if (entry.type != ModPackage::Content::Entry::Type::Add)
		{
			continue;
		}

		if (entry.archiveRoots.empty())
		{
			std::string fn = modPackage->GetRootPath() + "content/" + entry.sourceFile;
			std::replace(fn.begin(), fn.end(), '\\', '/');

			fwRefContainer<vfs::RagePackfile7> packfile = new vfs::RagePackfile7();
			if (packfile->OpenArchive(fn, ModsNeedEncryption()))
			{
				std::string devName;

				vfs::Mount(packfile, "tempModDlc:/");

				{
					auto setupFile = vfs::OpenRead("tempModDlc:/setup2.xml");
					auto text = setupFile->ReadToEnd();

					tinyxml2::XMLDocument doc;
					if (doc.Parse(reinterpret_cast<char*>(text.data()), text.size()) == tinyxml2::XML_SUCCESS)
					{
						devName = doc.RootElement()->FirstChildElement("deviceName")->GetText();
					}
				}

				vfs::Unmount("tempModDlc:/");

				vfs::Mount(packfile, devName + ":/");

				{
					auto contentFile = vfs::OpenRead(devName + ":/content.xml");
					
					auto text = contentFile->ReadToEnd();

					tinyxml2::XMLDocument doc;
					if (doc.Parse(reinterpret_cast<char*>(text.data()), text.size()) == tinyxml2::XML_SUCCESS)
					{
						for (auto item = doc.RootElement()->FirstChildElement("dataFiles")->FirstChildElement("Item"); item; item = item->NextSiblingElement("Item"))
						{
							std::string filename = item->FirstChildElement("filename")->GetText();
							std::string fileType = item->FirstChildElement("fileType")->GetText();

							auto platform = filename.find("%PLATFORM%");

							if (platform != std::string::npos)
							{
								filename.replace(platform, 10, "x64");
							}

							if (fileType == "RPF_FILE")
							{
								MountFauxStreamingRpf(filename);
							}
							else
							{
								streaming::AddDataFileToLoadList(fileType, filename);
							}
						}
					}
				}

				g_devices.push_back(packfile);
			}
		}
	}
}
}

static InitFunction initFunction([]()
{
	OnPostFrontendRender.Connect([]()
	{
		if (!fx::ModsNeedEncryption())
		{
			TheFonts->DrawText(L"CFX MOD DEV MODE ENABLED", CRect(40.0f, 40.0f, 800.0f, 500.0f), CRGBA(255, 0, 0, 255), 40.0f, 1.0f, "Comic Sans MS");
		}
	}, -1000);
});
