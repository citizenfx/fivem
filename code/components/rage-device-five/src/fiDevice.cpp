#include "StdInc.h"
#include "Hooking.h"
#include "Hooking.Patterns.h"
#include "Hooking.Invoke.h"
#include "fiDevice.h"
#include "CrossLibraryInterfaces.h"

namespace rage
{
hook::cdecl_stub<rage::fiDevice*(const char*, bool)> fiDevice__GetDevice([] ()
{
	return hook::pattern("41 B8 07 00 00 00 48 8B F1 E8").count(1).get(0).get<void>(-0x1F);
});

fiDevice* fiDevice::GetDevice(const char* path, bool allowRoot) { return fiDevice__GetDevice(path, allowRoot); }

hook::cdecl_stub<bool(const char*, fiDevice*, bool)> fiDevice__MountGlobal([] ()
{
	return hook::pattern("41 8A F0 48 8B F9 E8 ? ? ? ? 33 DB 85 C0").count(1).get(0).get<void>(-0x28);
});

bool fiDevice::MountGlobal(const char* mountPoint, fiDevice* device, bool allowRoot)
{
	return fiDevice__MountGlobal(mountPoint, device, allowRoot);
}

// DCEC20
hook::cdecl_stub<void(const char*)> fiDevice__Unmount([] ()
{
	return hook::pattern("E8 ? ? ? ? 85 C0 75 23 48 83").count(1).get(0).get<void>(-0x22);
});

void fiDevice::Unmount(const char* rootPath) { fiDevice__Unmount(rootPath); }

rage::fiDevice::~fiDevice() {}

__declspec(dllexport) fwEvent<> fiDevice::OnInitialMount;
}

static InitFunction initFunction([] ()
{
#ifdef TESTING_RESOURCE_BITS
	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		rage::fiDevice* device = rage::fiDevice::GetDevice("platform:/models/farlods.ydd", true);

		packfile->OpenPackfile("platform:/levels/gta5/_citye/downtown_01/dt1_07.rpf", true, false, 3, false);
		packfile->Mount("temp:/");

		uint64_t ptr;
		uint64_t handle = packfile->OpenBulk("temp:/dt1_07_building2.ydr", &ptr);

		char* outBuffer = new char[512 * 1024];
		uint32_t read = packfile->ReadBulk(handle, ptr, outBuffer, 512 * 1024);

		packfile->CloseBulk(handle);

		rage::ResourceFlags flags;
		int32_t version = packfile->GetResourceVersion("temp:/dt1_07_building2.ydr", &flags);

		FILE* savedFile = fopen("Y:\\dev\\ydr\\dt1_07_building2.ydr", "wb");
		fwrite(outBuffer, 1, read, savedFile);
		fclose(savedFile);

		__debugbreak();

		/*rage::fiPackfile* packfile = new rage::fiPackfile();
		packfile->OpenPackfile("x64g.rpf", true, false, 3, false);
		packfile->Mount("temp:/");

		rage::fiFindData findData;
		auto findHandle = packfile->FindFirst("temp:/", &findData);

		if (findHandle != -1)
		{
			do 
			{
				trace("%s\n", findData.fileName);
			} while (packfile->FindNext(findHandle, &findData));

			packfile->FindClose(findHandle);
		}*/

		//__debugbreak();
	});
#endif

#ifdef TESTING_EXPORT
	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		rage::fiDevice* device = rage::fiDevice::GetDevice("common:/data/default.xml", true);

		auto dump = [&] (const char* path)
		{
			char dirName[256] = { 0 };
			strcpy(dirName, path);

			strrchr(dirName, '/')[0] = '\0';

			CreateDirectoryAnyDepth(va("Y:\\common\\%s", strchr(dirName, ':') + 1));

			auto handle = device->Open(path, true);

			auto dataLength = device->GetFileLength(handle);
			char* dataStuff = new char[dataLength];

			int read = device->ReadFull(handle, dataStuff, dataLength);

			FILE* f = fopen(va("Y:\\common\\%s", strchr(path, ':') + 1), "wb");
			fwrite(dataStuff, 1, dataLength, f);
			fclose(f);

			device->Close(handle);

			delete[] dataStuff;
		};

		std::vector<std::string> filenames;

		std::function<void(std::string)> recurse;
		recurse = [&] (std::string parent)
		{
			std::vector<std::string> directories;

			rage::fiFindData findData = { 0 };
			auto handlef = device->FindFirst(parent.c_str(), &findData);

			if (handlef != -1)
			{
				do
				{
					std::string fn = va("%s/%s", parent.c_str(), findData.fileName);

					if (findData.fileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						directories.push_back(fn);
					}
					else
					{
						filenames.push_back(fn);
					}
				} while (device->FindNext(handlef, &findData));

				device->FindClose(handlef);
			}

			for (auto& directory : directories)
			{
				recurse(directory);
			}
		};

		recurse("common:/");

		for (auto& file : filenames)
		{
			trace("%s\n", file.c_str());

			dump(file.c_str());
		}

		rage::fiDevice* device = rage::fiDevice::GetDevice("platform:/", true);

		ExitProcess(0);
	});
#endif
});