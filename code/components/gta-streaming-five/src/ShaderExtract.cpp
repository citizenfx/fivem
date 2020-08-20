#include <StdInc.h>

// this randomly corrupts on weird systems and it's not failsafe enough
#if 0
#include <fiDevice.h>

#include <Error.h>

#include <CrossBuildRuntime.h>

static InitFunction initFunction([]()
{
	rage::fiDevice::OnInitialMount.Connect([]()
	{
		// 1604
		// 1868
		auto cacheRoot = MakeRelativeCitPath(fmt::sprintf(L"cache\\game\\shaders_%d", Is2060() ? 1868 : 1604));

		if (GetFileAttributesW(cacheRoot.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			CreateDirectoryW(cacheRoot.c_str(), NULL);

			auto roots = { "db", "win32_40_final", "win32_40_lq_final" };

			for (auto root : roots)
			{
				auto bit = va("update:/common/shaders/%s/", root);
				auto device = rage::fiDevice::GetDevice(bit, true);

				if (device)
				{
					rage::fiFindData findData;
					auto hdl = device->FindFirst(bit, &findData);

					if (hdl != -1)
					{
						do
						{
							auto fileHdl = device->Open(va("update:/common/shaders/%s/%s", root, findData.fileName), true);

							if (fileHdl != -1)
							{
								_wmkdir(va(L"%s\\%s", cacheRoot, ToWide(root)));

								FILE* outFile = _wfopen(va(L"%s\\%s\\%s", cacheRoot, ToWide(root), ToWide(findData.fileName)), L"wb");

								if (!outFile)
								{
									return;
								}

								uint8_t buffer[4096];
								size_t read = 0;
								
								do 
								{
									read = device->Read(fileHdl, buffer, sizeof(buffer));

									if (read > 0)
									{
										if (fwrite(buffer, 1, read, outFile) != read)
										{
											FatalError("Could not save shader %s.", findData.fileName);
										}
									}
								} while (read > 0);

								fclose(outFile);
								device->Close(fileHdl);
							}
						} while (device->FindNext(hdl, &findData));

						device->FindClose(hdl);
					}
				}
			}
		}

		rage::fiDeviceRelative* baseRelative = new rage::fiDeviceRelative();
		baseRelative->SetPath("update:/common/shaders/", true);
		baseRelative->Mount("common:/shaders/");

		rage::fiDeviceRelative* relative = new rage::fiDeviceRelative();
		relative->SetPath(ToNarrow(cacheRoot).c_str(), true);
		relative->Mount("common:/shaders/");
	}, -1000);
});
#endif
