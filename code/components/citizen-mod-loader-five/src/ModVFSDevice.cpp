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

#include <PureModeState.h>

#include <CrossBuildRuntime.h>
#include <Error.h>

#include <boost/algorithm/string.hpp>

void DLL_IMPORT CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags);

namespace streaming
{
	void AddDataFileToLoadList(const std::string& type, const std::string& path);
}

namespace fx
{
static void MountFauxStreamingRpf(const std::string& fn);

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

	virtual THandle Open(const std::string & fileName, bool readOnly, bool append = false) override
	{
		auto fn = MapFileName(fileName);

		if (fn.empty())
		{
			return InvalidHandle;
		}

		return m_parentDevice->Open(fn, readOnly, append);
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

	virtual size_t GetLength(THandle handle) override
	{
		return m_parentDevice->GetLength(handle);
	}

	virtual size_t GetLength(const std::string& fileName)
	{
		return m_parentDevice->GetLength(MapFileName(fileName));
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

	std::string GetAbsolutePath() const override;

	bool Flush(THandle handle) override;

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
			auto srcFile = entry.sourceFile;
			auto tgtFile = entry.targetFile;

			std::replace(srcFile.begin(), srcFile.end(), '\\', '/');
			std::replace(tgtFile.begin(), tgtFile.end(), '\\', '/');

			if (tgtFile[0] == '/')
			{
				tgtFile = tgtFile.substr(1);
			}

			if (tgtFile == "common/data/gameconfig.xml" ||
				tgtFile == "common/data/ai/scenarios.meta" ||
				tgtFile == "common/data/ai/conditionalanims.meta")
			{
				continue;
			}

			if (boost::algorithm::ends_with(tgtFile, ".rpf"))
			{
				MountFauxStreamingRpf(m_modPackage->GetRootPath() + "content/" + srcFile);
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

std::string ModVFSDevice::GetAbsolutePath() const
{
	return "";
}

bool ModVFSDevice::Flush(THandle handle)
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

bool loadedUnencryptedMod;
int modCount;

bool ModsNeedEncryption()
{
	if (fx::client::GetPureLevel() >= 1)
	{
		return true;
	}

	// they don't currently
	return false;
}

static void MountFauxStreamingRpf(const std::string& fn)
{
	static int packIdx;

	fwRefContainer<vfs::RagePackfile7> packfile = new vfs::RagePackfile7();
	if (packfile->OpenArchive(fn, ModsNeedEncryption()))
	{
		if (!ModsNeedEncryption())
		{
			loadedUnencryptedMod = true;
		}

		std::string devName;

		std::string mount = fmt::sprintf("faux_pack%d:/", packIdx++);
		vfs::Mount(packfile, mount);

		vfs::FindData findData;
		auto findHandle = packfile->FindFirst(mount, &findData);

		if (findHandle != INVALID_DEVICE_HANDLE)
		{
			bool shouldUseCache = false;
			bool shouldUseMapStore = false;

			do 
			{
				if (!(findData.attributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					std::string tfn = mount + findData.name;

					GetRagePageFlagsExtension data;
					data.fileName = tfn.c_str();
					packfile->ExtensionCtl(VFS_GET_RAGE_PAGE_FLAGS, &data, sizeof(data));

					CfxCollection_AddStreamingFileByTag(mount, tfn, data.flags);

					if (boost::algorithm::ends_with(tfn, ".ymf"))
					{
						shouldUseCache = true;
					}

					if (boost::algorithm::ends_with(tfn, ".ybn") || boost::algorithm::ends_with(tfn, ".ymap"))
					{
						shouldUseMapStore = true;
					}
				}
			} while (packfile->FindNext(findHandle, &findData));

			packfile->FindClose(findHandle);

			// in case of .#mf file
			if (shouldUseCache)
			{
				streaming::AddDataFileToLoadList("CFX_PSEUDO_CACHE", mount);
			}

			// in case of .#bn/.#map file
			if (shouldUseMapStore)
			{
				streaming::AddDataFileToLoadList("CFX_PSEUDO_ENTRY", "RELOAD_MAP_STORE");
			}
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

	modCount++;
	AddCrashometry("mod_package_count", "%d", modCount);
}

void MountModStream(const std::shared_ptr<fx::ModPackage>& modPackage)
{
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

		// strip leading slashes
		if (!tgtFile.empty() && tgtFile[0] == '/')
		{
			tgtFile = tgtFile.substr(1);
		}

		auto isCoreTexture = boost::algorithm::starts_with(tgtFile, "textures/");

		if (entry.archiveRoots.size() >= 2 || isCoreTexture)
		{
			// if only one path is there, as well
			auto slashCount = std::count(tgtFile.begin(), tgtFile.end(), '/');

			if (slashCount == 0 || isCoreTexture)
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
	struct DlcEntry
	{
		fwRefContainer<vfs::RagePackfile7> angryZip;
		std::string deviceName;
		int order;
	};

	std::vector<DlcEntry> dlcs;

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
				if (!ModsNeedEncryption())
				{
					loadedUnencryptedMod = true;
				}

				std::string devName;
				int order = 0;
				std::string requiredVersion;

				vfs::Mount(packfile, "tempModDlc:/");

				{
					auto setupFile = vfs::OpenRead("tempModDlc:/setup2.xml");
					auto text = setupFile->ReadToEnd();

					tinyxml2::XMLDocument doc;
					if (doc.Parse(reinterpret_cast<char*>(text.data()), text.size()) == tinyxml2::XML_SUCCESS)
					{
						devName = doc.RootElement()->FirstChildElement("deviceName")->GetText();

						if (auto orderEl = doc.RootElement()->FirstChildElement("order"); order)
						{
							order = orderEl->IntAttribute("value");
						}

						if (auto requiredVersionEl = doc.RootElement()->FirstChildElement("requiredVersion"); requiredVersionEl)
						{
							if (requiredVersionEl->GetText())
							{
								requiredVersion = requiredVersionEl->GetText();
							}
						}
					}
				}

				vfs::Unmount("tempModDlc:/");

				bool valid = true;

				if (!requiredVersion.empty())
				{
					int minBuild = 0;
					int maxBuild = INT32_MAX;

					// Cfx extension
					if (auto dashPos = requiredVersion.find("-"); dashPos != std::string::npos)
					{
						minBuild = std::stoi(requiredVersion.substr(0, dashPos));
						maxBuild = std::stoi(requiredVersion.substr(dashPos + 1));
					}
					else
					{
						minBuild = std::stoi(requiredVersion);
					}

					auto gameBuild = xbr::GetGameBuild();
					if (gameBuild < minBuild)
					{
						valid = false;
					}
					else if (gameBuild > maxBuild)
					{
						valid = false;
					}
				}

				if (valid)
				{
					DlcEntry entry;
					entry.deviceName = devName;
					entry.angryZip = packfile;
					entry.order = order;

					dlcs.push_back(entry);
				}
			}
		}
	}

	std::stable_sort(dlcs.begin(), dlcs.end(), [](const DlcEntry& left, const DlcEntry& right)
	{
		return (left.order < right.order);
	});

	for (const auto& dlc : dlcs)
	{
		const auto& packfile = dlc.angryZip;
		const auto& devName = dlc.deviceName;

		{
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

static InitFunction initFunction([]()
{
	OnPostFrontendRender.Connect([]()
	{
		if (fx::modCount > 1)
		{
			int gameWidth, gameHeight;
			GetGameResolution(gameWidth, gameHeight);

			static CRect metrics;
			static fwWString lastString;
			static float lastHeight;

			std::wstring brandingString = fmt::sprintf(L"%d mod packs loaded", fx::modCount);

			float gameWidthF = static_cast<float>(gameWidth);
			float gameHeightF = static_cast<float>(gameHeight);

			if (metrics.Width() <= 0.1f || lastString != brandingString || lastHeight != gameHeightF)
			{
				TheFonts->GetStringMetrics(brandingString, 22.0f * (gameHeightF / 1440.0f), 1.0f, "Segoe UI", metrics);

				lastString = brandingString;
				lastHeight = gameHeightF;
			}

			CRect drawRect = { 10.0f, gameHeightF - metrics.Height() - 10.0f, gameWidthF, gameHeightF };
			CRGBA color(180, 180, 180, 120);

			TheFonts->DrawText(brandingString, drawRect, color, 22.0f * (gameHeightF / 1440.0f), 1.0f, "Segoe UI");
		}
	}, -1000);
});
