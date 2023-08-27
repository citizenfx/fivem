#pragma once

#ifndef IS_FXSERVER
namespace cfx
{
inline auto GetPlatformRelease()
{
	static auto version = ([]()
	{
		FILE* f = _wfopen(MakeRelativeCitPath(L"citizen/release.txt").c_str(), L"r");
		int version = -1;

		if (f)
		{
			char ver[128];

			fgets(ver, sizeof(ver), f);
			fclose(f);

			version = atoi(ver);
		}
		else
		{
			version = 0;
		}

		return version;
	})();

	return version;
}
}
#endif
