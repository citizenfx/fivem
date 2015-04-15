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

// DCEC20
hook::cdecl_stub<void(const char*)> fiDevice__Unmount([] ()
{
	return (void*)nullptr;
});

void fiDevice::Unmount(const char* rootPath) { fiDevice__Unmount(rootPath); }

rage::fiDevice::~fiDevice() {}

__declspec(dllexport) fwEvent<> fiDevice::OnInitialMount;
}

static InitFunction initFunction([] ()
{
	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		rage::fiDevice* device = rage::fiDevice::GetDevice("platform:/models/farlods.ydd", true);
	});

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