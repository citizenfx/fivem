#include "StdInc.h"

#ifndef IS_FXSERVER
#include <VFSManager.h>
#include <VFSZipFile.h>
#include <ResourceManager.h>

static void MountNatives(const std::string& name)
{
	fwRefContainer<vfs::ZipFile> device = new vfs::ZipFile();

	if (device->OpenArchive(fmt::sprintf("citizen:/scripting/lua/%s.zip", name)))
	{
		vfs::Mount(device, fmt::sprintf("nativesLua:/%s/", name));
	}
}

static InitFunction initFunction([]()
{
	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager*)
	{
		static bool mountedFiles = false;

		if (!mountedFiles)
		{
			MountNatives("natives_universal");
			MountNatives("natives_21e43a33");
			MountNatives("natives_0193d0af");

			mountedFiles = true;
		}
	});
});
#endif
