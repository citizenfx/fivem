#pragma once

#ifdef COMPILING_RAGE_DEVICE_FIVE
#define DEVICE_EXPORT __declspec(dllexport)
#define DEVICE_IMPORT
#else
#define DEVICE_IMPORT __declspec(dllimport)
#define DEVICE_EXPORT
#endif

namespace rage
{
struct DEVICE_EXPORT fiAssetManager
{
	static fiAssetManager* GetInstance();

	void PushFolder(const char* folder);

	void PopFolder();
};
}
